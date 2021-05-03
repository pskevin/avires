#ifndef MEMSIM_SUBSYSTEM_MEMORY_LINUX_H
#define MEMSIM_SUBSYSTEM_MEMORY_LINUX_H

#include "pin.H"

#include "../include/models/page.h"
#include "../include/controllers/memory.h"
#include "../include/models/tlb_entry.h"
#include "../include/utils/macros.h"

#include "../include/models/event.h"
#include "../include/events/access.h"
#include "../include/events/execution_time.h"
#include "../include/events/simulated_time.h"

#define NR_PAGES 32
#define KSWAPD_INTERVAL S(1) // In ns

typedef struct FifoNode
{
    struct FifoNode *next, *prev;
    uint64_t framenum;
    PageTableEntry *pte;
} node;

typedef struct FifoQueue
{
    node *first, *last;
    size_t numentries;
} queue;


namespace Memory
{

    class Linux : public Controller
    {
    public:
        void PageFault(Address *addr, bool readonly);
        PageTableEntry *GetCR3();

        Linux(Page::Type pt) : pt_(pt)
        {
            fast_pages_ = Size[Type::Fast] / Page::Size[pt];
            slow_pages_ = Size[Type::Slow] / Page::Size[pt];

            access = Event::GetPool()->Find<Event::Access>();
            execution_time = Event::GetPool()->Find<Event::ExecutionTime>();
            simulated_time = Event::GetPool()->Find<Event::SimulatedTime>();

            PIN_MutexInit(&global_lock);
            init();
        };
        void shutdown();
        void kswapd(void *arg);

    private:
        Event::Type* access;
        Event::ExecutionTime *execution_time;
        Event::SimulatedTime *simulated_time;

        void init();
        void EnqueueFifo(queue *queue, node *entry);
        node *DequeueFifo(queue *queue);
        void ShrinkCaches(queue *pages_active, queue *pages_inactive);
        void ExpandCaches(queue *pages_active, queue *pages_inactive);

        uint64_t GetPhysicalPage(Address *addr, PageTableEntry *pte);
        PageTableEntry* AllocatePageTables(Address *addr, Page::Type pt);
        PageTableEntry pml4_[512]; // Top-level page table (we only emulate one process)
        queue pages_active[2], pages_inactive[2], pages_free[2];
        Page::Type pt_;
        uint64_t fast_pages_, slow_pages_;

        PIN_MUTEX global_lock;
        PIN_TLS_INDEX in_kswapd;
        PIN_THREAD_UID threadUID;
        volatile BOOL thread_should_terminate;
        THREADID tid;
        NATIVE_TID native_tid;
    };
}



#endif