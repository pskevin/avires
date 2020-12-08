#include "memsim_new.h"

class SimpleMemoryManager: public MemoryManager {
  public:
    void pagefault(uint64_t addr, bool readonly);
    void init(MemorySimulator* sim);
    SimpleMemoryManager(enum pagetypes pt) : pt_(pt) {};

  private:
    pte *alloc_ptables(uint64_t addr, enum pagetypes pt);
    uint64_t getmem(uint64_t addr, pte *pte);
    int listnum(pte* pte);
    
    pte pml4[512];
    MemorySimulator* sim_;
    uint64_t fastmem = 0, slowmem = 0;
    enum pagetypes pt_;
};