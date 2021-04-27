#ifndef MEMSIM_EVENTS_ACCESS_H
#define MEMSIM_EVENTS_ACCESS_H

#include "../models/event.h"

#include <string>

using std::string;

namespace Event
{
    class Access : public Type
    {
    private:
        
    public:
        static string ID;        

        Access(bool dummy) : Type(dummy, "access") {}

        typedef enum
        {
            Miss,
            Hit,
            Count
        } Attempt;

        void Observe(string source, Attempt obs);
    };
}

#endif