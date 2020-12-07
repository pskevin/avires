#include "two_level_tlb.h"

tlbe* FourLevelTLB::alltlb_lookup(uint64_t vaddr, int* level) {
  tlbe *ret = NULL;

  // 1G L1 TLB
  ret = tlb_lookup(l1tlb_1g, 4, vaddr & GIGA_PFN_MASK);
  if(ret != NULL) {
    *level = 2;
    return ret;
  }

  // 2M L1 TLB
  ret = tlb_lookup(l1tlb_2m, 32, vaddr & HUGE_PFN_MASK);
  if(ret != NULL) {
    *level = 3;
    return ret;
  }

  // 4K L1 TLB
  ret = tlb_lookup(l1tlb_4k, 64, vaddr & BASE_PFN_MASK);
  if(ret != NULL) {
    *level = 4;
    return ret;
  }

  // 1G L2 TLB
  ret = tlb_lookup(l2tlb_1g, 16, vaddr & GIGA_PFN_MASK);
  if(ret != NULL) {
    *level = 2;
    return ret;
  }

  // 2M L2 TLB
  ret = tlb_lookup(l2tlb_2m4k, 1536, vaddr & HUGE_PFN_MASK);
  if(ret != NULL && ret->hugepage) {
    *level = 3;
    return ret;
  }

  ret = tlb_lookup(l2tlb_2m4k, 1536, vaddr & BASE_PFN_MASK);
  if(ret != NULL && !ret->hugepage) {
    *level = 4;
    return ret;
  }

  return NULL;
}

tlbe* FourLevelTLB::tlb_lookup(struct tlbe *tlb, unsigned int size, uint64_t vpfn)
{
  tlbe *ret;
  
  PIN_MutexLock(&tlb_lock);
  struct tlbe *te = &tlb[tlb_hash(vpfn) % size];
  if(te->present && te->vpfn == vpfn) {
    ret = te;
  } else {
    ret = NULL;
  }

  PIN_MutexUnlock(&tlb_lock);
  return ret;
}


void FourLevelTLB::tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level)
{
  struct tlbe *te;
  uint64_t vpfn = 0, ppfn = 0;

  assert(level >= 2 && level <= 4);

  PIN_MutexLock(&tlb_lock);

  switch(level) {
  case 2:	// 1GB page
    vpfn = vaddr & GIGA_PFN_MASK;
    ppfn = paddr & GIGA_PFN_MASK;
    te = &l1tlb_1g[tlb_hash(vpfn) % 4];
    if(te->present) {
      // Move previous entry down
      assert(te->vpfn != vpfn);
      memcpy(&l2tlb_1g[tlb_hash(vpfn) % 16], te, sizeof(struct tlbe));
    }
    break;

  case 3:	// 2MB page
    vpfn = vaddr & HUGE_PFN_MASK;
    ppfn = paddr & HUGE_PFN_MASK;
    te = &l1tlb_2m[tlb_hash(vpfn) % 32];
    te->hugepage = true;

    // Fall through...
  case 4:	// 4KB page
    if(level == 4) {
      vpfn = vaddr & BASE_PFN_MASK;
      ppfn = paddr & BASE_PFN_MASK;
      te = &l1tlb_4k[tlb_hash(vpfn) % 64];
      te->hugepage = false;
    }
    if(te->present) {
      // Move previous entry down
      assert(te->vpfn != vpfn);
      memcpy(&l2tlb_2m4k[tlb_hash(vpfn) % 1536], te, sizeof(struct tlbe));
    }
    break;
  }

  te->present = true;
  te->vpfn = vpfn;
  te->ppfn = ppfn;
  
  PIN_MutexUnlock(&tlb_lock);
}

void FourLevelTLB::shootdown(uint64_t addr) {
  PIN_MutexLock(&tlb_lock);
  memset(l1tlb_1g, 0, sizeof(l1tlb_1g));
  memset(l1tlb_2m, 0, sizeof(l1tlb_2m));
  memset(l1tlb_4k, 0, sizeof(l1tlb_4k));
  memset(l2tlb_1g, 0, sizeof(l2tlb_1g));
  memset(l2tlb_2m4k, 0, sizeof(l2tlb_2m4k));
  PIN_MutexUnlock(&tlb_lock);
}