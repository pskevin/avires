#ifndef MEMSIM_SUBSYSTEM_SIMULATOR_H
#define MEMSIM_SUBSYSTEM_SIMULATOR_H

#include "./include/controllers/cache.h"
#include "./include/controllers/memory.h"
#include "./include/controllers/tlb.h"
#include "./include/models/address.h"
#include "./include/models/page_table_entry.h"
#include "./include/models/tlb_entry.h"
#include "./include/utils/macros.h"

#include "./include/models/event.h"
#include "./include/events/execution_time.h"
#include "./include/events/simulated_time.h"

#include <vector>

using std::vector;

class MemorySimulator
{

private:
    Event::ExecutionTime *execution_time;
    Event::SimulatedTime *simulated_time;

    Memory::Controller *mem_;
    TLB::Controller *tlb_;
    Cache::Controller *cache_;
    PageTableEntry *cr3_;

public:
    virtual void Access(Address *addr, uint64_t size);
    virtual uint64_t WalkPageTable(Address *addr, int &level);
    virtual Address *NewAddress(uint64_t vaddr, MemoryAccessType access_type);

    MemorySimulator(
        Memory::Controller *mem,
        TLB::Controller *tlb,
        Cache::Controller *cache) : mem_(mem), tlb_(tlb), cache_(cache), cr3_(mem->GetCR3())
    {
        execution_time = Event::GetPool()->Find<Event::ExecutionTime>();
        simulated_time = Event::GetPool()->Find<Event::SimulatedTime>();
    }

    enum class Simulate
    {
        PageFault,
        PageWalk,
        Count
    };

    static map<Simulate, uint64_t> TimeTo;

    static map<Simulate, uint64_t> SimulatedTimes()
    {
        map<Simulate, uint64_t> time_to;

        time_to[Simulate::PageFault] = 2000;
        time_to[Simulate::PageWalk] = 200;

        return time_to;
    }

};
#endif