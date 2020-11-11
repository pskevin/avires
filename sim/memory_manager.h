#include <stdint.h>
#include "shared_new.h"
#include "memsim_new.h"

#ifndef MMGR_H
#define MMGR_H
class MemoryManager {
  public:
    virtual uint64_t getmem(uint64_t addr, pte *pte);
    virtual pte *alloc_ptables(uint64_t addr, enum pagetypes pt);
    virtual void pagefault(uint64_t addr, bool readonly);
    virtual int listnum(pte* pte);
    virtual void init(MemorySimulator& sim);
};
#endif