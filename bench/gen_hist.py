import numpy as np


# start = np.int64(140328197926912)
# end = start + 32768

# addrs = np.loadtxt('./results/hist/rndm_write/gups_rndm_write_vaddrs.out', dtype=np.int64)

# addrs = addrs[(addrs >= start) & (addrs <= end)]

# mask = (1024-1) ^ np.iinfo(np.int64).max

# buckets = addrs & mask

# np.savetxt('./results/hist/rndm_write/gups_rndm_write_buckets.out')

addrs = np.loadtxt('./results/hist/rndm_write/gups_rndm_write_vaddrs.out', dtype=np.int64)