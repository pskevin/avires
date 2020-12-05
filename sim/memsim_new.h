#ifndef MEMSIM_H
#define MEMSIM_H
#include <stdint.h>
// typedef uint64_t CACHE_STATS; // type of cache hit/miss counters
#include <stddef.h>
#include <semaphore.h>
#include "shared_new.h"
#include "pin.H"
#include "pin_profile.H"

using namespace std;

// Memory access type
typedef enum access_type {
  TYPE_READ,
  TYPE_WRITE,
} memory_access_type;

// Hardware 2-level TLB emulating Cascade Lake
typedef struct tlbe {
  uint64_t	vpfn, ppfn;
  bool		present, hugepage;
} tlbe;

// Software cache entry;
// typedef struct cacheentry {
//   uint64_t	vpfn, ppfn;
//   bool  present;
// } cacheentry;

class MemorySimulator;

class MemoryManager {
  public:
    virtual uint64_t getmem(uint64_t addr, pte *pte);
    virtual pte *alloc_ptables(uint64_t addr, enum pagetypes pt);
    virtual void pagefault(uint64_t addr, bool readonly);
    virtual int listnum(pte* pte);
    virtual void init(MemorySimulator* sim);
};

class TLB {
  public:
    virtual tlbe *alltlb_lookup(uint64_t vaddr, int *level);
    virtual void tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level);
    virtual void shootdown(uint64_t addr);
};

class CacheManager {
  public:
    virtual bool cache_access(uint64_t vaddr, memory_access_type type, uint32_t size);
};

typedef enum
{
  COUNTER_MISS = 0,
  COUNTER_HIT = 1,
  COUNTER_NUM
} COUNTER;

typedef COUNTER_ARRAY<uint64_t, COUNTER_NUM> COUNTER_HIT_MISS;

class MemorySimulator {
  public:
    void memaccess(uint64_t addr, memory_access_type type, uint32_t size, uint64_t insid);
    void setCR3(pte* ptr);
    void tlb_shootdown(uint64_t addr);
    void add_runtime(size_t delta);
    void memsim_nanosleep(size_t sleeptime);
    size_t runtime = 0;

    // holds the counters with misses and hits
    // conceptually this is an array indexed by instruction address
    COMPRESSOR_COUNTER<ADDRINT, uint64_t, COUNTER_HIT_MISS> cache_profile;
    COMPRESSOR_COUNTER<ADDRINT, uint64_t, COUNTER_HIT_MISS> mmgr_profile;
    COMPRESSOR_COUNTER<ADDRINT, uint64_t, COUNTER_HIT_MISS> tlb_profile;

  MemorySimulator(MemoryManager* mgr, TLB* tlb, CacheManager* cache, COUNTER_HIT_MISS profile_threshold) : mmgr_(mgr), tlb_(tlb), cache_(cache) {
    PIN_SemaphoreInit(&wakeup_sem);
    PIN_SemaphoreInit(&timebound_sem);

    tlb_profile.SetKeyName("iaddr          ");
    tlb_profile.SetCounterName("tlb:miss        tlb:hit");

    mmgr_profile.SetKeyName("iaddr          ");
    mmgr_profile.SetCounterName("mmgr:miss        mmgr:hit");

    cache_profile.SetKeyName("iaddr          ");
    cache_profile.SetCounterName("dcache:miss        dcache:hit");

    cache_profile.SetThreshold(profile_threshold);
    mmgr_profile.SetThreshold(profile_threshold);
    tlb_profile.SetThreshold(profile_threshold);
  }
  
  private:
    uint64_t walk_page_table(uint64_t addr, memory_access_type type, int &level);

    size_t pagefaults = 0;
    size_t accesses[NMEMTYPES];
    size_t wakeup_time = 0;
    size_t memsim_timebound = 0;
    bool	memsim_timebound_thread = false;
    
    PIN_SEMAPHORE wakeup_sem, timebound_sem;
    
    pte* cr3;
    MemoryManager* mmgr_;
    TLB* tlb_;
    CacheManager* cache_;
};

// From Wikipedia
static uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  
  return hash;
}

inline unsigned int tlb_hash(uint64_t addr)
{
  return jenkins_one_at_a_time_hash((uint8_t *)&addr, sizeof(uint64_t));
}

#endif