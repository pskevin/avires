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
  COUNTER_CACHE_MISS = 0,
  COUNTER_CACHE_HIT = 1,
  COUNTER_CACHE_NUM
} CACHE_COUNTER;

typedef enum
{
  COUNTER_TLB_MISS = 0,
  COUNTER_TLB_HIT = 1,
  COUNTER_TLB_SHOOTDOWN = 2,
  COUNTER_TLB_NUM
} TLB_COUNTER;

typedef enum {
  COUNTER_PAGEFAULT = 0,
  COUNTER_SLOWMEM = 1,
  COUNTER_FASTMEM = 2,
  COUNTER_MEM_NUM
} MEM_COUNTER;

typedef COUNTER_ARRAY<uint64_t, COUNTER_CACHE_NUM> COUNTER_CACHE;
typedef COUNTER_ARRAY<uint64_t, COUNTER_TLB_NUM> COUNTER_TLB;

// Pagefault counters only have one number to track
typedef COUNTER_ARRAY<uint64_t, COUNTER_MEM_NUM> COUNTER_PAGEFAULTS;

class MemorySimulator {
  public:
    void memaccess(uint64_t addr, memory_access_type type, uint32_t size, uint64_t timestep);
    void setCR3(pte* ptr);
    void tlb_shootdown(uint64_t addr);
    void add_runtime(size_t delta);
    void memsim_nanosleep(size_t sleeptime);
    void PrintInstructionProfiles();
    void PrintAggregateProfiles();
    void WriteStatsFiles(std::string out_prefix);
    CacheManager* GetCacheManager();
    MemoryManager* GetMemoryManager();
    volatile size_t runtime = 0;

  MemorySimulator(MemoryManager* mgr, TLB* tlb, CacheManager* cache, COUNTER_CACHE cache_threshold, COUNTER_TLB tlb_threshold, COUNTER_PAGEFAULTS pf_threshold) :
    mmgr_(mgr), tlb_(tlb), cache_(cache),
    cache_agg_profile(COUNTER_CACHE_NUM, 0), tlb_agg_profile(COUNTER_TLB_NUM, 0), mmgr_agg_profile(COUNTER_MEM_NUM, 0) {
    PIN_SemaphoreInit(&wakeup_sem);
    PIN_SemaphoreInit(&timebound_sem);

    memsim_timebound_thread = OS_TlsAlloc(NULL);
    OS_TlsSetValue(memsim_timebound_thread, reinterpret_cast<void *> (static_cast<int> (false)));

    tlb_profile.SetKeyName("vaddr          ");
    tlb_profile.SetCounterName("tlb:miss        tlb:hit");

    cache_profile.SetKeyName("vaddr          ");
    cache_profile.SetCounterName("dcache:miss        dcache:hit");

    mmgr_profile.SetKeyName("vaddr          ");
    mmgr_profile.SetCounterName("mmgr:pagefault        mmgr:slowmem        mmgr:fastmem");

    cache_profile.SetThreshold(cache_threshold);
    tlb_profile.SetThreshold(tlb_threshold);
    mmgr_profile.SetThreshold(pf_threshold);
  }
  
  private:
    uint64_t walk_page_table(uint64_t addr, memory_access_type type, uint64_t timestep, int &level);
    // these should be atomic (maybe need mutexes?)
    volatile size_t wakeup_time = 0;
    volatile size_t memsim_timebound = 0;
    
    PIN_SEMAPHORE wakeup_sem, timebound_sem;
    
    pte* cr3;
    MemoryManager* mmgr_;
    TLB* tlb_;
    CacheManager* cache_;

    std::vector<uint64_t> cache_agg_profile;
    std::vector<uint64_t> tlb_agg_profile;
    std::vector<uint64_t> mmgr_agg_profile;
    std::vector<uint64_t> v_addrs;

    // holds the counters with misses and hits
    // conceptually this is an array indexed by instruction address
    COMPRESSOR_COUNTER<uint64_t, uint64_t, COUNTER_CACHE> cache_profile;
    COMPRESSOR_COUNTER<uint64_t, uint64_t, COUNTER_TLB> tlb_profile;
    // holds the counter for pagefaults
    COMPRESSOR_COUNTER<uint64_t, uint64_t, COUNTER_PAGEFAULTS> mmgr_profile;

    PIN_TLS_INDEX memsim_timebound_thread;
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