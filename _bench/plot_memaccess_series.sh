#!/bin/bash

#hotset
python plot_memaccess_series.py --mmgr_file results/mmgr/hotset_simple/gups_hotset_simple_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_hotset_simple.png --title "Simple Memory Manager"
python plot_memaccess_series.py --mmgr_file results/mmgr/hotset_linux/gups_hotset_linux_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_hotset_linux.png --title "Linux Memory Manager"

python plot_memaccess_series.py --mmgr_file results/mmgr/seq_read_simple/gups_seq_read_simple_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_seq_read_simple.png --title "Simple Memory Manager"
python plot_memaccess_series.py --mmgr_file results/mmgr/seq_read_linux/gups_seq_read_linux_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_seq_read_linux.png --title "Linux Memory Manager"

python plot_memaccess_series.py --mmgr_file results/mmgr/rndm_read_simple/gups_rndm_read_simple_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_rndm_read_simple.png --title "Simple Memory Manager"
python plot_memaccess_series.py --mmgr_file results/mmgr/rndm_read_linux/gups_rndm_read_linux_mmgr.out --timesteps 100000 --out_name timeseries_memaccess_rndm_read_linux.png --title "Linux Memory Manager"