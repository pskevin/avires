#include "memsim_new.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include "pin.H"

void MemorySimulator::memaccess(uint64_t addr, memory_access_type type, uint32_t size, uint64_t timestep)
{
    int level = -1;
    // assert(timestep == v_addrs.size());
    if (profiling_) {
        v_addrs.push_back(addr);
    }
    uint64_t initial_runtime = runtime;

    // Must be canonical addr
    assert((addr >> 48) == 0);

    struct tlbe *te = NULL;
    uint64_t paddr = 0;
    if ((te = tlb_->alltlb_lookup(addr, &level)) != NULL)
    {
        if (profiling_) {
            tlb_profile.Increment(timestep, "HIT");
        }
        paddr = te->ppfn + (addr & ((1 << (12 + (4 - level) * 9)) - 1));
    }
    else
    {
        if (profiling_) {
            tlb_profile.Increment(timestep, "MISS");
        }

        paddr = walk_page_table(addr, type, timestep, level);
        assert(level >= 2 && level <= 4);
        
        if (paddr == 0) {
            printf("BROKEN %lu %lu %d\n", addr, paddr, level);
        }
        
        assert (paddr != 0);
        assert(level != -1);
        // Insert in TLB
        tlb_->tlb_insert(addr, paddr, level);
    }

    bool cachehit = cache_->cache_access(paddr, type, size);
    if (profiling_) {
        const std::string counter = cachehit ? "HIT" : "MISS";
        cache_profile.Increment(timestep, counter);
    }

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
        // Pay the cost of accessing the memory (conditional on the type of memory)
        if (type == TYPE_READ)
        {
            add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_READ : TIME_FASTMEM_READ);
        }
        else
        {
            add_runtime((paddr & SLOWMEM_BIT) ? TIME_SLOWMEM_WRITE : TIME_FASTMEM_WRITE);
        }
        if (profiling_) {
            const std::string counter = (paddr & SLOWMEM_BIT) ? "SLOWMEM" : "FASTMEM";
            mmgr_profile.Increment(timestep, counter);
        }
    }

    if (profiling_) {
        assert(runtime - initial_runtime >= 0);
        runtime_profile.Set(timestep, "RUNTIME", runtime - initial_runtime);
    }
}

// 4-level page walk
uint64_t MemorySimulator::walk_page_table(uint64_t addr, memory_access_type type, uint64_t timestep, int &level)
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
            if (profiling_) {
                mmgr_profile.Increment(timestep, "PAGEFAULT");
            }
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
        PIN_SemaphoreSet(&wakeup_sem);
    }

    if (memsim_timebound != 0 && runtime >= memsim_timebound && !static_cast<bool>(OS_TlsGetValue(memsim_timebound_thread)))
    {
        PIN_SemaphoreWait(&timebound_sem);
    }

#ifndef LOG_DEBUG
    if (profiling_ && runtime - oldruntime > 1000000)
    { // Every millisecond
        fprintf(stderr, "Runtime: %.3f       \r", (double)runtime / 1000000000.0);
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
        assert(static_cast<bool>(OS_TlsGetValue(memsim_timebound_thread)) == true);
        memsim_timebound = 0;
        PIN_SemaphoreSet(&timebound_sem);
    }
    
    PIN_SemaphoreWait(&wakeup_sem);
    assert(wakeup_time == 0);
    wakeup_time = runtime + sleeptime;
}

void MemorySimulator::setCR3(pte *ptr)
{
    cr3 = ptr;
}

void MemorySimulator::PrintInstructionProfiles()
{
    std::cout << "TLB Profile: " << std::endl;
    std::cout << tlb_profile.String() << std::endl;
    std::cout << "Cache Profile: " << std::endl;
    std::cout << cache_profile.String() << std::endl;
    std::cout << "MMGR Profile: " << std::endl;
    std::cout << mmgr_profile.String() << std::endl;
    std::cout << "Runtime Profile: " << std::endl;
    std::cout << runtime_profile.String() << std::endl;
}

void MemorySimulator::PrintAggregateProfiles()
{
    std::cout << "TLB Profile: " << std::endl;
    std::cout << tlb_profile.AggregateString() << std::endl;

    std::cout << std::endl;
    std::cout << "Cache Profile: " << std::endl;
    std::cout << cache_profile.AggregateString() << std::endl;

    std::cout << std::endl;
    std::cout << "MMGR Profile: " << std::endl;
    std::cout << mmgr_profile.AggregateString() << std::endl;
    std::cout << std::endl;

    std::cout << "Runtime Profile: " << std::endl;
    std::cout << runtime_profile.AggregateString() << std::endl;
}

void MemorySimulator::WriteStatsFiles(std::string out_prefix)
{
    std::ofstream cache_agg_file((out_prefix + "cache_agg.out").c_str(), ios::out | ios::app);
    if (cache_agg_file.is_open()) {
        cache_agg_file << cache_profile.AggregateString() << std::endl;
    }
    cache_agg_file.close();

    std::ofstream tlb_agg_file((out_prefix + "tlb_agg.out").c_str(), ios::out | ios::app);
    if (tlb_agg_file.is_open()) {
        tlb_agg_file << tlb_profile.AggregateString() << std::endl;
    }
    tlb_agg_file.close();

    std::ofstream mmgr_agg_file((out_prefix + "mmgr_agg.out").c_str(), ios::out | ios::app);
    if (mmgr_agg_file.is_open()) {
        mmgr_agg_file << mmgr_profile.AggregateString() << std::endl;
    }
    mmgr_agg_file.close();

    std::ofstream runtime_agg_file((out_prefix + "runtime_agg.out").c_str(), ios::out | ios::app);
    if (runtime_agg_file.is_open()) {
        runtime_agg_file << runtime_profile.AggregateString() << std::endl;
    }
    runtime_agg_file.close();


    std::ofstream cache_file((out_prefix + "cache.out").c_str(), ios::out);
    if (cache_file.is_open()) {
        cache_file << cache_profile.String() << std::endl;
    }
    cache_file.close();

    std::ofstream tlb_file((out_prefix + "tlb.out").c_str(), ios::out);
    if (tlb_file.is_open()) {
        tlb_file << tlb_profile.String() << std::endl;
    }
    tlb_file.close();

    std::ofstream mmgr_file((out_prefix + "mmgr.out").c_str(), ios::out);
    if (mmgr_file.is_open()) {
        mmgr_file << mmgr_profile.String() << std::endl;
    }
    mmgr_file.close();

    std::ofstream runtime_file((out_prefix + "runtime.out").c_str(), ios::out);
    if (runtime_file.is_open()) {
        runtime_file << runtime_profile.String() << std::endl;
    }
    runtime_file.close();

    std::ofstream vaddr_file((out_prefix + "vaddrs.out").c_str(), ios::out);
    if (vaddr_file.is_open()) {
        for(const auto v_addr : v_addrs) {
            vaddr_file << v_addr << std::endl;
        }
    }
    vaddr_file.close();
}

CacheManager* MemorySimulator::GetCacheManager() 
{
    return cache_;
}

MemoryManager* MemorySimulator::GetMemoryManager() 
{
    return mmgr_;
}