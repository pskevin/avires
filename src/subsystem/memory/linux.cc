#include "linux.h"

#include <ctime>

namespace Memory
{

    void Linux::EnqueueFifo(queue *queue, node *entry)
    {
        assert(entry->prev == NULL);
        entry->next = queue->first;
        if (queue->first != NULL)
        {
            assert(queue->first->prev == NULL);
            queue->first->prev = entry;
        }
        else
        {
            assert(queue->last == NULL);
            assert(queue->numentries == 0);
            queue->last = entry;
        }

        queue->first = entry;
        queue->numentries++;
    }

    node *Linux::DequeueFifo(queue *queue)
    {
        node *ret = queue->last;

        if (ret == NULL)
        {
            assert(queue->numentries == 0);
            return ret;
        }

        queue->last = ret->prev;
        if (queue->last != NULL)
        {
            queue->last->next = NULL;
        }
        else
        {
            queue->first = NULL;
        }

        ret->prev = ret->next = NULL;
        assert(queue->numentries > 0);
        queue->numentries--;
        return ret;
    }

    void Linux::ShrinkCaches(queue *pages_active,queue *pages_inactive)
    {
        size_t nr_pages = 1;
        /* size_t nr_pages = */
        /*   NR_PAGES * pages_active->numentries / ((pages_inactive->numentries + 1) * 2); */

        // Move cold pages down (or rotate)
        while (nr_pages > 0 && pages_active->numentries > 0)
        {
            node *p = DequeueFifo(pages_active);

            if (p->pte->Accessed)
            {
                // XXX: Dangerous. Introduce soft Accessed bit instead, like Linux?
                p->pte->Accessed = false;
                // sim_->tlb_shootdown(p->framenum * Page::Size[pt_]);
                EnqueueFifo(pages_active, p);
            }
            else
            {
                // XXX: Dangerous. Introduce soft Accessed bit instead, like Linux?
                /* p->pte->Accessed = true; */
                EnqueueFifo(pages_inactive, p);
                nr_pages--;
            }
        }
    }

    void Linux::ExpandCaches(queue *pages_active,queue *pages_inactive)
    {
        size_t nr_pages = pages_inactive->numentries;

        // Move hot pages up
        for (size_t i = 0; i < nr_pages; i++)
        {
            node *p = DequeueFifo(pages_inactive);

            if (p->pte->Accessed)
            {
                EnqueueFifo(pages_active, p);
            }
            else
            {
                EnqueueFifo(pages_inactive, p);
            }
        }
    }

    uint64_t Linux::GetPhysicalPage(Address *addr, PageTableEntry *pte)
    {
        PIN_MutexLock(&global_lock);

        for (int tries = 0; tries < 2; tries++)
        {
            // Allocate from fastmem, put on active FIFO queue
            node *newpage = DequeueFifo(&pages_free[0]);

            if (newpage != NULL)
            {
                newpage->pte = pte;
                EnqueueFifo(&pages_active[0], newpage);

                PIN_MutexUnlock(&global_lock);
                return newpage->framenum * Page::Size[pt_];
            }

            // Move a page to cold memory
            if (pages_inactive[0].numentries == 0)
            {
                // Force some pages down if there aren't any
                ShrinkCaches(&pages_active[0], &pages_inactive[0]);
                ShrinkCaches(&pages_active[1], &pages_inactive[1]);
            }

            // Move a cold page from fastmem to slowmem
            node *p = DequeueFifo(&pages_inactive[0]);
            node *np = DequeueFifo(&pages_free[1]);

            if (np != NULL)
            {
                // Emulate memory copy from fast to slow mem
                if (!static_cast<bool>(OS_TlsGetValue(in_kswapd)))
                {
                    // simulated_time->Observe(TIME_SLOWMOVE)
                }

                assert(p->pte->Pagemap == true);

                // Remap page
                np->pte = p->pte;
                np->pte->addr = (np->framenum * Page::Size[pt_]) | SlowBit;
                // sim_->tlb_shootdown(0);

                // Put on slowmem inactive list
                EnqueueFifo(&pages_inactive[1], np);

                // Free fastmem
                EnqueueFifo(&pages_free[0], p);
            }
        }

        PIN_MutexUnlock(&global_lock);
        assert(!"Out of memory");
    }

    PageTableEntry *Linux::AllocatePageTables(Address *addr, Page::Type pt)
    {
        PageTableEntry *page_table = pml4_, *pte;

        // Allocate page tables down to the leaf
        for (int i = 1; i < 4; i++)
        {
            pte = &page_table[addr->PageTableEntryOffset(i)];

            if (!pte->Present)
            {
                pte->Present = true;
                pte->next = (PageTableEntry *)calloc(512, sizeof(PageTableEntry));
            }

            page_table = pte->next;
        }

        // Return last-level PTE corresponding to addr
        return &page_table[addr->PageTableEntryOffset(4)];
    }

