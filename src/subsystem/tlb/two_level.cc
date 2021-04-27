#include "two_level.h"

#include <cstring>

namespace TLB
{

    Entry *TwoLevel::Lookup(Address *addr, int *level)
    {
        string source = "tlb_lookup";
        execution_time->ObserveStart(source);

        Entry *ret = NULL;
        do
        {
            // 1G L1 TLB
            ret = LookupLevel("l1_1g", l1tlb_1g, L1_GIGA_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Giga]);
            if (ret != NULL)
            {
                *level = 2;
                break;
            }

            // 2M L1 TLB
            ret = LookupLevel("l1_2m", l1tlb_2m, L1_HUGE_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Huge]);
            if (ret != NULL)
            {
                *level = 3;
                break;
            }

            // 4K L1 TLB
            ret = LookupLevel("l1_4k", l1tlb_4k, L1_BASE_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Base]);
            if (ret != NULL)
            {
                *level = 4;
                break;
            }

            // 1G L2 TLB
            ret = LookupLevel("l2_1g", l2tlb_1g, L2_GIGA_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Giga]);
            if (ret != NULL)
            {
                *level = 2;
                break;
            }

            // 2M L2 TLB
            ret = LookupLevel("ul2_2m", l2tlb_2m4k, L2_HUGE_BASE_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Huge]);
            if (ret != NULL && ret->hugepage)
            {
                *level = 3;
                break;
            }

            ret = LookupLevel("ul2_4k", l2tlb_2m4k, L2_HUGE_BASE_ENTRIES, addr->Virtual() & Page::FrameNumberMask[Page::Type::Base]);
            if (ret != NULL && !ret->hugepage)
            {
                *level = 4;
                break;
            }
        } while (0);

        if (ret != NULL)
            access->Observe(source, Event::Access::Hit);
        else
            access->Observe(source, Event::Access::Miss);
        simulated_time->Observe(source, TimeTo[Simulate::Read]);
        execution_time->ObserveEnd(source);
        return ret;
    }

    Entry *TwoLevel::LookupLevel(string level, Entry *tlb, unsigned int size, uint64_t vpfn)
    {
        // string source = "tlb_lookup_" + level;
        // execution_time->ObserveStart(source);

        Entry *ret;
        PIN_MutexLock(&lock_);
        ret = LookupLevel(tlb, size, vpfn);
        PIN_MutexUnlock(&lock_);

        // execution_time->ObserveEnd(source);
        return ret;
    }

    Entry *TwoLevel::LookupLevel(Entry *tlb, unsigned int size, uint64_t vpfn)
    {
        Entry *ret;
        Entry *tlbe = &tlb[hash(vpfn) % size];
        if (tlbe->present && tlbe->vpfn == vpfn)
            ret = tlbe;
        else
            ret = NULL;
        return ret;
    }

    void TwoLevel::Insert(Address *addr, unsigned int level)
    {
        string source = "tlb_insert";
        execution_time->ObserveStart(source);

        Entry *tlbe;
        uint64_t vpfn = 0, ppfn = 0;

        assert(level >= 2 && level <= 4);

        PIN_MutexLock(&lock_);

        switch (level)
        {
        case 2: // 1GB page
            vpfn = addr->Virtual() & Page::FrameNumberMask[Page::Type::Giga];
            ppfn = addr->Physical() & Page::FrameNumberMask[Page::Type::Giga];
            tlbe = &l1tlb_1g[hash(vpfn) % L1_GIGA_ENTRIES];
            if (tlbe->present)
            {
                // Move previous entry down
                assert(tlbe->vpfn != vpfn);
                memcpy(&l2tlb_1g[hash(vpfn) % L2_GIGA_ENTRIES], tlbe, sizeof(Entry));
            }
            break;

        case 3: // 2MB page
            vpfn = addr->Virtual() & Page::FrameNumberMask[Page::Type::Huge];
            ppfn = addr->Physical() & Page::FrameNumberMask[Page::Type::Huge];
            tlbe = &l1tlb_2m[hash(vpfn) % L1_HUGE_ENTRIES];
            tlbe->hugepage = true;

            // Fall through...
        case 4: // 4KB page
            if (level == 4)
            {
                vpfn = addr->Virtual() & Page::FrameNumberMask[Page::Type::Base];
                ppfn = addr->Physical() & Page::FrameNumberMask[Page::Type::Base];
                tlbe = &l1tlb_4k[hash(vpfn) % L1_BASE_ENTRIES];
                tlbe->hugepage = false;
            }
            if (tlbe->present && tlbe->vpfn != vpfn)
            {
                // Move previous entry down
                memcpy(&l2tlb_2m4k[hash(vpfn) % L2_HUGE_BASE_ENTRIES], tlbe, sizeof(Entry));
            }
            break;
        }

        tlbe->present = true;
        tlbe->vpfn = vpfn;
        tlbe->ppfn = ppfn;
        PIN_MutexUnlock(&lock_);
    
        simulated_time->Observe(source, TimeTo[Simulate::Write]);
        access->Observe(source, Event::Access::Hit);
        execution_time->ObserveEnd(source);
    }

    void TwoLevel::Shootdown(Address *addr)
    {
        string source = "tlb_shootdown";
        execution_time->ObserveStart(source);
        PIN_MutexLock(&lock_);
        memset(l1tlb_1g, 0, sizeof(l1tlb_1g));
        memset(l1tlb_2m, 0, sizeof(l1tlb_2m));
        memset(l1tlb_4k, 0, sizeof(l1tlb_4k));
        memset(l2tlb_1g, 0, sizeof(l2tlb_1g));
        memset(l2tlb_2m4k, 0, sizeof(l2tlb_2m4k));
        PIN_MutexUnlock(&lock_);
        simulated_time->Observe(source, TimeTo[Simulate::Shootdown]);
        execution_time->ObserveEnd(source);
    }
}