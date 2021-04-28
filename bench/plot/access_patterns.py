#!/usr/bin/env python3
import argparse
import numpy as np


parser = argparse.ArgumentParser()

parser.add_argument('--path', type=str, required=True)
parser.add_argument('--region-start', type=int, required=True)
parser.add_argument('--region-size', type=int, required=True)
# parser.add_argument('--title', type=str, required=True)
# parser.add_argument('--out_name', type=str, required=True)

args = parser.parse_args()

if __name__ == '__main__':
    start = args.region_start
    end = start + args.region_size
    data = np.genfromtxt(args.path, dtype=np.uint64, delimiter=',', names=True) 
    v = data['vaddr']
    i = np.where((v >= start) & (v <= end))