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

/*! RMR (rodric@gmail.com) 
 *   - temporary work around because decstr()
 *     casts 64 bit ints to 32 bit ones
 */
static string mydecstr(uint64_t v, uint64_t w)
{
  ostringstream o;
  o.width(w);
  o << v;
  string str(o.str());
  return str;
}

CACHE_BASE::CACHE_BASE(std::string name, uint64_t cacheSize, uint64_t lineSize, uint64_t associativity)
    : _name(name),
      _cacheSize(cacheSize),
      _lineSize(lineSize),
      _associativity(associativity),
      _lineShift(FloorLog2(lineSize)),
      _setIndexMask((cacheSize / (associativity * lineSize)) - 1)
{

  ASSERTX(IsPower2(_lineSize));
  ASSERTX(IsPower2(_setIndexMask + 1));

  for (uint64_t accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
  {
    _access[accessType][false] = 0;
    _access[accessType][true] = 0;
  }
}

/*!    
 *  @brief Stats output method
 */

string CACHE_BASE::StatsLong(string prefix, CACHE_TYPE cache_type) const
{
  const uint64_t headerWidth = 19;
  const uint64_t numberWidth = 12;

  string out;

  out += prefix + _name + ":" + "\n";

  if (cache_type != CACHE_TYPE_ICACHE)
  {
    for (uint64_t i = 0; i < ACCESS_TYPE_NUM; i++)
    {
      const ACCESS_TYPE accessType = ACCESS_TYPE(i);

      std::string type(accessType == ACCESS_TYPE_LOAD ? "Load" : "Store");

      out += prefix + ljstr(type + "-Hits:      ", headerWidth) + mydecstr(Hits(accessType), numberWidth) +
             "  " + fltstr(100.0 * Hits(accessType) / Accesses(accessType), 2, 6) + "%\n";

      out += prefix + ljstr(type + "-Misses:    ", headerWidth) + mydecstr(Misses(accessType), numberWidth) +
             "  " + fltstr(100.0 * Misses(accessType) / Accesses(accessType), 2, 6) + "%\n";

      out += prefix + ljstr(type + "-Accesses:  ", headerWidth) + mydecstr(Accesses(accessType), numberWidth) +
             "  " + fltstr(100.0 * Accesses(accessType) / Accesses(accessType), 2, 6) + "%\n";

      out += prefix + "\n";
    }
  }

  out += prefix + ljstr("Total-Hits:      ", headerWidth) + mydecstr(Hits(), numberWidth) +
         "  " + fltstr(100.0 * Hits() / Accesses(), 2, 6) + "%\n";

  out += prefix + ljstr("Total-Misses:    ", headerWidth) + mydecstr(Misses(), numberWidth) +
         "  " + fltstr(100.0 * Misses() / Accesses(), 2, 6) + "%\n";

  out += prefix + ljstr("Total-Accesses:  ", headerWidth) + mydecstr(Accesses(), numberWidth) +
         "  " + fltstr(100.0 * Accesses() / Accesses(), 2, 6) + "%\n";
  out += "\n";

  return out;
}