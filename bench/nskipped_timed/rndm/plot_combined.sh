python3 /root/memsim/bench/plot/patterns_with_timing.py \
--pin-nskipped-time-path /root/memsim/bench/nskipped_timed/rndm/nskipped_timed.npy \
--pin-all-time-path /root/memsim/bench/all_observed/rndm/all_observed.npy \
--app-on-nvm-time-path /root/memsim/bench/gups_on_nvm/rndm/gups_on_nvm.npy \
--names 100% 10% 1% 0.1% \
--addr-paths \
    /root/memsim/bench/nskipped_timed/rndm/takes-1-th/0 \
    /root/memsim/bench/nskipped_timed/rndm/takes-10-th/0 \
    /root/memsim/bench/nskipped_timed/rndm/takes-100-th/0 \
    /root/memsim/bench/nskipped_timed/rndm/takes-1000-th/0 \
--region-starts 140431601778688 139724479389696 139707505971200 140456159576064 \
--region-sizes 1073741824 1073741824 1073741824 1073741824 \
--gen-combined-freq-hist True
# --gen-invidual-access-patterns True
# --gen-combined-timing True
