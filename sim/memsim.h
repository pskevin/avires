#ifndef MEMSIM_H__
#define MEMSIM_H__

// Memory access type
enum access_type {
  TYPE_READ,
  TYPE_WRITE,
};


// Hardware 2-level TLB emulating Cascade Lake
struct tlbe {
  uint64_t	vpfn, ppfn;
  bool		present, hugepage;
};

void memaccess(uint64_t addr, enum access_type type);

#endif