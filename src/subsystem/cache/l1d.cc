#include "l1d.h"

namespace Cache
{
    bool L1D::Access(Address *addr, uint32_t size)
    {
        string source = "cache_access";
        execution_time->ObserveStart(source);
        const bool single = (size <= 4);

        bool ret = false;

        if (addr->AccessType == READ_MEM)
        {
            simulated_time->Observe(source, TimeTo[Simulate::Read]);
            if (single)
                ret = LoadSingle(addr);
            else
                ret = LoadMulti(addr, size);
        }

        if (addr->AccessType == WRITE_MEM)
        {
            simulated_time->Observe(source, TimeTo[Simulate::Write]);
            if (single)
                ret = StoreSingle(addr);
            else
                ret = StoreMulti(addr, size);
        }
        
        if(ret != false)
            access->Observe(source, Event::Access::Hit);
        else
            access->Observe(source, Event::Access::Miss);
        
        execution_time->ObserveEnd(source);
        return ret;
    }

    bool L1D::LoadMulti(Address *addr, uint32_t size)
    {
        const bool ret = cache_->Access(addr->Physical(), size, CACHE_BASE::ACCESS_TYPE_LOAD);
        return ret;
    }

    bool L1D::StoreMulti(Address *addr, uint32_t size)
    {
        const bool ret = cache_->Access(addr->Physical(), size, CACHE_BASE::ACCESS_TYPE_STORE);
        return ret;
    }

    bool L1D::LoadSingle(Address *addr)
    {
        // @todo we may access several cache lines for
        const bool ret = cache_->AccessSingleLine(addr->Physical(), CACHE_BASE::ACCESS_TYPE_LOAD);
        return ret;
    }

    bool L1D::StoreSingle(Address *addr)
    {
        // @todo we may access several cache lines for
        const bool ret = cache_->AccessSingleLine(addr->Physical(), CACHE_BASE::ACCESS_TYPE_STORE);
        return ret;
    }
}
