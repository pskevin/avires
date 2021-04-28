import argparse
from matplotlib import pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--mmgr_file', type=str, required=True)
parser.add_argument('--timesteps', type=int, required=True)
parser.add_argument('--title', type=str, required=True)
parser.add_argument('--out_name', type=str, required=True)

args = parser.parse_args()

with open(args.mmgr_file) as mmgr_file:
  timesteps = sum(1 for _ in mmgr_file)

MAX_TIMESTEPS = args.timesteps

if timesteps > MAX_TIMESTEPS:
  timesteps = MAX_TIMESTEPS


access_level = np.zeros((timesteps))

with open(args.mmgr_file) as mmgr_file:
  line_idx = 0
  for line in mmgr_file:
    if line_idx > 0:
      fastmem, slowmem, pf = [int(l) for l in line.strip().split(',')[:-1]]
      assert(not (fastmem == 1 and slowmem ==1))
      assert(fastmem <= 1)
      assert(slowmem <= 1)
      access_level[line_idx] = fastmem * 1 + slowmem * 2
    line_idx += 1
    if line_idx >= timesteps:
      break

plt.scatter(np.arange(timesteps), access_level, s=0.25)
plt.ylabel('Memory Access Level')
plt.xlabel('Application Time')
plt.title(args.title)
plt.savefig(args.out_name)
