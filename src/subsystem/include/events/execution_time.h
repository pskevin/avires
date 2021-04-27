#ifndef MEMSIM_EVENTS_EXECUTION_TIME_H
#define MEMSIM_EVENTS_EXECUTION_TIME_H

#include "../models/event.h"


#include <string>

using std::string;

namespace Event
{
    class ExecutionTime : public Type
    {
    private:
        uint64_t start, end;

    public:
        static string ID;
        
        ExecutionTime(bool dummy) : Type(dummy, ID){}

        void ObserveStart(string source);
        void ObserveEnd(string source);
    };
}

#endif