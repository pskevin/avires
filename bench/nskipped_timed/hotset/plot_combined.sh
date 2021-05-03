python3 /root/memsim/bench/plot/patterns_with_timing.py \
--pin-nskipped-time-path /root/memsim/bench/nskipped_timed/hotset/nskipped_timed.npy \
--pin-all-time-path /root/memsim/bench/all_observed/hotset/all_observed.npy \
--app-on-nvm-time-path /root/memsim/bench/gups_on_nvm/hotset/gups_on_nvm.npy \
--names 100% 10% 1% 0.1% \
--addr-paths \
    /root/memsim/bench/nskipped_timed/hotset/takes-1-th/0 \
    /root/memsim/bench/nskipped_timed/hotset/takes-10-th/0 \
    /root/memsim/bench/nskipped_timed/hotset/takes-100-th/0 \
    /root/memsim/bench/nskipped_timed/hotset/takes-1000-th/0 \
--region-starts 140491537457152 139866208485376 140051268243456 140227529629696 \
--region-sizes 1073741824 1073741824 1073741824 1073741824 \
--gen-combined-freq-hist True
# --gen-combined-timing True
# --gen-invidual-access-patterns True
