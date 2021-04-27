#ifndef MEMSIM_CONTROLLERS_TLB_H
#define MEMSIM_CONTROLLERS_TLB_H

#include "../models/address.h"
#include "../models/tlb_entry.h"

#include <map>

using std::map;

namespace TLB
{
    enum class Simulate
    {
        Read,
        Write,
        Shootdown,
        Count
    };

    static map<Simulate, uint64_t> SimulatedTimes()
    {
        map<Simulate, uint64_t> time_to;
        time_to[Simulate::Read] = 0;
        time_to[Simulate::Write] = 1;
        time_to[Simulate::Shootdown] = 4000;
        return time_to;
    }

    static map<Simulate, uint64_t> TimeTo = SimulatedTimes();

    class Controller
    {
    public:
        virtual TLB::Entry* Lookup(Address *addr, int *level);
        virtual void Insert(Address *addr, unsigned int level);
        virtual void Shootdown(Address *addr);
    };
}
#endif