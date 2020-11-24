#include <stdint.h>
#include <stddef.h>
#include <semaphore.h>
#include "shared_new.h"
#include "pin.H"

#ifndef MEMSIM_H
#define MEMSIM_H

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
typedef struct cacheentry {
  uint64_t	vpfn, ppfn;
  bool  present;
} cacheentry;

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
    virtual cacheentry* cache_lookup(uint64_t vaddr, int *level);
};

class MemorySimulator {
  public:
    void memaccess(uint64_t addr, memory_access_type type);
    void setCR3(pte* ptr);
    void tlb_shootdown(uint64_t addr);
    void add_runtime(size_t delta);
    void memsim_nanosleep(size_t sleeptime);
    size_t runtime = 0;
    
  MemorySimulator(MemoryManager* mgr, TLB* tlb) : mmgr_(mgr), tlb_(tlb), cache_(NULL) {
    PIN_SemaphoreInit(&wakeup_sem);
    PIN_SemaphoreInit(&timebound_sem);
  }
  
  private:
    uint64_t walk_page_table(uint64_t addr, memory_access_type type, int &level);

    size_t tlbmisses = 0;
    size_t tlbhits = 0;
    size_t tlbshootdowns = 0;
    size_t pagefaults = 0;
    size_t accesses[NMEMTYPES];
    size_t perf_limit = 0;
    size_t perf_accesses = 0;

    size_t wakeup_time = 0;
    size_t memsim_timebound = 0;
    bool	memsim_timebound_thread = false;
    PerfCallback perf_callback = NULL;
    
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