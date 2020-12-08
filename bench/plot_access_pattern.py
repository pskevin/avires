import argparse
from matplotlib import pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--bucket_file', type=str, required=True)
parser.add_argument('--out_name', type=str, required=True)

args = parser.parse_args()


with open(args.bucket_file) as bucket_file:
  bucket_lines = sum(1 for _ in bucket_file)

buckets = np.zeros((bucket_lines))

with open(args.bucket_file) as bucket_file:
  line_idx = 0
  for line in bucket_file:
    bucket = float(line.strip()[:-len('0000e+14')]) * 1e14
    buckets[line_idx] = bucket
    line_idx += 1

plt.scatter(np.arange(bucket_lines), buckets, s=0.001)
plt.show()
