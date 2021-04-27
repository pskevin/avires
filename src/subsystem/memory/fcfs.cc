/*
 * First Come First Serve memory allocator that only allocates fixed-size pages, allocates
 * physical memory linearly (first fast, then slow), and cannot free memory.
 */

#include "fcfs.h"

namespace Memory
{

    uint64_t FirstComeFirstServe::GetPhysicalPage(Address *addr, PageTableEntry*pte)
    {
        uint64_t ret;

        if (fastmem < Size[Type::Fast])
        {
            ret = fastmem;
            fastmem += Page::Size[pt_];
        }
        else
        {
            assert(slowmem < Size[Type::Slow]);
            ret = slowmem | SlowBit;
            slowmem += Page::Size[pt_];
        }
        
        assert((ret & Page::OffsetMask[pt_]) == 0); // Must be aligned
        return ret;
    }

    PageTableEntry* FirstComeFirstServe::AllocatePageTables(Address *addr, Page::Type pt)
    {
        PageTableEntry*page_table = pml4_, *pte;

        // Allocate page tables down to the leaf
        for (int level = 1; level < (int)pt + 2; level++)
        {
            pte = &page_table[addr->PageTableEntryOffset(level)];

            if (!pte->Present)
            {
                pte->Present = true;
                pte->next = (struct PageTableEntry *)calloc(512, sizeof(struct PageTableEntry));
            }

            page_table = pte->next;
        }

        // Return last-level PTE corresponding to addr
        return &page_table[addr->PageTableEntryOffset((int)pt + 2)];
    }

    void FirstComeFirstServe::PageFault(Address *addr, bool readonly)
    {
        string source = "mem_pagefault";
        execution_time->ObserveStart(source);
        access->Observe(source, Event::Access::Hit);
        assert(!readonly);
        // Allocate page tables
        PageTableEntry *pte = AllocatePageTables(addr, pt_);
        assert(pte != NULL);
        pte->Present = true;
        pte->Pagemap = true;
        pte->addr = GetPhysicalPage(addr, pte);
        execution_time->ObserveEnd(source);
    }

    PageTableEntry* FirstComeFirstServe::GetCR3()
    {
        return pml4_;
    }

}