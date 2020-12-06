#include "mmgr_linux.h"
#include "inttypes.h"
#include "pin.H"

int LinuxMemoryManager::listnum(struct pte *pte)
{
  int listnum = -1;
  
  PIN_MutexLock(&global_lock);
  
  uint64_t framenum = (pte->addr & SLOWMEM_MASK) / BASE_PAGE_SIZE;

  for(struct page *p = pages_active[SLOWMEM].first; p != NULL; p = p->next) {
    if(p->framenum == framenum) {
      listnum = 0;
      goto out;
    }
  }

  for(struct page *p = pages_inactive[SLOWMEM].first; p != NULL; p = p->next) {
    if(p->framenum == framenum) {
      listnum = 1;
      goto out;
    }
  }

  for(struct page *p = pages_active[FASTMEM].first; p != NULL; p = p->next) {
    if(p->framenum == framenum) {
      listnum = 2;
      goto out;
    }
  }

  for(struct page *p = pages_inactive[FASTMEM].first; p != NULL; p = p->next) {
    if(p->framenum == framenum) {
      listnum = 3;
      goto out;
    }
  }

 out:
  assert(listnum != -1);
  PIN_MutexUnlock(&global_lock);
  return listnum;
}

void LinuxMemoryManager::enqueue_fifo(struct fifo_queue *queue, struct page *entry)
{
  assert(entry->prev == NULL);
  entry->next = queue->first;
  if(queue->first != NULL) {
    assert(queue->first->prev == NULL);
    queue->first->prev = entry;
  } else {
    assert(queue->last == NULL);
    assert(queue->numentries == 0);
    queue->last = entry;
  }
  
  queue->first = entry;
  queue->numentries++;
}

struct page * LinuxMemoryManager::dequeue_fifo(struct fifo_queue *queue)
{
  struct page *ret = queue->last;

  if(ret == NULL) {
    assert(queue->numentries == 0);
    return ret;
  }

  queue->last = ret->prev;
  if(queue->last != NULL) {
    queue->last->next = NULL;
  } else {
    queue->first = NULL;
  }

  ret->prev = ret->next = NULL;
  assert(queue->numentries > 0);
  queue->numentries--;
  return ret;
}

void LinuxMemoryManager::shrink_caches(struct fifo_queue *pages_active,
			  struct fifo_queue *pages_inactive)
{
  size_t nr_pages = 1;
  /* size_t nr_pages = */
  /*   NR_PAGES * pages_active->numentries / ((pages_inactive->numentries + 1) * 2); */

  // Move cold pages down (or rotate)
  while(nr_pages > 0 && pages_active->numentries > 0) {
    struct page *p = dequeue_fifo(pages_active);

    if(p->pte->accessed) {
      // XXX: Dangerous. Introduce soft accessed bit instead, like Linux?
      p->pte->accessed = false;
      sim_->tlb_shootdown(p->framenum * BASE_PAGE_SIZE);
      enqueue_fifo(pages_active, p);
    } else {
      // XXX: Dangerous. Introduce soft accessed bit instead, like Linux?
      /* p->pte->accessed = true; */
      enqueue_fifo(pages_inactive, p);
      nr_pages--;
    }
  }
}

void LinuxMemoryManager::expand_caches(struct fifo_queue *pages_active,
			  struct fifo_queue *pages_inactive)
{
  size_t nr_pages = pages_inactive->numentries;

  // Move hot pages up
  for(size_t i = 0; i < nr_pages; i++) {
    struct page *p = dequeue_fifo(pages_inactive);

    if(p->pte->accessed) {
      enqueue_fifo(pages_active, p);
    } else {
      enqueue_fifo(pages_inactive, p);
    }
  }
}

uint64_t LinuxMemoryManager::getmem(uint64_t addr, struct pte *pte)
{
  PIN_MutexLock(&global_lock);

  for(int tries = 0; tries < 2; tries++) {
    // Allocate from fastmem, put on active FIFO queue
    struct page *newpage = dequeue_fifo(&pages_free[FASTMEM]);

    if(newpage != NULL) {
      newpage->pte = pte;
      enqueue_fifo(&pages_active[FASTMEM], newpage);

      PIN_MutexUnlock(&global_lock);
      return newpage->framenum * BASE_PAGE_SIZE;
    }

    // Move a page to cold memory
    if(pages_inactive[FASTMEM].numentries == 0) {
      // Force some pages down if there aren't any
      shrink_caches(&pages_active[FASTMEM], &pages_inactive[FASTMEM]);
      shrink_caches(&pages_active[SLOWMEM], &pages_inactive[SLOWMEM]);
    }

    // Move a cold page from fastmem to slowmem
    struct page *p = dequeue_fifo(&pages_inactive[FASTMEM]);
    struct page *np = dequeue_fifo(&pages_free[SLOWMEM]);
    if(np != NULL) {
      // Emulate memory copy from fast to slow mem
      if(!static_cast<bool>(OS_TlsGetValue(in_kswapd))) {
        sim_->add_runtime(TIME_SLOWMOVE);
      }

      MEMSIM_LOG("OOM hot (%" PRIu64 ") -> cold (%" PRIu64 ")\n", sim_->runtime,
        p->framenum, np->framenum);

      // Remap page
      np->pte = p->pte;
      np->pte->addr = (np->framenum * BASE_PAGE_SIZE) | SLOWMEM_BIT;
      sim_->tlb_shootdown(0);

      // Put on slowmem inactive list
      enqueue_fifo(&pages_inactive[SLOWMEM], np);

      // Free fastmem
      enqueue_fifo(&pages_free[FASTMEM], p);
    }
  }

  PIN_MutexUnlock(&global_lock);
  assert(!"Out of memory");
}

