#include "./simulator.h"

#include <cassert>

uint64_t MemorySimulator::WalkPageTable(Address* addr, int &level)
{
    string source = "page_walk";
    uint64_t page_walk_time = 0;
    execution_time->ObserveStart(source);

    assert(cr3_ != NULL);

    PageTableEntry* page_table = cr3_, *pte = NULL;

    for (level = 1; level <= 4 && page_table != NULL; level++)
    {

        page_walk_time += TimeTo[Simulate::PageWalk];

        pte = &page_table[addr->PageTableEntryOffset(level)];
        assert(pte != NULL);

        if (!pte->Present || (pte->Readonly && addr->AccessType == WRITE_MEM))
        {
            simulated_time->Observe("mem_pagefault", TimeTo[Simulate::PageFault]);
            
            mem_->PageFault(addr, pte->Readonly && addr->AccessType == WRITE_MEM);

            assert(pte->Present);
            assert(!pte->Readonly || addr->AccessType != WRITE_MEM);
        }

        if (!pte->Accessed && pte->Pagemap && (pte->addr & Memory::SlowBit))
        {
            
        }

        pte->Accessed = true;
        if (addr->AccessType == WRITE_MEM)
        {
            pte->Modified = true;
        }

        if (pte->Pagemap)
        {
            // Page here -- terminate walk
            break;
        }

        page_table = pte->next;
    }

    simulated_time->Observe(source, page_walk_time);

    assert(pte != NULL);
    assert(level >= 2 && level <= 4);
    
    execution_time->ObserveEnd(source);
    return pte->addr + addr->PhysicalOffset(level);
}