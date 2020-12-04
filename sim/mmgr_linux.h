#include "memsim_new.h"
#include "pin.H"

#define NR_PAGES		32
#define KSWAPD_INTERVAL		S(1)	// In ns

#define FASTMEM_PAGES	(FASTMEM_SIZE / BASE_PAGE_SIZE)
#define SLOWMEM_PAGES	(SLOWMEM_SIZE / BASE_PAGE_SIZE)

struct page {
  struct page	*next, *prev;
  uint64_t	framenum;
  struct pte	*pte;
};

struct fifo_queue {
  struct page	*first, *last;
  size_t	numentries;
};


class LinuxMemoryManager: public MemoryManager {
  public:
    void pagefault(uint64_t addr, bool readonly);
    void init(MemorySimulator* sim);
    void kswapd(void *arg);

  private:
    int listnum(struct pte *pte);
    void enqueue_fifo(struct fifo_queue *queue, struct page *entry);
    struct page *dequeue_fifo(struct fifo_queue *queue);
    void shrink_caches(struct fifo_queue *pages_active, struct fifo_queue *pages_inactive);
    void expand_caches(struct fifo_queue *pages_active, struct fifo_queue *pages_inactive);
    uint64_t getmem(uint64_t addr, struct pte *pte);

    pte *alloc_ptables(uint64_t addr, enum pagetypes pt);
    struct pte pml4[512]; // Top-level page table (we only emulate one process)
    struct fifo_queue pages_active[NMEMTYPES], pages_inactive[NMEMTYPES], pages_free[NMEMTYPES];
    PIN_MUTEX global_lock;
    volatile bool in_kswapd = false;
    MemorySimulator* sim_;
};