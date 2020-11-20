/*
 * Simple memory allocator that only allocates fixed-size pages, allocates
 * physical memory linearly (first fast, then slow), and cannot free memory.
 */

#include "shared_new.h"
#include "mmgr_simple.h"
#include "memsim_new.h"
#include <stdlib.h>

#define PAGE_TYPE	HUGE_PAGE

uint64_t SimpleMemoryManager::getmem(uint64_t addr, struct pte *pte)
{
  uint64_t ret;

  if(fastmem < FASTMEM_SIZE) {
    ret = fastmem;
    fastmem += page_size(PAGE_TYPE);
  } else {
    assert(slowmem < SLOWMEM_SIZE);
    ret = slowmem | SLOWMEM_BIT;
    slowmem += page_size(PAGE_TYPE);
  }

  assert((ret & page_mask(PAGE_TYPE)) == 0);	// Must be aligned
  return ret;
}

pte * SimpleMemoryManager::alloc_ptables(uint64_t addr, enum pagetypes pt)
{
  struct pte *ptable = pml4, *pte;

  // Allocate page tables down to the leaf
  for(int i = 1; i < pt + 2; i++) {
    pte = &ptable[(addr >> (48 - (i * 9))) & 511];

    if(!pte->present) {
      pte->present = true;
      pte->next = (struct pte *)calloc(512, sizeof(struct pte));
    }

    ptable = pte->next;
  }

  // Return last-level PTE corresponding to addr
  return &ptable[(addr >> (48 - ((pt + 2) * 9))) & 511];
}

void SimpleMemoryManager::pagefault(uint64_t addr, bool readonly)
{
  assert(!readonly);
  // Allocate page tables
  struct pte *pte = alloc_ptables(addr, PAGE_TYPE);
  pte->present = true;
  pte->pagemap = true;

  pte->addr = getmem(addr, pte);
}

int SimpleMemoryManager::listnum(struct pte *pte)
{
  // Nothing to do
  return -1;
}

void SimpleMemoryManager::init(MemorySimulator& sim) {
  sim.setCR3(pml4);
}