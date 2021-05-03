python3 /root/memsim/bench/plot/patterns_with_timing.py \
--pin-nskipped-time-path /root/memsim/bench/nskipped_timed/seq/nskipped_timed.npy \
--pin-all-time-path /root/memsim/bench/all_observed/seq/all_observed.npy \
--app-on-nvm-time-path /root/memsim/bench/gups_on_nvm/seq/gups_on_nvm.npy \
--names 100% 10% 1% 0.1% \
--addr-paths \
    /root/memsim/bench/nskipped_timed/seq/takes-1-th/0 \
    /root/memsim/bench/nskipped_timed/seq/takes-10-th/0 \
    /root/memsim/bench/nskipped_timed/seq/takes-100-th/0 \
    /root/memsim/bench/nskipped_timed/seq/takes-1000-th/0 \
--region-starts 140381656240128 140273360949248 139644811390976 140155333804032  \
--region-sizes 1073741824 1073741824 1073741824 1073741824 \
--gen-combined-freq-hist True
# --gen-combined-timing True
# --gen-invidual-access-patterns True