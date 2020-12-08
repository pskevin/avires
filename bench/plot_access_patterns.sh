#!/bin/bash

python plot_access_pattern.py --bucket_file results/hist/hotset/gups_hotset_buckets.out --timesteps 5000 --out_name access_pattern_hotset.png --addr_start_file results/hist/hotset/gups_hotset_start.out --title "Hotset Page Access Pattern"

python plot_access_pattern.py --bucket_file results/hist/seq_read/gups_seq_read_buckets.out --timesteps 25000 --out_name access_pattern_seq.png --title "Sequential Page Access Pattern"

python plot_access_pattern.py --bucket_file results/hist/rndm_read/gups_rndm_read_buckets.out --timesteps 10000 --out_name access_pattern_random.png --title "Random Page Access Pattern"