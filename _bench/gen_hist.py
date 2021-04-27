import numpy as np


start = np.int64(139870077784064)
end = start + 32768

addrs = np.loadtxt('./results/hist/hotset/gups_hotset_vaddrs.out', dtype=np.int64)

# addrs = addrs[(addrs >= start) & (addrs <= end)]


# mask = (1024-1) ^ np.iinfo(np.int64).max

# buckets = addrs & mask

# np.savetxt('./results/hist/hotset/gups_hotset_buckets.out', buckets)
# np.savetxt('./results/hist/hotset/gups_hotset_start.out', np.array([start]))