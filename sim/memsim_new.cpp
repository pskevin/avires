#include "memsim_new.h"
#include <assert.h>
#include <iostream>
#include "memory_manager.h"
#include "pin.H"

void MemorySimulator::memaccess(uint64_t addr, memory_access_type type)
{
  int level;

  // Must be canonical addr
  assert((addr >> 48) == 0);

  // In TLB?
  struct tlbe *te = NULL;
  uint64_t paddr;
  if((te = alltlb_lookup(addr, &level)) == NULL) {
    tlbmisses++;

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

    // Insert in TLB
    tlb_insert(addr, paddr, level);
  } else {
    tlbhits++;
    paddr = te->ppfn + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
  }

#ifdef MMM
  assert(MMM_TAGS_SIZE <= UINT32_MAX);

  uint64_t cline = paddr / MMM_LINE_SIZE;
  unsigned int mmm_idx = tlb_hash(cline) % MMM_TAGS_SIZE;
  bool in_fastmem = mmm_tags[mmm_idx] == cline ? true : false;

  // MMM miss? Write back and (maybe) load new
  if(!in_fastmem) {
    mmm_misses++;
    if(mmm_tags[mmm_idx] != (uint64_t)-1) {
      add_runtime(TIME_SLOWMEM_WRITE);	// Write back
      accesses[SLOWMEM]++;
    }
    if(type == TYPE_READ) {
      add_runtime(TIME_SLOWMEM_READ);	// Load new
      accesses[SLOWMEM]++;
    }
    
    mmm_tags[mmm_idx] = cline;
  }

  add_runtime((type == TYPE_READ) ? TIME_FASTMEM_READ : TIME_FASTMEM_WRITE);
  accesses[FASTMEM]++;
#else
  if(type == TYPE_READ) {
    add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_READ : TIME_FASTMEM_READ);
  } else {
    add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_WRITE : TIME_FASTMEM_WRITE);
  }

  accesses[(paddr & SLOWMEM_BIT) ? SLOWMEM : FASTMEM]++;
#endif

  // Performance counters
  if(perf_callback != NULL) {
    perf_accesses++;
    if(perf_accesses >= perf_limit) {
      perf_accesses = 0;
      perf_callback(addr);
    }
  }
}

tlbe* MemorySimulator::alltlb_lookup(uint64_t vaddr, int* level) {
  tlbe *ret = NULL;

  // 1G L1 TLB
  ret = tlb_lookup(l1tlb_1g, 4, vaddr & GIGA_PFN_MASK);
  if(ret != NULL) {
    *level = 2;
    return ret;
  }

  // 2M L1 TLB
  ret = tlb_lookup(l1tlb_2m, 32, vaddr & HUGE_PFN_MASK);
  if(ret != NULL) {
    *level = 3;
    return ret;
  }

  // 4K L1 TLB
  ret = tlb_lookup(l1tlb_4k, 64, vaddr & BASE_PFN_MASK);
  if(ret != NULL) {
    *level = 4;
    return ret;
  }

  // 1G L2 TLB
  ret = tlb_lookup(l2tlb_1g, 16, vaddr & GIGA_PFN_MASK);
  if(ret != NULL) {
    *level = 2;
    return ret;
  }

  // 2M L2 TLB
  ret = tlb_lookup(l2tlb_2m4k, 1536, vaddr & HUGE_PFN_MASK);
  if(ret != NULL && ret->hugepage) {
    *level = 3;
    return ret;
  }

  ret = tlb_lookup(l2tlb_2m4k, 1536, vaddr & BASE_PFN_MASK);
  if(ret != NULL && !ret->hugepage) {
    *level = 4;
    return ret;
  }

  return NULL;
}

tlbe* MemorySimulator::tlb_lookup(struct tlbe *tlb, unsigned int size, uint64_t vpfn)
{
  // tlbe *ret;
  
  // pthread_mutex_lock(&tlb_lock);
  // struct tlbe *te = &tlb[tlb_hash(vpfn) % size];
  // if(te->present && te->vpfn == vpfn) {
  //   ret = te;
  // } else {
    // ret = NULL;
  // }

  // pthread_mutex_unlock(&tlb_lock);
  // return ret;
  return nullptr;
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

void MemorySimulator::tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level)
{
//   struct tlbe *te;
//   uint64_t vpfn = 0, ppfn = 0;

//   assert(level >= 2 && level <= 4);

//   pthread_mutex_lock(&tlb_lock);

//   switch(level) {
//   case 2:	// 1GB page
//     vpfn = vaddr & GIGA_PFN_MASK;
//     ppfn = paddr & GIGA_PFN_MASK;
//     te = &l1tlb_1g[tlb_hash(vpfn) % 4];
//     if(te->present) {
//       // Move previous entry down
//       assert(te->vpfn != vpfn);
//       memcpy(&l2tlb_1g[tlb_hash(vpfn) % 16], te, sizeof(struct tlbe));
//     }
//     break;

//   case 3:	// 2MB page
//     vpfn = vaddr & HUGE_PFN_MASK;
//     ppfn = paddr & HUGE_PFN_MASK;
//     te = &l1tlb_2m[tlb_hash(vpfn) % 32];
//     te->hugepage = true;

//     // Fall through...
//   case 4:	// 4KB page
//     if(level == 4) {
//       vpfn = vaddr & BASE_PFN_MASK;
//       ppfn = paddr & BASE_PFN_MASK;
//       te = &l1tlb_4k[tlb_hash(vpfn) % 64];
//       te->hugepage = false;
//     }
//     if(te->present) {
//       // Move previous entry down
//       assert(te->vpfn != vpfn);
//       memcpy(&l2tlb_2m4k[tlb_hash(vpfn) % 1536], te, sizeof(struct tlbe));
//     }
//     break;
//   }

//   te->present = true;
//   te->vpfn = vpfn;
//   te->ppfn = ppfn;
  
//   pthread_mutex_unlock(&tlb_lock);
}

void MemorySimulator::setCR3(pte* ptr) {
  cr3 = ptr;
}

void MemorySimulator::setMemoryManager(MemoryManager* mmgr) {
  mmgr_ = mmgr;
}