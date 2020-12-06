#include "memsim_new.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include "pin.H"

void MemorySimulator::memaccess(uint64_t addr, memory_access_type type, uint32_t size, ADDRINT insaddr)
{
    int level = -1;

    // Must be canonical addr
    assert((addr >> 48) == 0);

    struct tlbe *te = NULL;
    uint64_t paddr = 0;
    if ((te = tlb_->alltlb_lookup(addr, &level)) != NULL)
    {
        tlb_agg_profile[COUNTER_HIT]++;
        tlb_profile[tlb_profile.Map(insaddr)][COUNTER_HIT]++;
        paddr = te->ppfn + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
    }
    else
    {
        tlb_agg_profile[COUNTER_MISS]++;
        tlb_profile[tlb_profile.Map(insaddr)][COUNTER_MISS]++;
        paddr = walk_page_table(addr, type, insaddr, level);
        assert(level >= 2 && level <= 4);

        assert(paddr != 0);
        assert(level != -1);
        // Insert in TLB
        tlb_->tlb_insert(addr, paddr, level);
    }

    bool cachehit = cache_->cache_access(paddr, type, size);
    const COUNTER_HM counter = cachehit ? COUNTER_HIT : COUNTER_MISS;
    cache_agg_profile[counter]++;
    cache_profile[cache_profile.Map(insaddr)][counter]++;

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
        const COUNTER_MEM counter = (paddr & SLOWMEM_BIT) ? COUNTER_SLOWMEM : COUNTER_FASTMEM;
        mmgr_agg_profile[counter]++;
        mmgr_profile[mmgr_profile.Map(insaddr)][counter]++;
    }
}

// 4-level page walk
uint64_t MemorySimulator::walk_page_table(uint64_t addr, memory_access_type type, ADDRINT insaddr, int &level)
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
            mmgr_agg_profile[COUNTER_PAGEFAULT]++;
            mmgr_profile[mmgr_profile.Map(insaddr)][COUNTER_PAGEFAULT]++;
            assert(pte->present);
            assert(!pte->readonly || type != TYPE_WRITE);
        }

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

void MemorySimulator::PrintInstructionProfiles()
{
    std::cout << "TLB Profile: " << std::endl;
    std::cout << tlb_profile.StringLong() << std::endl;
    std::cout << "Cache Profile: " << std::endl;
    std::cout << cache_profile.StringLong() << std::endl;
    std::cout << "MMGR Profile: " << std::endl;
    std::cout << mmgr_profile.StringLong() << std::endl;
}

void MemorySimulator::PrintAggregateProfiles()
{
    // TODO: Figure out how to do this...kinda sucks that we can't aggregate the values directly from the other ones
    std::cout << "TLB Profile: " << std::endl;
    std::cout << "Miss\t\tHit" << std::endl;
    for(const auto& val : tlb_agg_profile) {
        std::cout << val << "\t\t";
    }
    std::cout <<  std::endl;
    std::cout << "Cache Profile: " << std::endl;
    std::cout << "Miss\t\tHit" << std::endl;
    for(const auto& val : cache_agg_profile) {
        std::cout << val << "\t\t";
    }
    std::cout <<  std::endl;
    std::cout << "MMGR Profile: " << std::endl;
    std::cout << "Pagefaults\t\tSLOWMEM\t\tFASTMEM" << std::endl;
    for(const auto& val : mmgr_agg_profile) {
        std::cout << val << "\t\t";
    }
    std::cout <<  std::endl;
}

void MemorySimulator::WriteStatsFiles(std::string out_prefix)
{
    std::ofstream cache_file((out_prefix + "cache.out").c_str(), ios::out | ios::app);
    if (cache_file.is_open()) {
        for(const auto& val : cache_agg_profile) {
            cache_file << val << " ";
        }
        cache_file << endl;
    }
    cache_file.close();

    std::ofstream tlb_file((out_prefix + "tlb.out").c_str(), ios::out | ios::app);
    if (tlb_file.is_open()) {
        for(const auto& val : tlb_agg_profile) {
            tlb_file << val << " ";
        }
        tlb_file << endl;
    }
    tlb_file.close();


    std::ofstream mmgr_file((out_prefix + "mmgr.out").c_str(), ios::out | ios::app);
    if (mmgr_file.is_open()) {
        for(const auto& val : mmgr_agg_profile) {
            mmgr_file << val << " ";
        }
        mmgr_file << endl;
    }
    mmgr_file.close();
}