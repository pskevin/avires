#ifndef MEMSIM_H
#define MEMSIM_H
#include <stdint.h>
// typedef uint64_t CACHE_STATS; // type of cache hit/miss counters
#include <stddef.h>
#include <semaphore.h>
#include <sstream>      // std::stringstream
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

struct ProfileCounter {
  // indexed first by timestep, then by label
  std::vector<std::vector<uint64_t>> counts;
  std::vector<std::string> labels;
  std::map<std::string, size_t> label_map;
  PIN_MUTEX counter_mutex;

  ProfileCounter(std::vector<std::string> l) {
    for(size_t i = 0; i < l.size(); i++) {
      label_map[l[i]] = i;
    }
    labels = l;
    PIN_MutexInit(&counter_mutex);
  }
  
  void Grow(uint64_t t) {
    while(counts.size() <= t) {
      counts.push_back(std::vector<uint64_t>(labels.size(), 0));
    }
  }

  void Increment(uint64_t t, std::string label) {
    PIN_MutexLock(&counter_mutex);
    Grow(t);
    counts[t][label_map[label]] += 1;
    PIN_MutexUnlock(&counter_mutex);
  }

  std::string String() {
    std::stringstream ss;
    for(const auto& l : labels) {
      ss << l << ", ";
    }
    ss << std::endl;
    for(const auto& c : counts) {
      for(const auto& l : labels) {
        ss << c[label_map[l]] << ", ";
      }
      ss << std::endl;
    }

    return ss.str();
  }

  std::string AggregateString() {
    std::vector<uint64_t> aggregate_counts(labels.size(), 0);
    std::stringstream ss;

    for(const auto& c : counts) {
      for(const auto& l : labels) {
        aggregate_counts[label_map[l]] += c[label_map[l]];
      }
    }

    for(const auto& l : labels) {
      ss << l << ", ";
    }
    ss << std::endl;

    for(const auto& l : labels) {
      ss << aggregate_counts[label_map[l]] << ", ";
    }

    return ss.str();
  }
};

const std::string cache_profile_options[] = { "MISS", "HIT" };
const std::string tlb_profile_options[] = { "MISS", "HIT" };
const std::string mmgr_profile_options[] = { "FASTMEM", "SLOWMEM", "PAGEFAULT" };

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

  MemorySimulator(MemoryManager* mgr, TLB* tlb, CacheManager* cache) :
    mmgr_(mgr), tlb_(tlb), cache_(cache),
    cache_agg_profile(2, 0), tlb_agg_profile(3, 0), mmgr_agg_profile(3, 0),
    cache_profile(std::vector<std::string>(cache_profile_options, cache_profile_options + sizeof(cache_profile_options) / sizeof(std::string) )),
    tlb_profile(std::vector<std::string>(tlb_profile_options, tlb_profile_options + sizeof(tlb_profile_options) / sizeof(std::string) )),
    mmgr_profile(std::vector<std::string>(mmgr_profile_options, mmgr_profile_options + sizeof(mmgr_profile_options) / sizeof(std::string) )) {
    PIN_SemaphoreInit(&wakeup_sem);
    PIN_SemaphoreInit(&timebound_sem);

    memsim_timebound_thread = OS_TlsAlloc(NULL);
    OS_TlsSetValue(memsim_timebound_thread, reinterpret_cast<void *> (static_cast<int> (false)));
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

    ProfileCounter cache_profile;
    ProfileCounter tlb_profile;
    ProfileCounter mmgr_profile;

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