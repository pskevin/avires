#include <stdint.h>
#include <stddef.h>
// #include <atomic>
#include <assert.h>
#include "inttypes.h"

#ifndef MEMSIM_SHARED_H
#define MEMSIM_SHARED_H

// Handy size macros
#define KB(x)		(((uint64_t)x) * 1024)
#define MB(x)		(KB(x) * 1024)
#define GB(x)		(MB(x) * 1024)
#define TB(x)		(GB(x) * 1024)

// Handy time macros
#define US(x)		(((uint64_t)x) * 1000)
#define MS(x)		(US(x) * 1000)
#define S(x)		(MS(x) * 1000)

// Page sizes
#define BASE_PAGE_SIZE	KB(1)
#define HUGE_PAGE_SIZE	MB(2)
#define GIGA_PAGE_SIZE	GB(1)

// Page offset masks
#define BASE_PAGE_MASK	(BASE_PAGE_SIZE - 1)
#define HUGE_PAGE_MASK	(HUGE_PAGE_SIZE - 1)
#define GIGA_PAGE_MASK	(GIGA_PAGE_SIZE - 1)

// Page frame number masks
#define BASE_PFN_MASK	(BASE_PAGE_MASK ^ UINT64_MAX)
#define HUGE_PFN_MASK	(HUGE_PAGE_MASK ^ UINT64_MAX)
#define GIGA_PFN_MASK	(GIGA_PAGE_MASK ^ UINT64_MAX)

// Physical memory sizes in bytes
#define FASTMEM_SIZE	KB(8)
#define SLOWMEM_SIZE	MB(512)
#define CACHELINE_SIZE	64
#define MMM_LINE_SIZE	256

#define TLB_L1_GIGA_ENTRIES 1
#define TLB_L1_HUGE_ENTRIES 4
#define TLB_L1_BASE_ENTRIES 8
#define TLB_L2_GIGA_ENTRIES 4
#define TLB_L2_HUGE_BASE_ENTRIES 32

// Simulated execution times in ns
#define TIME_PAGEFAULT		2000		// pagefault interrupt
#define TIME_PAGEWALK		200		// 1 level of pagewalk
#define TIME_SLOWMOVE		2000		// Move to slow memory
#define TIME_FASTMOVE		1000		// Move to fast memory
#define TIME_TLBSHOOTDOWN	4000		// TLB shootdown

// From Intel memory latency checker
#define TIME_FASTMEM_READ	82
#define TIME_FASTMEM_WRITE	82

#define TIME_SLOWMEM_READ	1000		// From DCPMM QoS slides
#define TIME_SLOWMEM_WRITE	1000		// maybe worse?

#define TIME_CACHE_READ	1
#define TIME_CACHE_WRITE 2

// Fake offset for slowmem in physical memory
#define SLOWMEM_BIT	((uint64_t)1 << 63)
#define SLOWMEM_MASK	(((uint64_t)1 << 63) - 1)

#ifdef LOG_DEBUG
#	define MEMSIM_LOG(str, runtime, ...)	fprintf(stderr, "%.2f " str, (double)runtime / 1000000000.0, ##__VA_ARGS__)
#else
#	define MEMSIM_LOG(std, ...)	while(0) {}
#endif

enum memtypes {
  FASTMEM = 0,
  SLOWMEM = 1,
  NMEMTYPES,
};

enum pagetypes {
  GIGA_PAGE = 0,
  HUGE_PAGE,
  BASE_PAGE,
  NPAGETYPES
};

typedef struct pte {
  // Hardware bits
  uint64_t addr;			// Page physical address, if pagemap
  struct pte *next;			// Next page table pointer, if !pagemap
  
  // these bools should be "atomic"
  volatile bool present;
  volatile bool readonly;
  volatile bool accessed;
  volatile bool modified;
  volatile bool pagemap;			// This PTE maps a page

  // OS bits (16 bits available)
  volatile bool migration;		// Range is under migration
  volatile bool all_slow;		// All in slowmem

  // Statistics
  size_t ups, downs;
} pte;

static inline uint64_t page_size(enum pagetypes pt)
{
  switch(pt) {
  case GIGA_PAGE: return GIGA_PAGE_SIZE;
  case HUGE_PAGE: return HUGE_PAGE_SIZE;
  case BASE_PAGE: return BASE_PAGE_SIZE;
  default: assert(!"Unknown page type");
  }
}

static inline uint64_t pfn_mask(enum pagetypes pt)
{
  switch(pt) {
  case GIGA_PAGE: return GIGA_PFN_MASK;
  case HUGE_PAGE: return HUGE_PFN_MASK;
  case BASE_PAGE: return BASE_PFN_MASK;
  default: assert(!"Unknown page type");
  }
}

static inline uint64_t page_mask(enum pagetypes pt)
{
  switch(pt) {
  case GIGA_PAGE: return GIGA_PAGE_MASK;
  case HUGE_PAGE: return HUGE_PAGE_MASK;
  case BASE_PAGE: return BASE_PAGE_MASK;
  default: assert(!"Unknown page type");
  }
}


#endif

hotset_linux rndm_write_linux seq_write_linux hotset_simple rndm_write_simple seq_write_simple rndm_read_linux seq_read_linux rndm_read_simple seq_read_simple