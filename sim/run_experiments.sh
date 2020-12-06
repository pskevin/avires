#!/bin/bash

pin -t obj-intel64/mem_trace.so -o experiments/sim_small_linux_ -mm 1 -- ../apps/gups/obj/gups.o
pin -t obj-intel64/mem_trace.so -o experiments/sim_small_ -- ../apps/gups/obj/gups.o