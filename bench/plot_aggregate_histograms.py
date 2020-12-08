import argparse
from matplotlib import pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--agg_dir', type=str, required=True)
parser.add_argument('--title', type=str, required=True)
parser.add_argument('--out_name', type=str, required=True)

def compute_states(agg_dir, name):
  cache_agg_file = open(os.path.join(agg_dir, name, '{}_cache_agg.out'))
  cache_stats = []
  for line in cache_agg_file:
    if line.split(',')[0] != 'MISS':
      cache_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1])))


  mmgr_agg_file = open(os.path.join(agg_dir, name, '{}_mmgr_agg.out'))
  mmgr_stats = []
  for line in mmgr_agg_file:
    if line.split(',')[0] != 'FASTMEM':
      mmgr_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1], int(line.strip().split(',')[2])))


  tlb_agg_file = open(os.path.join(agg_dir, name, '{}_tlb_agg.out'))
  tlb_stats = []
  for line in tlb_agg_file:
    if line.split(',')[0] != 'MISS':
      tlb_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1])))

  runtime_agg_file = open(os.path.join(agg_dir, name, '{}_runtime_agg.out'))
  runtime_stats = []
  for line in runtime_agg_file:
    if line.split(',')[0] != 'RUNTIME':
      runtime_stats.append((int(line.strip().split(',')[0]))))

experiments = [
  (args.agg_dir, 'hotset_linux'),
  (args.agg_dir, 'hotset_simple'),
  (args.agg_dir, 'rndm_write_linux'),
  (args.agg_dir, 'rndm_write_simple'),
  (args.agg_dir, 'seq_write_linux'),
  (args.agg_dir, 'seq_write_simple'),
]

for dir, name in experiments:
  compute_states(dir, name)