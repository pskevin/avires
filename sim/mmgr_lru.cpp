/*
 * Simple memory allocator that only allocates fixed-size pages, allocates
 * physical memory linearly (first fast, then slow), and cannot free memory.
 */

#include "shared_new.h"
#include "mmgr_lru.h"
#include "memsim_new.h"
#include <stdlib.h>

#define PAGE_TYPE	HUGE_PAGE

// adds a new item to the priority queue, returns its framenum, and populates info for the pte that was popped off.
uint64_t LRUMemoryManager::enqueue_fast(uint64_t addr, struct pte *pte, struct pte* popped) {
  // put this page in memory where the old page is being evicted
  page_pair p = fastq.top();
  popped = p.second->pte;
  p.second->pte = pte;
  p.first = ++access_ctr;
  fastq.pop();
  fastq.push(p);

  return p.second->framenum;
}

// adds a new item to the slow queue, and returns the pte that was popped off, which is no longer in the page table;
struct pte* LRUMemoryManager::enqueue_slow(uint64_t addr, struct pte *pte) {
  // put this page in memory where the old page is being evicted
  page_pair p = slowq.top();
  struct pte* ret = p.second->pte;
  p.second->pte = pte;
  p.second->pte->addr = p.second->framenum * (PAGE_TYPE) | SLOWMEM_BIT;
  p.first = ++access_ctr;
  slowq.pop();
  slowq.push(p);

  return ret;
}

uint64_t LRUMemoryManager::getmem(uint64_t addr, struct pte *pte)
{
  uint64_t ret;

  struct pte* tmp_pte = nullptr;
  uint64_t framenum = enqueue_fast(addr, pte, tmp_pte);
  
  tmp_pte = enqueue_slow(addr, tmp_pte);
  // remove this page / mark as non-present
  tmp_pte->present = false;
  tmp_pte->pagemap = false;

  ret = framenum * (PAGE_TYPE);
  assert((ret & page_mask(PAGE_TYPE)) == 0);	// Must be aligned
  return ret;
}

pte * LRUMemoryManager::alloc_ptables(uint64_t addr, enum pagetypes pt)
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

void LRUMemoryManager::pagefault(uint64_t addr, bool readonly)
{
  assert(!readonly);
  // Allocate page tables
  struct pte *pte = alloc_ptables(addr, PAGE_TYPE);
  pte->present = true;
  pte->pagemap = true;
  pte->addr = getmem(addr, pte);
}

int LRUMemoryManager::listnum(struct pte *pte)
{
  // Nothing to do
  return -1;
}

void LRUMemoryManager::init(MemorySimulator* sim) {
  sim->setCR3(pml4);
  const uint64_t fastmem_pages = FASTMEM_SIZE / page_size(PAGE_TYPE);
  struct page *p = (page*)calloc(fastmem_pages, sizeof(struct page));
  for(uint32_t i = 0; i < fastmem_pages; i++) {
    p[i].framenum = i;
    fastq.push(std::make_pair(access_ctr, &p[i]));
  }
  const uint64_t slowmem_pages = FASTMEM_SIZE / page_size(PAGE_TYPE);
  p = (page*) calloc(slowmem_pages, sizeof(struct page));
  for(uint32_t i = 0; i < slowmem_pages; i++) {
    p[i].framenum = i;
    slowq.push(std::make_pair(access_ctr, &p[i]));
  }
}