#ifndef MEMSIM_CONTROLLERS_CACHE_H
#define MEMSIM_CONTROLLERS_CACHE_H

#include "../models/address.h"

#include<map>

using std::map;

namespace Cache
{

    enum class Simulate
    {
        Read,
        Write,
        Count
    };

    static map<Simulate, uint64_t> SimulatedTimes()
    {
        map<Simulate, uint64_t> time_to;
        time_to[Simulate::Read] = 1;
        time_to[Simulate::Write] = 2;
        return time_to;
    }

    static map<Simulate, uint64_t> TimeTo = SimulatedTimes();

    class Controller
    {
    public:
        virtual bool Access(Address *addr, uint32_t size);
    };
}

#endif