pte * LinuxMemoryManager::alloc_ptables(uint64_t addr, enum pagetypes pt)
{
  struct pte *ptable = pml4, *pte;

  // Allocate page tables down to the leaf
  for(int i = 1; i < 4; i++) {
    pte = &ptable[(addr >> (48 - (i * 9))) & 511];

    if(!pte->present) {
      pte->present = true;
      pte->next = (struct pte *) calloc(512, sizeof(struct pte));
    }

    ptable = pte->next;
  }

  // Return last-level PTE corresponding to addr
  return &ptable[(addr >> (48 - (4 * 9))) & 511];
}

void LinuxMemoryManager::pagefault(uint64_t addr, bool readonly)
{
  assert(!readonly);
  // Allocate page tables
  struct pte *pte = alloc_ptables(addr, BASE_PAGE);
  pte->present = true;
  pte->pagemap = true;

  pte->addr = getmem(addr, pte);
  assert((pte->addr & BASE_PAGE_MASK) == 0);	// Must be aligned
}

void swapHelper(void* mgr) {
  static_cast<LinuxMemoryManager*>(mgr)->kswapd(nullptr);
}

void LinuxMemoryManager::init(MemorySimulator* sim)
{
  sim_ = sim;
  sim->setCR3(pml4);
  PIN_MutexInit(&global_lock);
  struct page *p = (page*)calloc(FASTMEM_PAGES, sizeof(struct page));
  for(uint32_t i = 0; i < FASTMEM_PAGES; i++) {
    p[i].framenum = i;
    enqueue_fifo(&pages_free[FASTMEM], &p[i]);
  }
  p = (page*) calloc(SLOWMEM_PAGES, sizeof(struct page));
  for(uint32_t i = 0; i < SLOWMEM_PAGES; i++) {
    p[i].framenum = i;
    enqueue_fifo(&pages_free[SLOWMEM], &p[i]);
  }


  in_kswapd = OS_TlsAlloc(NULL);
  OS_TlsSetValue(in_kswapd, reinterpret_cast<void *> (static_cast<int> (false)));

  tid = PIN_SpawnInternalThread(&swapHelper, this, 0, &threadUID);
  assert(tid != INVALID_THREADID);
}

void LinuxMemoryManager::kswapd(void *arg)
{
  OS_TlsSetValue(in_kswapd, reinterpret_cast<void *> (static_cast<int> (true)));
  OS_GetTid(&native_tid);
  // thread_should_terminate = PIN_CreateThreadDataKey(NULL);
// tatic_cast<bool>(PIN_GetThreadData(thread_should_terminate, tid))
  while(!thread_should_terminate) {
    sim_->memsim_nanosleep(KSWAPD_INTERVAL);

    PIN_MutexLock(&global_lock);

    shrink_caches(&pages_active[FASTMEM], &pages_inactive[FASTMEM]);
    shrink_caches(&pages_active[SLOWMEM], &pages_inactive[SLOWMEM]);

    // Move_hot
    expand_caches(&pages_active[FASTMEM], &pages_inactive[FASTMEM]);
    expand_caches(&pages_active[SLOWMEM], &pages_inactive[SLOWMEM]);

    // Move hot pages from slowmem to fastmem
    for(struct page *p = dequeue_fifo(&pages_active[SLOWMEM]); p != NULL; p = dequeue_fifo(&pages_active[SLOWMEM])) {
      for(int tries = 0; tries < 2; tries++) {
        struct page *np = dequeue_fifo(&pages_free[FASTMEM]);

        if(np != NULL) {
          MEMSIM_LOG("cold (%" PRIu64 ") -> hot (%" PRIu64 "), "
              "slowmem.active = %zu, slowmem.inactive = %zu\n",
              sim_->runtime,
              p->framenum, np->framenum,
              pages_active[SLOWMEM].numentries,
              pages_inactive[SLOWMEM].numentries);

          // Remap page
          np->pte = p->pte;
          np->pte->addr = np->framenum * BASE_PAGE_SIZE;
          sim_->tlb_shootdown(0);

          // Put on fastmem active list
          enqueue_fifo(&pages_active[FASTMEM], np);

          // Free slowmem
          enqueue_fifo(&pages_free[SLOWMEM], p);

          break;
        }

        // Not moved - Out of fastmem - move cold page down
        struct page *cp = dequeue_fifo(&pages_inactive[FASTMEM]);
        if(cp == NULL) {
          // All fastmem pages are hot -- bail out
          enqueue_fifo(&pages_active[SLOWMEM], p);
          goto out;
        }

        np = dequeue_fifo(&pages_free[SLOWMEM]);
        if(np != NULL) {
          // Remap page
          np->pte = cp->pte;
          np->pte->addr = (np->framenum * BASE_PAGE_SIZE) | SLOWMEM_BIT;
          sim_->tlb_shootdown(0);

          // Put on slowmem inactive list
          enqueue_fifo(&pages_inactive[SLOWMEM], np);

          // Free fastmem
          enqueue_fifo(&pages_free[FASTMEM], cp);
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

void LinuxMemoryManager::shutdown() 
{
  thread_should_terminate = true;
}