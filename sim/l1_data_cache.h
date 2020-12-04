#ifndef PIN_CACHE_64_H
#define PIN_CACHE_64_H

#include <sstream>
#include <string>
#include <iostream>

#include "inttypes.h"
#include "pin.H"

typedef uint64_t CACHE_STATS; // type of cache hit/miss counters

using std::cerr;
using std::string;
using std::endl;
using std::ostringstream;

/*!
 *  @brief Checks if n is a power of 2.
 *  @returns true if n is power of 2
 */
static inline bool IsPower2(uint64_t n)
{
  return ((n & (n - 1)) == 0);
}

/*!
 *  @brief Computes floor(log2(n))
 *  Works by finding position of MSB set.
 *  @returns -1 if n == 0.
 */
static inline uint64_t FloorLog2(uint64_t n)
{
  uint64_t p = 0;

  if (n == 0)
    return -1;

  if (n & 0xffffffff00000000)
  {
    p += 32;
    n >>= 32;
  }
  if (n & 0x00000000ffff0000)
  {
    p += 16;
    n >>= 16;
  }
  if (n & 0x000000000000ff00)
  {
    p += 8;
    n >>= 8;
  }
  if (n & 0x00000000000000f0)
  {
    p += 4;
    n >>= 4;
  }
  if (n & 0x000000000000000c)
  {
    p += 2;
    n >>= 2;
  }
  if (n & 0x0000000000000002)
  {
    p += 1;
  }

  return p;
}

/*!
 *  @brief Cache tag - self clearing on creation
 */
class CACHE_TAG
{
private:
  ADDRINT _tag;

public:
  CACHE_TAG(ADDRINT tag = 0) { _tag = tag; }
  bool operator==(const CACHE_TAG &right) const { return _tag == right._tag; }
  operator ADDRINT() const { return _tag; }
};

/*!
 * Everything related to cache sets
 */
namespace CACHE_SET
{

  /*!
 *  @brief Cache set direct mapped
 */
  class DIRECT_MAPPED
  {
  private:
    CACHE_TAG _tag;

  public:
    DIRECT_MAPPED(uint64_t associativity = 1) { ASSERTX(associativity == 1); }

    VOID SetAssociativity(uint64_t associativity) { ASSERTX(associativity == 1); }
    uint64_t GetAssociativity(uint64_t associativity) { return 1; }

    uint64_t Find(CACHE_TAG tag) { return (_tag == tag); }
    VOID Replace(CACHE_TAG tag) { _tag = tag; }
  };

  /*!
 *  @brief Cache set with round robin replacement
 */
  template <uint64_t MAX_ASSOCIATIVITY = 4>
  class ROUND_ROBIN
  {
  private:
    CACHE_TAG _tags[MAX_ASSOCIATIVITY];
    uint64_t _tagsLastIndex;
    uint64_t _nextReplaceIndex;

  public:
    ROUND_ROBIN(uint64_t associativity = MAX_ASSOCIATIVITY)
        : _tagsLastIndex(associativity - 1)
    {
      ASSERTX(associativity <= MAX_ASSOCIATIVITY);
      _nextReplaceIndex = _tagsLastIndex;

      for (uint64_t index = _tagsLastIndex; index >= 0; index--)
      {
        _tags[index] = CACHE_TAG(0);
      }
    }

    VOID SetAssociativity(uint64_t associativity)
    {
      ASSERTX(associativity <= MAX_ASSOCIATIVITY);
      _tagsLastIndex = associativity - 1;
      _nextReplaceIndex = _tagsLastIndex;
    }
    uint64_t GetAssociativity(uint64_t associativity) { return _tagsLastIndex + 1; }

    uint64_t Find(CACHE_TAG tag)
    {
      bool result = true;

      for (uint64_t index = _tagsLastIndex; index >= 0; index--)
      {
        // this is an ugly micro-optimization, but it does cause a
        // tighter assembly loop for ARM that way ...
        if (_tags[index] == tag)
          goto end;
      }
      result = false;

    end:
      return result;
    }

