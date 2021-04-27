#ifndef MEMSIM_EVENTS_CONF_H
#define MEMSIM_EVENTS_CONF_H

#include "./access.h"
#include "./execution_time.h"
#include "./simulated_time.h"

#include "../models/event.h"
#include "../utils/macros.h"

#include <set>
#include <map>

using std::map;
using std::set;

namespace Event
{
    Pool *pool_;

    Pool *GetPool()
    {
        return pool_;
    }

    void ConfigurePool(map<string, set<string>> events_sources_map)
    {
        pool_ = new Pool();

        LogPoint();
        for (auto event_sources : events_sources_map)
        {
            string eventID = event_sources.first;
            Type *type = nullptr;
            if (eventID == Access::ID)
            {
                type = new Access(false);
            }
            else if (eventID == ExecutionTime::ID)
            {
                type = new ExecutionTime(false);
            }
            else if (eventID == SimulatedTime::ID)
            {
                type = new SimulatedTime(false);
            }

            if (type != nullptr)
            {
                pool_->RegisterType(type);
                for (auto event_source : event_sources.second)
                {
                    LogMessage("Event %s -> Source %s", event_sources.first, event_source);
                    type->RegisterSource(event_source);
                }
            }
        }
        LogPoint();
    }
}

#endif