    void Linux::PageFault(Address *addr, bool readonly)
    {
        assert(!readonly);
        // Allocate page tables
        string source = "mem_pagefault";
        execution_time->ObserveStart(source);
        access->Observe(source, Event::Access::Hit);

        // cout << "page fault" <<endl;
        PageTableEntry *pte = AllocatePageTables(addr, pt_);
        pte->Present = true;
        pte->Pagemap = true;

        pte->addr = GetPhysicalPage(addr, pte);
        execution_time->ObserveEnd(source);
        assert((pte->addr & Page::OffsetMask[pt_]) == 0); // Must be aligned
    }

    void Linux::kswapd(void *arg)
    {
        OS_TlsSetValue(in_kswapd, reinterpret_cast<void *>(static_cast<int>(true)));
        OS_GetTid(&native_tid);
        // thread_should_terminate = PIN_CreateThreadDataKey(NULL);
        // static_cast<bool>(PIN_GetThreadData(thread_should_terminate, tid))

        struct timespec sleep_time, rem;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = KSWAPD_INTERVAL;
        
        while (!thread_should_terminate)
        {
            // sim_->memsim_nanosleep(KSWAPD_INTERVAL);
            int ret = nanosleep(&sleep_time, &rem);
            assert(ret >= 0);

            PIN_MutexLock(&global_lock);

            ShrinkCaches(&pages_active[0], &pages_inactive[0]);
            ShrinkCaches(&pages_active[1], &pages_inactive[1]);

            // Move_hot
            ExpandCaches(&pages_active[0], &pages_inactive[0]);

            ExpandCaches(&pages_active[1], &pages_inactive[1]);

            // Move hot pages from slowmem to fastmem
            for (node *p = DequeueFifo(&pages_active[1]); p != NULL; p = DequeueFifo(&pages_active[1]))
            {
                for (int tries = 0; tries < 2; tries++)
                {
                    node *np = DequeueFifo(&pages_free[0]);

                    if (np != NULL)
                    {
                        // Remap page
                        np->pte = p->pte;
                        np->pte->addr = np->framenum * Page::Size[pt_];
                        // sim_->tlb_shootdown(0);

                        // Put on fastmem active list
                        EnqueueFifo(&pages_active[0], np);

                        // Free slowmem
                        EnqueueFifo(&pages_free[1], p);

                        break;
                    }

                    // Not moved - Out of fastmem - move cold page down
                    node *cp = DequeueFifo(&pages_inactive[0]);
                    if (cp == NULL)
                    {
                        // All fastmem pages are hot -- bail out
                        EnqueueFifo(&pages_active[1], p);
                        goto out;
                    }

                    np = DequeueFifo(&pages_free[1]);
                    if (np != NULL)
                    {
                        // Remap page
                        np->pte = cp->pte;
                        np->pte->addr = (np->framenum * Page::Size[pt_]) | SlowBit;
                        // sim_->tlb_shootdown(0);

                        // Put on slowmem inactive list
                        EnqueueFifo(&pages_inactive[1], np);

                        // Free fastmem
                        EnqueueFifo(&pages_free[0], cp);
                        /* fprintf(stderr, "%zu hot -> cold\n", runtime); */
                    }
                }
            }

        out:
            PIN_MutexUnlock(&global_lock);
        }

        PIN_ExitThread(0);
        return;
    }

 
    void swapHelper(void* mgr) {
            static_cast<Linux*>(mgr)->kswapd(nullptr);
    }
 
    void Linux::init()
    {
        node *p = (node *)calloc(fast_pages_, sizeof(node));
        for (uint32_t i = 0; i < fast_pages_; i++)
        {
            p[i].framenum = i;
            EnqueueFifo(&pages_free[0], &p[i]);
        }
        p = (node *)calloc(slow_pages_, sizeof(node));
        for (uint32_t i = 0; i < slow_pages_; i++)
        {
            p[i].framenum = i;
            EnqueueFifo(&pages_free[1], &p[i]);
        }
        in_kswapd = OS_TlsAlloc(NULL);
        OS_TlsSetValue(in_kswapd, reinterpret_cast<void *>(static_cast<int>(false)));
        tid = PIN_SpawnInternalThread(&swapHelper, this, 0, &threadUID);
        assert(tid != INVALID_THREADID);
    
    }

    void Linux::shutdown()
    {
        thread_should_terminate = true;
    }


    PageTableEntry* Linux::GetCR3()
    {
        return pml4_;
    }
}