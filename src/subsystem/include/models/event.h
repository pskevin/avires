#ifndef MEMSIM_MODELS_EVENT_H
#define MEMSIM_MODELS_EVENT_H

#include "./address.h"

#include "../utils/macros.h"

#include "pin.H"


#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include <memory>

using std::map;
using std::ofstream;
using std::string;
using std::vector;
using std::set;

// using std::unique_ptr;
// using std::shared_ptr;

namespace Event
{
    class Type
    {
    protected:
        const bool dummy_;
    
    private:
        const string id_;
        int num_sources = 0;
        map<int, uint64_t> *working_set_; // Prospective unique pointer
        PIN_MUTEX lock_;


        void RefreshWorkingSet()
        {
            if(working_set_->size() > 0)
                data_.push_back(working_set_);
            working_set_ = new map<int, uint64_t>();
        }

        friend class Pool;

    public:
        uint64_t count = 0;
        // Why not vector? Because sources needn't observe events in
        // the order they are registered. For example,
        // 1. In cases of granular instrumentation, some sources may not log an event at each timestep
        // 2. In cases a source in observing events from a different thread (kswapd)

        vector<map<int, uint64_t>*> data_; // Prospective shared pointers
        map<string, int> sources_; 


        virtual ~Type() {}

        Type(bool dummy, string id) : dummy_(dummy), id_(id)
        {
            PIN_MutexInit(&lock_);
            working_set_ = new map<int, uint64_t>();
        }

        string ID() { return id_; }

        void RegisterSource(string source)
        {
            sources_[source] = num_sources;
            num_sources++;
        }

        virtual void Observe(string source, uint64_t value)
        {
            // Fail-safe, if specific event doesn't handle it
            if(!dummy_)
            {
                ObserveEvent(source, value);
            }
        }

        virtual void ObserveEvent(string source, uint64_t value)
        {
            PIN_MutexLock(&lock_);
            count++;
            auto id = sources_.find(source);
            if(id != sources_.end())
            {
                
                (*working_set_)[id->second] = value;
            }
            else
            {
                LogMessage("Could not find source %s", source);
            }
            PIN_MutexUnlock(&lock_);
        }

    };

    class Pool
    {
    private:
        vector<Address *> addrs_;   // Most definitely shared pointers
        map<string, Type *> types_; // Most definitely shared pointers

    public:
        Pool(){};

        void RegisterType(Type *type)
        {
            this->types_[type->ID()] = type;
        }

        void AddAddress(Address *addr)
        {
            this->addrs_.push_back(addr);
            for(auto kv: types_)
            {
                kv.second->RefreshWorkingSet();
            }
        }

        template <typename EventType>
        EventType *Find()
        {
            map<string, Type *>::iterator it = types_.find(EventType::ID);
            if (it != types_.end())
            {
                return static_cast<EventType*>(it->second);
            }
            LogMessage("\t\tReturning Dummy Type!");
            return new EventType(true);
        }

        void WriteAll(string file_prefix);

        void WriteAddress(string file_prefix);
    };

    Pool *GetPool();
    void ConfigurePool(map<string, set<string>> events_sources_map);
}
#endif
