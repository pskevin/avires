#ifndef MEMSIM_UTILS_MACROS_H
#define MEMSIM_UTILS_MACROS_H

#include <cstdint>

// Handy size macros
#define KB(x) (((uint64_t)x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)
#define TB(x) (GB(x) * 1024)

// Handy time macros
#define US(x) (((uint64_t)x) * 1000)
#define MS(x) (US(x) * 1000)
#define S(x) (MS(x) * 1000)

#if (MEMSIM_LOGGING == 1)
#include <cstdarg>
#include <iostream>

using std::cerr;
using std::endl;

void MemsimLog(int line, const char *fileName, const char *functionName, const char *formattedMessage, ...);
void MemsimLog(int line, const char *fileName, const char *functionName);

#define LogMessage(msg, ...) \
    MemsimLog(__LINE__, __FILE__, __FUNCTION__,  msg, ##__VA_ARGS__)

#define LogPoint() \
    MemsimLog(__LINE__, __FILE__, __FUNCTION__)

#else

#define LogMessage(msg, ...) \
    while(0) {}

#define LogPoint() \
    while(0) {}
#endif


/*
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
#define TIME_FASTMEM_READ	81
#define TIME_FASTMEM_WRITE	86

#define TIME_SLOWMEM_READ	169		// From DCPMM QoS slides
#define TIME_SLOWMEM_WRITE 90	

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


*/

#endif