#ifndef MEMSIM_SUBSYSTEM_MEMORY_FCFS_H
#define MEMSIM_SUBSYSTEM_MEMORY_FCFS_H

#include "pin.H"

#include "../include/models/page.h"
#include "../include/controllers/memory.h"
#include "../include/models/tlb_entry.h"
#include "../include/utils/macros.h"

#include "../include/models/event.h"
#include "../include/events/access.h"
#include "../include/events/execution_time.h"
#include "../include/events/simulated_time.h"

namespace Memory
{

    class FirstComeFirstServe : public Controller
    {
    public:
        void PageFault(Address *addr, bool readonly);
        PageTableEntry* GetCR3();

        FirstComeFirstServe(Page::Type pt) : pt_(pt){
            access = Event::GetPool()->Find<Event::Access>();
            execution_time = Event::GetPool()->Find<Event::ExecutionTime>();
            simulated_time = Event::GetPool()->Find<Event::SimulatedTime>();
        };

    private:
        Event::Type* access;
        Event::ExecutionTime *execution_time;
        Event::SimulatedTime *simulated_time;

        PageTableEntry* AllocatePageTables(Address *addr, Page::Type pt);
        uint64_t GetPhysicalPage(Address *addr, PageTableEntry*pte);

        PageTableEntry pml4_[512];
        uint64_t fastmem = 0, slowmem = 0;
        Page::Type pt_;
    };

}
#endif