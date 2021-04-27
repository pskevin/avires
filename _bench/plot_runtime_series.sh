#!/bin/bash

python plot_runtime_series.py --runtime_file results/mmgr/hotset_simple/gups_hotset_simple_runtime.out --timesteps 10000 --out_name timeseries_runtime_hotset_simple.png --title "Simple Memory Manager"
python plot_runtime_series.py --runtime_file results/mmgr/hotset_linux/gups_hotset_linux_runtime.out --timesteps 10000 --out_name timeseries_runtime_hotset_linux.png --title "Linux Memory Manager"

python plot_runtime_series.py --runtime_file results/mmgr/seq_read_simple/gups_seq_read_simple_runtime.out --timesteps 10000 --out_name timeseries_runtime_seq_read_simple.png --title "Simple Memory Manager"
python plot_runtime_series.py --runtime_file results/mmgr/seq_read_linux/gups_seq_read_linux_runtime.out --timesteps 10000 --out_name timeseries_runtime_seq_read_linux.png --title "Linux Memory Manager"

python plot_runtime_series.py --runtime_file results/mmgr/rndm_read_simple/gups_rndm_read_simple_runtime.out --timesteps 10000 --out_name timeseries_runtime_rndm_read_simple.png --title "Simple Memory Manager"
python plot_runtime_series.py --runtime_file results/mmgr/rndm_read_linux/gups_rndm_read_linux_runtime.out --timesteps 10000 --out_name timeseries_runtime_rndm_read_linux.png --title "Linux Memory Manager"