#include "memsim_new.h"
#include <assert.h>
#include <iostream>
#include "pin.H"

void MemorySimulator::memaccess(uint64_t addr, memory_access_type type, uint32_t size, uint64_t insid)
{
    int level = -1;

    // Must be canonical addr
    assert((addr >> 48) == 0);

    struct tlbe *te = NULL;
    uint64_t paddr = 0;
    if ((te = tlb_->alltlb_lookup(addr, &level)) != NULL)
    {
        tlb_profile[insid][COUNTER_HIT]++;
        paddr = te->ppfn + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
    }
    else
    {
        tlb_profile[insid][COUNTER_MISS]++;
       
        paddr = walk_page_table(addr, type, level);
        assert(level >= 2 && level <= 4);

        assert(paddr != 0);
        assert(level != -1);
        // Insert in TLB
        tlb_->tlb_insert(addr, paddr, level);
    }

    bool cachehit = cache_->cache_access(paddr, type, size);
    const COUNTER counter = cachehit ? COUNTER_HIT : COUNTER_MISS;
    cache_profile[insid][counter]++;

    if(cachehit) 
    {
        if (type == TYPE_READ) {
            add_runtime(TIME_CACHE_READ);
        } else {
            add_runtime(TIME_CACHE_WRITE);
        }
    }
    else
    { 
        // Everything below needs to go. Runtime should be decided by individual pieces and not memaccess. Perf counters are similarly useless here.
        // Pay the cost of accessing the memory (conditional on the type of memory)
        if (type == TYPE_READ)
        {
            add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_READ : TIME_FASTMEM_READ);
        }
        else
        {
            add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_WRITE : TIME_FASTMEM_WRITE);
        }
        accesses[(paddr & SLOWMEM_BIT) ? SLOWMEM : FASTMEM]++;   
    }
}

// 4-level page walk
uint64_t MemorySimulator::walk_page_table(uint64_t addr, memory_access_type type, int &level)
{
    assert(cr3 != NULL);
    struct pte *ptable = cr3, *pte = NULL;

    for (level = 1; level <= 4 && ptable != NULL; level++)
    {
        pte = &ptable[(addr >> (48 - (level * 9))) & 511];

        add_runtime(TIME_PAGEWALK);
        if (!pte->present || (pte->readonly && type == TYPE_WRITE))
        {
            mmgr_->pagefault(addr, pte->readonly && type == TYPE_WRITE);
            add_runtime(TIME_PAGEFAULT);
            pagefaults++;
            assert(pte->present);
            assert(!pte->readonly || type != TYPE_WRITE);
        }

        // printf("HERE %d %d %lu\n", !pte->accessed, pte->pagemap, pte->accessed & SLOWMEM_BIT);
        if (!pte->accessed && pte->pagemap && (pte->addr & SLOWMEM_BIT))
        {
            MEMSIM_LOG("[%s in SLOWMEM vaddr 0x%" PRIx64 ", pt %u], paddr 0x%" PRIx64 "\n",
                       runtime,
                       type == TYPE_WRITE ? "MODIFIED" : "ACCESSED",
                       addr & pfn_mask((pagetypes)(level - 2)), level - 2, pte->addr);
        }

        pte->accessed = true;
        if (type == TYPE_WRITE)
        {
            pte->modified = true;
        }

        if (pte->pagemap)
        {
            // Page here -- terminate walk
            break;
        }

        ptable = pte->next;
    }

    assert(pte != NULL);
    return pte->addr + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
}

void MemorySimulator::add_runtime(size_t delta)
{
#ifndef LOG_DEBUG
    static size_t oldruntime = 0;
#endif

    runtime += delta;

    if (wakeup_time != 0 && runtime >= wakeup_time)
    {
        wakeup_time = 0;
        PIN_SemaphoreWait(&wakeup_sem);
    }

    if (memsim_timebound != 0 && runtime >= memsim_timebound && !memsim_timebound_thread)
    {
        PIN_SemaphoreWait(&timebound_sem);
    }

#ifndef LOG_DEBUG
    if (runtime - oldruntime > 1000000)
    { // Every millisecond
        fprintf(stderr, "Runtime: %.3f       \r", (float)runtime / 1000000000.0);
        oldruntime = runtime;
    }
#endif
}

void MemorySimulator::tlb_shootdown(uint64_t addr)
{
    tlb_->shootdown(addr);
    add_runtime(TIME_TLBSHOOTDOWN);
}

void MemorySimulator::memsim_nanosleep(size_t sleeptime)
{
    if (memsim_timebound != 0)
    {
        assert(memsim_timebound_thread == true);
        memsim_timebound = 0;
        // TODO validate sem_post is the same as SemaphoreSet
        PIN_SemaphoreSet(&timebound_sem);
    }

    assert(wakeup_time == 0);
    wakeup_time = runtime + sleeptime;
    PIN_SemaphoreWait(&wakeup_sem);
}

void MemorySimulator::setCR3(pte *ptr)
{
    cr3 = ptr;
}