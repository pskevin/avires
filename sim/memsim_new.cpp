#include "memsim_new.h"
#include <assert.h>
#include <iostream>
#include "pin.H"

void MemorySimulator::memaccess(uint64_t addr, memory_access_type type)
{
  int level = -1;

  // Must be canonical addr
  assert((addr >> 48) == 0);

  // In TLB?
  struct tlbe *te = NULL;
  uint64_t paddr = 0;
  if((te = tlb_->alltlb_lookup(addr, &level)) != NULL) {
    tlbhits++;
    paddr = te->ppfn + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
  } else {
    tlbmisses++;
    struct cacheentry *ce = NULL;
    if (cache_ != NULL && (ce = cache_->cache_lookup(addr, &level)) != NULL) {
      // handle this case once we add caches
      // paddr = something
    } else {
        // 4-level page walk
        assert(cr3 != NULL);
        struct pte *ptable = cr3, *pte = NULL;

        for(level = 1; level <= 4 && ptable != NULL; level++) {
          pte = &ptable[(addr >> (48 - (level * 9))) & 511];

          add_runtime(TIME_PAGEWALK);

          if(!pte->present || (pte->readonly && type == TYPE_WRITE)) {
            mmgr_->pagefault(addr, pte->readonly && type == TYPE_WRITE);
            add_runtime(TIME_PAGEFAULT);
            pagefaults++;
            assert(pte->present);
            assert(!pte->readonly || type != TYPE_WRITE);
          }

          if(!pte->accessed && pte->pagemap && addr < 1048576 && (pte->addr & SLOWMEM_BIT)) {
            MEMSIM_LOG("[%s in SLOWMEM vaddr 0x%" PRIx64 ", pt %u], paddr 0x%" PRIx64 "\n",
                type == TYPE_WRITE ? "MODIFIED" : "ACCESSED",
                addr & pfn_mask(level - 2), level - 2, pte->addr);
          }
          pte->accessed = true;
          if(type == TYPE_WRITE) {
            pte->modified = true;
          }

          if(pte->pagemap) {
            // Page here -- terminate walk
            break;
          }
          
          ptable = pte->next;
      }

      assert(pte != NULL);
      assert(level >= 2 && level <= 4);
      paddr = pte->addr + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
    }

    assert(paddr != 0);
    assert(level != -1);
    // Insert in TLB
    tlb_->tlb_insert(addr, paddr, level);
  }

  if(type == TYPE_READ) {
    add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_READ : TIME_FASTMEM_READ);
  } else {
    add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_WRITE : TIME_FASTMEM_WRITE);
  }

  accesses[(paddr & SLOWMEM_BIT) ? SLOWMEM : FASTMEM]++;

  // Performance counters
  if(perf_callback != NULL) {
    perf_accesses++;
    if(perf_accesses >= perf_limit) {
      perf_accesses = 0;
      perf_callback(addr);
    }
  }
}

void MemorySimulator::add_runtime(size_t delta) {
#ifndef LOG_DEBUG
  static size_t oldruntime = 0;
#endif
  
  runtime += delta;

  if(wakeup_time != 0 && runtime >= wakeup_time) {
    wakeup_time = 0;
    PIN_SemaphoreWait(&wakeup_sem);
  }

  if(memsim_timebound != 0 && runtime >= memsim_timebound && !memsim_timebound_thread) {
    PIN_SemaphoreWait(&timebound_sem);
  }
  
#ifndef LOG_DEBUG
  if(runtime - oldruntime > 1000000) {	// Every millisecond
    fprintf(stderr, "Runtime: %.3f       \r", (float)runtime / 1000000000.0);
    oldruntime = runtime;
  }
#endif
}

void MemorySimulator::setCR3(pte* ptr) {
  cr3 = ptr;
}