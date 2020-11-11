#include <stdint.h>
#include <stddef.h>
#include <semaphore.h>
#include "shared_new.h"
#include "pin.H"

#ifndef MEMSIM_H
#define MEMSIM_H
class MemoryManager;
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

class MemorySimulator {
  public:
    void memaccess(uint64_t addr, memory_access_type type);
    void setMemoryManager(MemoryManager* mmgr);
    void setCR3(pte* ptr);
    
  MemorySimulator() {
    PIN_SemaphoreInit(&wakeup_sem);
    PIN_SemaphoreInit(&timebound_sem);
    PIN_MutexInit(&tlb_lock);
  }
  
  private:
    tlbe *alltlb_lookup(uint64_t vaddr, int *level);
    tlbe *tlb_lookup(uint64_t vaddr, int *level);
    tlbe *tlb_lookup(struct tlbe *tlb, unsigned int size,
            uint64_t vpfn);
    void add_runtime(size_t delta);
    void tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level);

    size_t tlbmisses = 0;
    size_t tlbhits = 0;
    size_t pagefaults = 0;
    size_t accesses[NMEMTYPES];
    size_t perf_limit = 0;
    size_t perf_accesses = 0;

    size_t runtime = 0;
    size_t wakeup_time = 0;
    size_t memsim_timebound = 0;
    bool	memsim_timebound_thread = false;
    PerfCallback perf_callback = NULL;
    
    PIN_SEMAPHORE wakeup_sem, timebound_sem;
    PIN_MUTEX tlb_lock;

    pte* cr3;
    MemoryManager* mmgr_;

    tlbe l1tlb_1g[4], l1tlb_2m[32], l1tlb_4k[64];
    tlbe l2tlb_1g[16], l2tlb_2m4k[1536];
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