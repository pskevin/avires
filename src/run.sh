# make clean && make -j MEMSIM_DEBUG=0

pin -t obj-intel64/main.so \
    -- ../apps/gups/obj/gups_rndm_read.o 1 100000 25 0 0 0
    # -take-every-nth \
    # -observe-event access \
    # -from-source tlb_lookup \
    # -from-source mem_pagefault \
    # -from-source tlb_insert \
    # -from-source cache_access \
    # -from-source mem_fast_read \
    # -from-source mem_fast_write \
    # -from-source mem_slow_read \
    # -from-source mem_slow_write \






    # -observe-event execution_time \
    # -observe-event simulated_time \
    # -observe-event-from-source execution_time,page_walk \
    # -observe-event-from-source execution_time,e2e \
    # -observe-event-from-source execution_time,write \
    # -observe-event-from-source simulated_time,page_walk \
    # -observe-event-from-source simulated_time,mem_slow_read \
    # -observe-event-from-source simulated_time,mem_slow_write \
    # -observe-event-from-source simulated_time,mem_fast_read \
    # -observe-event-from-source simulated_time,mem_fast_write \
    # -observe-event-from-source execution_time,tlb_lookup_l1_1g \
    # -observe-event-from-source execution_time,tlb_lookup_l1_2m \
    # -observe-event-from-source execution_time,tlb_lookup_l1_4k \
    # -observe-event-from-source execution_time,tlb_lookup_l2_1g \
    # -observe-event-from-source execution_time,tlb_lookup_ul2_2m \
    # -observe-event-from-source execution_time,tlb_lookup_ul2_4k \
    


