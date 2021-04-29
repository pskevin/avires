#include "gups.h"
#include "log.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <sys/mman.h>
#include <inttypes.h>

typedef struct Context
{
    int tid;                  // thread id
    void *region;             // pointer to mmaped region
    uint64_t region_size;     // size of region
    uint64_t num_updates;     // iterations to perform
    uint64_t hotset_start;    // start of hotset
    uint64_t hotset_size;     // size of hotset
    float access_probability; // probability of sampling hotset
} Context;

static void *gups(void *args)
{
    Context *ctx = (Context *)args;
    uint64_t *region = (uint64_t *)(ctx->region);
    uint64_t index, seed;

    for (uint64_t i = 0; i < ctx->num_updates; i++)
    {
        switch (ACCESS_PATTERN)
        {
        case RANDOM:
            if (i == 0)
            {
                srand(0);
                seed = rand();
            }
            seed = next_index(seed);
            float p = ((float)rand()) / RAND_MAX;

            if (p < ctx->access_probability)
                index = ctx->hotset_start + (seed % ctx->hotset_size);
            else
                index = seed % ctx->region_size;
            break;

        case SEQUENTIAL:
            index = i % ctx->region_size;
            break;
        case ZIPFIAN:
            break;
        }


        uint64_t tmp = region[index] + i;
        // uint64_t tmp = *(region+index) + i;
        if (ACCESS_TYPE == WRITE)
        {
            region[index] = tmp;
        }
    }
}

int main(int argc, char **argv)
{
    
    if (argc != 7)
    {
        printf("Usage: %s [threads] [updates per thread] [exponent] [hotset start (\%)] [hotset size (\%)] [access probability (\%)]\n", argv[0]);
        printf("  threads\t\t\tnumber of threads to launch\n");
        printf("  updates per thread\t\tnumber of updates per thread\n");
        printf("  exponent\t\t\tlog size of region\n");
        printf("  hotset start\t\t\tstarting from (in percentage)\n");
        printf("  hotset size\t\t\tnumber of elements (in percentage)\n");
        printf("  access probability\t\tprobability of accessing hotset (in percentage)\n");

        return 0;
    }

    printf("Starting GUPS\n");
    // asm("ud2");

    int num_threads, exponent;
    uint64_t num_updates, region_size, region_per_thread;

    uint64_t hotset_start, hotset_size;
    float access_probability;

    num_threads = atoi(argv[1]);
    assert(num_threads <= MAX_THREADS);

    // TODO: Figure out from Tim why "256"?
    num_updates = atol(argv[2]);
    num_updates -= num_updates % 256;
    exponent = atoi(argv[3]);

    assert(exponent > 8);
    assert(num_updates > 0 && (num_updates % 256 == 0));

    region_size = (uint64_t)(1) << exponent;
    region_size -= (region_size % 256);

    assert(region_size > 0 && (region_size % 256 == 0));

    region_per_thread = (region_size / num_threads) / 8; // uint64_t is 8 bytes

    hotset_start = atoi(argv[4]);
    hotset_size = atoi(argv[5]);
    assert(hotset_start >= 0 && hotset_start <= 100);
    assert(hotset_size >= 0 && hotset_size <= 100);
    assert(hotset_start + hotset_size >= 0 && hotset_start + hotset_size <= 100);
    access_probability = (float)atoi(argv[6]);
    assert(access_probability >= 0 && access_probability <= 100);

    access_probability /= 100;
    hotset_start = (uint64_t)region_per_thread * hotset_start / 100;
    hotset_size = (uint64_t)region_per_thread * hotset_size / 100;

    LogMessage("Field of 2^%lu, i.e, (%lu) bytes", exponent, region_size);
    LogMessage("%lu region indices per thread (%d threads)", region_per_thread, num_threads);
    LogMessage("Updates per thread: %lu", num_updates);
    LogMessage("Hot start(%lld) size(%lld) prob(%f)", hotset_start, hotset_size, access_probability);

    void *region = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED)
    {
        perror("mmap");
        assert(0);
    }

    printf("Region start: %" PRIuPTR " or %p \tRegion size: %lld\n", (uintptr_t)region, region, region_size);

    uint64_t access = (uint64_t)&region[100];

    
    // Initializing thread data
    Context **ctxs = (Context **)malloc(num_threads * sizeof(Context *));
    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < num_threads; ++i)
    {
        ctxs[i] = (Context *)malloc(sizeof(Context));
        ctxs[i]->region = region + (i * region_per_thread * 8);
    }

    // Spawn worker threads
    LogPoint();
    for (int i = 0; i < num_threads; i++)
    {
        ctxs[i]->tid = i;
        ctxs[i]->num_updates = num_updates;
        ctxs[i]->region_size = region_per_thread;
        ctxs[i]->hotset_start = (region_per_thread * i) + hotset_start;
        ctxs[i]->hotset_size = hotset_size;
        ctxs[i]->access_probability = access_probability;
        int r = pthread_create(&threads[i], NULL, gups, (void *)ctxs[i]);
        assert(r == 0);
    }

    // Wait for worker threads
    for (int i = 0; i < num_threads; i++)
    {
        int r = pthread_join(threads[i], NULL);
        assert(r == 0);
    }

    for (int i = 0; i < num_threads; i++)
    {
        free(ctxs[i]);
    }
    free(ctxs);

    munmap(region, region_size);
    printf("GUPS finished safely.\n");
    return 0;
}