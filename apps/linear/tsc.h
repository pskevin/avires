#ifndef TSC_H
#define TSC_H

#include <cstdint>
#include <cstdlib>
#include <x86intrin.h>

#ifndef TSC
    #define TSC 4
#endif

#if TSC == 0
#pragma message("Preprocessor set to use Inline ASM to read TSC.")

uint64_t read_tsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

#elif TSC >= 1 && TSC <= 3
#pragma message("Preprocessor set to use intrinsic to read TSC.")

inline uint64_t read_tsc()
{
    #if TSC >= 1
    _mm_lfence(); // optionally wait for earlier insns to retire before reading the clock
    #endif
    
    uint64_t tsc = __rdtsc();
    
    #if TSC == 3
    _mm_lfence(); // optionally block later instructions until rdtsc retires
    #endif
    
    return tsc;
}

#else
#pragma message("Preprocessor set to use intrinsic to serially read TSC.")

// requires a Nehalem or newer CPU.  Not Core2 or earlier.  IDK when AMD added it.
inline uint64_t read_tsc()
{
    unsigned val;
    return __rdtscp(&val); // waits for earlier insns to retire, but allows later to start
}

#endif

#endif