#ifndef GUPS_H
#define GUPS_H

#include <stdint.h>

#define MAX_THREADS 64

enum access_pattern
{
    RANDOM,
    SEQUENTIAL,
    ZIPFIAN,
    NACCESSPATTERNS,
};

enum access_type
{
    READ,
    WRITE,
    NACCESSTYPES,
};

uint64_t next_index(uint64_t seed);

#if (PATTERN == 0)
static enum access_pattern ACCESS_PATTERN = RANDOM;
#elif (PATTERN == 1)
static enum access_pattern ACCESS_PATTERN = SEQUENTIAL;
// uint64_t next_index(uint64_t seed)
// {
//     while (0){};
// }
#else
static enum access_pattern ACCESS_PATTERN = ZIPFIAN;
#endif

#if (TYPE == 0)
static enum access_type ACCESS_TYPE = READ;
#else
static enum access_type ACCESS_TYPE = WRITE;
#endif

void calc_indices(unsigned long *indices, unsigned long updates, unsigned long nelems);

#endif