#include "memsim_new.h"
#include <queue>

struct page {
  uint64_t	framenum;
  struct pte	*pte;
};

typedef std::pair<uint64_t, struct page*> page_pair;

struct pair_cmp {
  bool operator()(const page_pair& p1, const page_pair& p2) {
    return p1.first < p2.first;
  }
};
typedef std::priority_queue<page_pair, std::vector<page_pair>, pair_cmp> lru_queue;

class LRUMemoryManager: public MemoryManager {
  public:
    void pagefault(uint64_t addr, bool readonly);
    void init(MemorySimulator* sim);

  private:
    pte *alloc_ptables(uint64_t addr, enum pagetypes pt);


    uint64_t getmem(uint64_t addr, pte *pte);
    int listnum(pte* pte);
    uint64_t enqueue_fast(uint64_t addr, struct pte *pte, struct pte* popped);
    struct pte* enqueue_slow(uint64_t addr, struct pte *pte);
    
    pte pml4[512];

    // used to decide which pages are kept in fast/slow memory
    lru_queue fastq;
    lru_queue slowq;

    uint64_t fastmem = 0, slowmem = 0;
    uint64_t access_ctr = 0;
};