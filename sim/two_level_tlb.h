#include "memsim_new.h"

#ifndef TWO_LEVEL_TLB_H
#define TWO_LEVEL_TLB_H

// Hardware 2-level TLB emulating Cascade Lake
class TwoLevelTLB : public TLB
{
public:
  TwoLevelTLB()
  {
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

  tlbe l1tlb_1g[TLB_L1_GIGA_ENTRIES], l1tlb_2m[TLB_L1_HUGE_ENTRIES], l1tlb_4k[TLB_L1_BASE_ENTRIES];
  tlbe l2tlb_1g[TLB_L2_GIGA_ENTRIES], l2tlb_2m4k[TLB_L2_HUGE_BASE_ENTRIES];
};

#endif