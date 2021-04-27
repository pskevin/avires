#ifndef MEMSIM_MEMORY_UNIFIED_L2_H
#define MEMSIM_MEMORY_UNIFIED_L2_H

#include "pin.H"

#include "../include/controllers/tlb.h"
#include "../include/models/page.h"
#include "../include/models/tlb_entry.h"
#include "../include/utils/hash.h"
#include "../include/utils/macros.h"

#include "../include/models/page.h"

#include "../include/models/event.h"
#include "../include/events/access.h"
#include "../include/events/execution_time.h"
#include "../include/events/simulated_time.h"

namespace TLB
{
    // Hardware 2-level TLB emulating Cascade Lake
    class TwoLevel : public Controller
    {
    public:
        // Constants
        const static unsigned int L1_GIGA_ENTRIES = 1;
        const static unsigned int L1_HUGE_ENTRIES = 4;
        const static unsigned int L1_BASE_ENTRIES = 8;
        const static unsigned int L2_GIGA_ENTRIES = 4;
        const static unsigned int L2_HUGE_BASE_ENTRIES = 8;

        Entry *Lookup(Address *addr, int *level);
        void Insert(Address *addr, unsigned int level);
        void Shootdown(Address *addr);

        TwoLevel()
        {
            PIN_MutexInit(&lock_);
            access = Event::GetPool()->Find<Event::Access>();
            execution_time = Event::GetPool()->Find<Event::ExecutionTime>();
            simulated_time = Event::GetPool()->Find<Event::SimulatedTime>();
        }

    private:
        Event::Access *access;
        Event::ExecutionTime *execution_time;
        Event::SimulatedTime *simulated_time;

        PIN_MUTEX lock_;

        Entry l1tlb_1g[L1_GIGA_ENTRIES], l1tlb_2m[L1_HUGE_ENTRIES], l1tlb_4k[L1_BASE_ENTRIES];
        Entry l2tlb_1g[L2_GIGA_ENTRIES], l2tlb_2m4k[L2_HUGE_BASE_ENTRIES];

        Entry *LookupLevel(string level, Entry *tlb, unsigned int size, uint64_t vpfn);
        Entry *LookupLevel(Entry *tlb, unsigned int size, uint64_t vpfn);

        inline unsigned int hash(uint64_t num)
        {
            return jenkins_one_at_a_time_hash((uint8_t *)&num, sizeof(uint64_t));
        }
    };
}
#endif