    VOID Replace(CACHE_TAG tag)
    {
      // g++ -O3 too dumb to do CSE on following lines?!
      const uint64_t index = _nextReplaceIndex;

      _tags[index] = tag;
      // condition typically faster than modulo
      _nextReplaceIndex = (index == 0 ? _tagsLastIndex : index - 1);
    }
  };

} // namespace CACHE_SET

namespace CACHE_ALLOC
{
  typedef enum
  {
    STORE_ALLOCATE,
    STORE_NO_ALLOCATE
  } STORE_ALLOCATION;
}

/*!
 *  @brief Generic cache base class; no allocate specialization, no cache set specialization
 */
class CACHE_BASE
{
public:
  // types, constants
  typedef enum
  {
    ACCESS_TYPE_LOAD,
    ACCESS_TYPE_STORE,
    ACCESS_TYPE_NUM
  } ACCESS_TYPE;

  typedef enum
  {
    CACHE_TYPE_ICACHE,
    CACHE_TYPE_DCACHE,
    CACHE_TYPE_NUM
  } CACHE_TYPE;

protected:
  static const uint64_t HIT_MISS_NUM = 2;
  CACHE_STATS _access[ACCESS_TYPE_NUM][HIT_MISS_NUM];

private: // input params
  const std::string _name;
  const uint64_t _cacheSize;
  const uint64_t _lineSize;
  const uint64_t _associativity;

  // computed params
  const uint64_t _lineShift;
  const uint64_t _setIndexMask;

  CACHE_STATS SumAccess(bool hit) const
  {
    CACHE_STATS sum = 0;

    for (uint64_t accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
    {
      sum += _access[accessType][hit];
    }

    return sum;
  }

protected:
  uint64_t NumSets() const { return _setIndexMask + 1; }

public:
  // constructors/destructors
  CACHE_BASE(std::string name, uint64_t cacheSize, uint64_t lineSize, uint64_t associativity);

  // accessors
  uint64_t CacheSize() const { return _cacheSize; }
  uint64_t LineSize() const { return _lineSize; }
  uint64_t Associativity() const { return _associativity; }
  //
  CACHE_STATS Hits(ACCESS_TYPE accessType) const { return _access[accessType][true]; }
  CACHE_STATS Misses(ACCESS_TYPE accessType) const { return _access[accessType][false]; }
  CACHE_STATS Accesses(ACCESS_TYPE accessType) const { return Hits(accessType) + Misses(accessType); }
  CACHE_STATS Hits() const { return SumAccess(true); }
  CACHE_STATS Misses() const { return SumAccess(false); }
  CACHE_STATS Accesses() const { return Hits() + Misses(); }

  VOID SplitAddress(const ADDRINT addr, CACHE_TAG &tag, uint64_t &setIndex) const
  {
    tag = addr >> _lineShift;
    setIndex = tag & _setIndexMask;
  }

  VOID SplitAddress(const ADDRINT addr, CACHE_TAG &tag, uint64_t &setIndex, uint64_t &lineIndex) const
  {
    const uint64_t lineMask = _lineSize - 1;
    lineIndex = addr & lineMask;
    SplitAddress(addr, tag, setIndex);
  }

  string StatsLong(string prefix = "", CACHE_TYPE = CACHE_TYPE_DCACHE) const;
};

/*!
 *  @brief Templated cache class with specific cache set allocation policies
 *
 *  All that remains to be done here is allocate and deallocate the right
 *  type of cache sets.
 */
template <class SET, uint64_t MAX_SETS, uint64_t STORE_ALLOCATION>
class CACHE : public CACHE_BASE
{
private:
  SET _sets[MAX_SETS];

public:
  // constructors/destructors
  CACHE(std::string name, uint64_t cacheSize, uint64_t lineSize, uint64_t associativity)
      : CACHE_BASE(name, cacheSize, lineSize, associativity)
  {
    printf("GOT IN CACHE CONSTRUCFTOR\n");
    ASSERTX(NumSets() <= MAX_SETS);

    for (uint64_t i = 0; i < NumSets(); i++)
    {
      _sets[i].SetAssociativity(associativity);
    }
  }

