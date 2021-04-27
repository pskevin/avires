# make clean && make -j MEMSIM_DEBUG=0

pin -t obj-intel64/main.so \
    -take-every-nth 500 \
    -observe-event access \
    -observe-event execution_time \
    -observe-event simulated_time \
    -from-source tlb_lookup \
    -from-source mem_pagefault \
    -from-source tlb_insert \
    -from-source cache_access \
    -observe-event-from-source execution_time,page_walk \
    -observe-event-from-source execution_time,e2e \
    -observe-event-from-source execution_time,write \
    -observe-event-from-source simulated_time,page_walk \
    -observe-event-from-source simulated_time,mem_slow_read \
    -observe-event-from-source simulated_time,mem_slow_write \
    -observe-event-from-source simulated_time,mem_fast_read \
    -observe-event-from-source simulated_time,mem_fast_write \
    -- ../apps/gups/obj/gups.o 1 256 10 8 0 0 0
    # -observe-event-from-source execution_time,tlb_lookup_l1_1g \
    # -observe-event-from-source execution_time,tlb_lookup_l1_2m \
    # -observe-event-from-source execution_time,tlb_lookup_l1_4k \
    # -observe-event-from-source execution_time,tlb_lookup_l2_1g \
    # -observe-event-from-source execution_time,tlb_lookup_ul2_2m \
    # -observe-event-from-source execution_time,tlb_lookup_ul2_4k \
    


