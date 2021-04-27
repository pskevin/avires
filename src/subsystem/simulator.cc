#include "simulator.h"

#include "./include/events/access.h"

#include <cassert>

#include <map>

using std::map;

map<MemorySimulator::Simulate, uint64_t> MemorySimulator::TimeTo = MemorySimulator::SimulatedTimes();

void MemorySimulator::Access(Address *addr, uint64_t size)
{
    string source = "mem_access";
    execution_time->ObserveStart(source);

    // Must be a canonical address
    assert((addr->Virtual() >> 48) == 0);

    int level = -1;
    uint64_t paddr;
    TLB::Entry *tlbe = NULL;

    tlbe = tlb_->Lookup(addr, &level);
    if (tlbe != NULL)
    {

        paddr = tlbe->ppfn + addr->PhysicalOffset(level);

        // Physical Page Frame exists
        assert((addr->Physical() ^ Address::NonCanonical) == 0);
        addr->Physical(paddr);
    }
    else
    {
        addr->Physical(WalkPageTable(addr, level));
        tlb_->Insert(addr, level);
    }

    bool hit = cache_->Access(addr, size);

    if (hit)
    {
        if (addr->AccessType == READ_MEM)
            simulated_time->Observe("cache_access", Cache::TimeTo[Cache::Simulate::Read]);
        else
            simulated_time->Observe("cache_access", Cache::TimeTo[Cache::Simulate::Write]);
    }
    else
    {
        // Pay the cost of accessing the memory (conditional on the type of memory)
        string source = "mem";
        Memory::Type type;
        if(addr->Physical() & Memory::SlowBit)
        {
            type = Memory::Type::Slow; 
            source += "_slow";
        }
        else
        {
            type = Memory::Type::Fast;
            source += "_fast";
        }

        Memory::Simulate operation;
        if( addr->AccessType == READ_MEM)
        {
            operation =  Memory::Simulate::Read; 
            source += "_read";
        }
        else
        {
            operation =  Memory::Simulate::Write; 
            source += "_write";
        }
        
        simulated_time->Observe(source, Memory::TimeTo[operation][type]);
    }

    Event::GetPool()->AddAddress(addr);
    execution_time->ObserveEnd(source);
}

Address *MemorySimulator::NewAddress(uint64_t vaddr, MemoryAccessType access_type)
{
    return new Address(vaddr, access_type);
}