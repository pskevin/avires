#ifndef MEMSIM_EVENTS_SIMULATED_TIME_H
#define MEMSIM_EVENTS_SIMULATED_TIME_H

#include "../models/event.h"

#include <string>

using std::string;

namespace Event
{
    class SimulatedTime : public Type
    {
    private:
        
    public:
        static string ID;        

        SimulatedTime(bool dummy) : Type(dummy, ID) {}

        void Observe(string source, uint64_t time);

    };
}

#endif