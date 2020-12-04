#include "l1_data_cache.h"

bool L1DataCache::cache_access(uint64_t paddr, memory_access_type type, uint32_t size)
{
    const bool single = (size <= 4);

    if (type == TYPE_READ)
    {
        if (single)
            return load_single(paddr);
        else
            return load_multi(paddr, size);
    }

    if (type == TYPE_WRITE)
    {
        if (single)
            return store_single(paddr);
        else
            return store_multi(paddr, size);
    }

    return false;
}

bool L1DataCache::load_multi(uint64_t addr, uint32_t size)
{
    // first level D-cache
    const bool l1d = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);

    // const COUNTER counter = l1d ? COUNTER_HIT : COUNTER_MISS;
    // profile[instId][counter]++;
    return l1d;
}

bool L1DataCache::store_multi(uint64_t addr, uint32_t size)
{
    // first level D-cache
    const bool l1d = dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE);

    // const COUNTER counter = l1d ? COUNTER_HIT : COUNTER_MISS;
    // profile[instId][counter]++;
    return l1d;
}

bool L1DataCache::load_single(uint64_t addr)
{
    // @todo we may access several cache lines for
    // first level D-cache
    const bool l1d = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);

    // const COUNTER counter = l1d ? COUNTER_HIT : COUNTER_MISS;
    // profile[instId][counter]++;
    return l1d;
}

bool L1DataCache::store_single(uint64_t addr)
{
    // @todo we may access several cache lines for
    // first level D-cache
    const bool l1d = dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE);

    // const COUNTER counter = l1d ? COUNTER_HIT : COUNTER_MISS;
    // profile[instId][counter]++;
    return l1d;
}