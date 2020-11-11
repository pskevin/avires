
#include "memory_manager.h"

class SimpleMemoryManager: public MemoryManager {
  public:
    uint64_t getmem(uint64_t addr, pte *pte);
    pte *alloc_ptables(uint64_t addr, enum pagetypes pt);
    void pagefault(uint64_t addr, bool readonly);
    int listnum(pte* pte);
    void init(MemorySimulator& sim);

  private:
    pte pml4[512];
    uint64_t fastmem = 0, slowmem = 0;
};