  // modifiers
  /// Cache access from addr to addr+size-1
  bool Access(ADDRINT addr, uint64_t size, ACCESS_TYPE accessType);
  /// Cache access at addr that does not span cache lines
  bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType);
};

/*!
 *  @return true if all accessed cache lines hit
 */

template <class SET, uint64_t MAX_SETS, uint64_t STORE_ALLOCATION>
bool CACHE<SET, MAX_SETS, STORE_ALLOCATION>::Access(ADDRINT addr, uint64_t size, ACCESS_TYPE accessType)
{
  const ADDRINT highAddr = addr + size;
  bool allHit = true;

  const ADDRINT lineSize = LineSize();
  const ADDRINT notLineMask = ~(lineSize - 1);
  do
  {
    CACHE_TAG tag;
    uint64_t setIndex;

    SplitAddress(addr, tag, setIndex);

    SET &set = _sets[setIndex];

    bool localHit = set.Find(tag);
    allHit &= localHit;

    // on miss, loads always allocate, stores optionally
    if ((!localHit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
    {
      set.Replace(tag);
    }

    addr = (addr & notLineMask) + lineSize; // start of next cache line
  } while (addr < highAddr);

  _access[accessType][allHit]++;

  return allHit;
}

/*!
 *  @return true if accessed cache line hits
 */
template <class SET, uint64_t MAX_SETS, uint64_t STORE_ALLOCATION>
bool CACHE<SET, MAX_SETS, STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType)
{
  CACHE_TAG tag;
  uint64_t setIndex;

  SplitAddress(addr, tag, setIndex);

  SET &set = _sets[setIndex];

  bool hit = set.Find(tag);

  // on miss, loads always allocate, stores optionally
  if ((!hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
  {
    set.Replace(tag);
  }

  _access[accessType][hit]++;

  return hit;
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

inline CACHE_BASE::CACHE_BASE(std::string name, uint64_t cacheSize, uint64_t lineSize, uint64_t associativity)
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

inline string CACHE_BASE::StatsLong(string prefix, CACHE_TYPE cache_type) const
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

#endif

#ifndef L1_DATA_CACHE_H
#define L1_DATA_CACHE_H
#include "memsim_new.h"
typedef uint64_t CACHE_STATS; // type of cache hit/miss counters
#include "pin_profile.H"

#include <sstream>
#include <string>
#include <iostream>
#define KILO 1024
#define MEGA (KILO * KILO)
#define GIGA (KILO * MEGA)

// define shortcuts
#define CACHE_DIRECT_MAPPED(MAX_SETS, ALLOCATION) CACHE<CACHE_SET::DIRECT_MAPPED, MAX_SETS, ALLOCATION>
#define CACHE_ROUND_ROBIN(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) CACHE<CACHE_SET::ROUND_ROBIN<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>

using std::cerr;
using std::endl;
using std::ostringstream;
using std::string;

// wrap configuation constants into their own name space to avoid name clashes
namespace DL1
{
  const uint64_t max_sets = KILO;         // cacheSize / (lineSize * associativity);
  const uint64_t max_associativity = 256; // associativity;
  const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

  typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace DL1

typedef enum
{
  COUNTER_MISS = 0,
  COUNTER_HIT = 1,
  COUNTER_NUM
} COUNTER;

typedef COUNTER_ARRAY<uint64_t, COUNTER_NUM> COUNTER_HIT_MISS;

class L1DataCache : public CacheManager
{
public:
  L1DataCache()
  {
    dl1 = new DL1::CACHE("L1 Data Cache", 32 * KILO, 64, 8);
  }
  bool cache_access(uint64_t vaddr, memory_access_type type, uint32_t size);

private:
  bool load_multi(uint64_t addr, uint32_t size);
  bool store_multi(uint64_t addr, uint32_t size);
  bool load_single(uint64_t addr);
  bool store_single(uint64_t addr);

  DL1::CACHE *dl1;

  // holds the counters with misses and hits
  // conceptually this is an array indexed by instruction address
  COMPRESSOR_COUNTER<ADDRINT, uint64_t, COUNTER_HIT_MISS> profile;
};

#endif // L1_DATA_CACHE_H