import argparse
from matplotlib import pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--bucket_file', type=str, required=True)
parser.add_argument('--timesteps', type=int, required=True)
parser.add_argument('--addr_start', type=int)
parser.add_argument('--title', type=str, required=True)
parser.add_argument('--out_name', type=str, required=True)

args = parser.parse_args()

MAX_TIMESTEPS = args.timesteps
PAGE_SIZE = 1024

with open(args.bucket_file) as bucket_file:
  bucket_lines = sum(1 for _ in bucket_file)

if bucket_lines > MAX_TIMESTEPS:
  bucket_lines = MAX_TIMESTEPS

buckets = np.zeros((bucket_lines))

with open(args.bucket_file) as bucket_file:
  line_idx = 0
  for line in bucket_file:
    bucket = float(line.strip()[:-len('0000e+14')]) * 1e14
    buckets[line_idx] = bucket

    line_idx += 1
    if line_idx >= bucket_lines:
      break

uniq_buckets = np.sort(np.unique(buckets))

for i in range(bucket_lines):
  if (args.addr_start):
    buckets[i] = (buckets[i] - args.addr_start) / PAGE_SIZE
  else:
    bucket_val = uniq_buckets[np.where(uniq_buckets == buckets[i])][0]
    buckets[i] = np.where(uniq_buckets == bucket_val)[0]


plt.scatter(np.arange(bucket_lines), buckets, s=1)
plt.ylabel('Page Number')
plt.ylim(0, 32)
plt.xlabel('Time')
plt.title(args.title)
plt.savefig(args.out_name)
