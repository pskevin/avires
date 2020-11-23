#include "memsim_new.h"

#ifndef FOUR_LEVEL_TLB_H
#define FOUR_LEVEL_TLB_H

class FourLevelTLB : public TLB {
  public:
  FourLevelTLB() {
      PIN_MutexInit(&tlb_lock);
    }
    tlbe *alltlb_lookup(uint64_t vaddr, int *level);
    tlbe *tlb_lookup(uint64_t vaddr, int *level);
    tlbe *tlb_lookup(struct tlbe *tlb, unsigned int size,
            uint64_t vpfn);
    void tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level);
    void shootdown(uint64_t addr);

  private:
    PIN_MUTEX tlb_lock;

    tlbe l1tlb_1g[4], l1tlb_2m[32], l1tlb_4k[64];
    tlbe l2tlb_1g[16], l2tlb_2m4k[1536];
};

#endif