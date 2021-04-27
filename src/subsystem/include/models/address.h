#ifndef MEMSIM_MODELS_ADDRESS_H
#define MEMSIM_MODELS_ADDRESS_H

#include <iterator>
#include <vector>
#include <map>
#include <string>
#include <cassert>

#include "../utils/macros.h"

enum MemoryAccessType
{
    READ_MEM,
    WRITE_MEM,
};

class Address
{
private:
    uint64_t v_;    // Virtual Address
    uint64_t p_;    // Physical Address
    uint64_t vpfn_; // Virtual Page Frame Number
    uint64_t ppfn_; // Physical Page Frame Number
    uint64_t ppo_;  // Physical Page Offset
    int level;      // Level of Page Walk resolution


public:
    const static uint64_t NonCanonical = ((uint64_t)1) << 49;
    MemoryAccessType AccessType;

    Address()
    {
        // Defaults to non-canonical address
        this->v_ = NonCanonical;
        this->p_ = NonCanonical;
    }

    Address(uint64_t vaddr)
    {
        this->v_ = vaddr;
        this->p_ = NonCanonical;
    }

    Address(uint64_t vaddr, MemoryAccessType access_type)
    {
        this->v_ = vaddr;
        this->p_ = NonCanonical;
        this->AccessType = access_type;
    }

    void Virtual(uint64_t vaddr) { this->v_ = vaddr; }

    uint64_t Virtual() { return this->v_; }

    void Physical(uint64_t paddr) { this->p_ = paddr; }

    uint64_t Physical() { return this->p_; }

    uint64_t PageTableEntryOffset(int level) { return (this->v_ >> (48 - (level * 9))) & 511; }

    uint64_t PhysicalOffset(int level) { return this->v_ & (((uint64_t)1 << (12 + (4 - level) * 9)) - 1); }
};

#endif