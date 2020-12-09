#!/bin/bash
# Run on linux memory manager
pin -t obj-intel64/mem_trace.so -o experiments/sim_small_linux_ -mm 1 -- ../apps/gups/obj/gups.o 1 100000 15 8 0 0 0

# Run application with hotset on simple memory manager
pin -t obj-intel64/mem_trace.so -o experiments/sim_small_ -- ../apps/gups/obj/gups.o 1 100000 15 8 40 30 90
