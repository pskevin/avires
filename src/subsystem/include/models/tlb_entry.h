#ifndef MEMSIM_MODELS_TLB_ENTRY_H
#define MEMSIM_MODELS_TLB_ENTRY_H

#include <cstdint>

namespace TLB
{
    struct Entry
    {
        uint64_t vpfn, ppfn;
        bool present, hugepage;
    };
}

#endif