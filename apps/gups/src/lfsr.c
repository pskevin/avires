#include "gups.h"

#include <stdint.h>

static uint64_t lfsr_fast(uint64_t lfsr)
{
    lfsr ^= lfsr >> 7;
    lfsr ^= lfsr << 9;
    lfsr ^= lfsr >> 13;
    return lfsr;
}


uint64_t next_index(uint64_t seed)
{
    return lfsr_fast(seed);
}
