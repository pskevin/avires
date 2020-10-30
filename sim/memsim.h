#ifndef MEMSIM_H__
#define MEMSIM_H__
#include "shared.h"

// Hardware 2-level TLB emulating Cascade Lake
struct tlbe {
  uint64_t	vpfn, ppfn;
  bool		present, hugepage;
};

void memsim_nanosleep(size_t sleeptime);

void perf_register(PerfCallback callback, size_t limit);

// delta is in ns
void add_runtime(size_t delta);

// From Wikipedia
static uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length);
static unsigned int tlb_hash(uint64_t addr);

void tlb_shootdown(uint64_t addr);

static struct tlbe *tlb_lookup(struct tlbe *tlb, unsigned int size, uint64_t vpfn);
static struct tlbe *alltlb_lookup(uint64_t vaddr, int *level);

static void tlb_insert(uint64_t vaddr, uint64_t paddr, unsigned int level);

static void memaccess(uint64_t addr, enum access_type type);

static void reset_stats(void);
static void print_stats(void);
#endif