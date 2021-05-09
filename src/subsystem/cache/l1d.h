#ifndef MEMSIM_SUBSYSTEM_CACHE_SIMULATOR_H
#define MEMSIM_SUBSYSTEM_CACHE_SIMULATOR_H

#include "../include/controllers/cache.h"
#include "../include/models/address.h"
#include "../include/utils/macros.h"

#include "../include/models/event.h"
#include "../include/events/access.h"
#include "../include/events/execution_time.h"
#include "../include/events/simulated_time.h"

#include "dcache.h"

#include <cstdint>

namespace Cache
{
    class L1D : public Controller
    {
    public:
        const static uint32_t max_sets = KB(1);        // cacheSize / (lineSize * associativity);
        const static uint32_t max_associativity = 256; // associativity;
        const static CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

        typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) RRCache;

        const static unsigned int CacheSize = KB(32);
        const static unsigned int LineSize = 64;
        const static unsigned int Associativity = 8;

        L1D()
        {
            this->cache_ = new RRCache("L1 Data Cache", CacheSize, LineSize, Associativity);
            access = Event::GetPool()->Find<Event::Access>();
            execution_time = Event::GetPool()->Find<Event::ExecutionTime>();
            simulated_time = Event::GetPool()->Find<Event::SimulatedTime>();
        }

        bool Access(Address *addr, uint32_t size);

    private:
        Event::Type* access;
        Event::ExecutionTime *execution_time;
        Event::SimulatedTime *simulated_time;

        bool LoadMulti(Address *addr, uint32_t size);
        bool StoreMulti(Address *addr, uint32_t size);
        bool LoadSingle(Address *addr);
        bool StoreSingle(Address *addr);

        RRCache *cache_;
    };
}
#endif