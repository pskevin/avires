#ifndef MEMSIM_MODELS_PAGE_TABLE_ENTRY_H
#define MEMSIM_MODELS_PAGE_TABLE_ENTRY_H

#include "address.h"

struct PageTableEntry {
  // Hardware bits
  uint64_t addr;			// Page physical address, if pagemap
  struct PageTableEntry *next;			// Next page table pointer, if !pagemap
  
  // these bools should be "atomic"
  volatile bool Present;
  volatile bool Readonly;
  volatile bool Accessed;
  volatile bool Modified;
  volatile bool Pagemap;			// This PTE maps a page

  // // OS bits (16 bits available)
  // volatile bool migration;		// Range is under migration
  // volatile bool all_slow;		// All in slowmem
};

#endif