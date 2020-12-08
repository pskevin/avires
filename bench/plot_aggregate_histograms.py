import argparse
from matplotlib import pyplot as plt
import numpy as np
import os

parser = argparse.ArgumentParser()

parser.add_argument('--agg_dir', type=str, required=True)

def compute_states(agg_dir, name):
  cache_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_cache_agg.out'.format(name)))
  cache_stats = []
  for line in cache_agg_file:
    if line.split(',')[0] != 'MISS':
      cache_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1])))


  mmgr_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_mmgr_agg.out'.format(name)))
  mmgr_stats = []
  for line in mmgr_agg_file:
    if line.split(',')[0] != 'FASTMEM':
      mmgr_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1]), int(line.strip().split(',')[2])))

  tlb_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_tlb_agg.out'.format(name)))
  tlb_stats = []
  for line in tlb_agg_file:
    if line.split(',')[0] != 'MISS':
      tlb_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1])))

  runtime_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_runtime_agg.out'.format(name)))
  runtime_stats = []
  for line in runtime_agg_file:
    if line.split(',')[0] != 'RUNTIME':
      runtime_stats.append((int(line.strip().split(',')[0])))

  cache_stats = np.array(cache_stats)
  mmgr_stats = np.array(mmgr_stats)
  tlb_stats = np.array(tlb_stats)
  runtime_stats = np.array(runtime_stats)

  cache_avg = np.average(cache_stats, axis=0)
  cache_std = np.std(cache_stats, axis=0)

  mmgr_avg = np.average(mmgr_stats, axis=0)
  mmgr_std = np.std(mmgr_stats, axis=0)

  tlb_avg = np.average(tlb_stats, axis=0)
  tlb_std = np.std(tlb_stats, axis=0)

  # plt.bar(np.arange(cache_avg.shape), cache_avg, yerr=cache_std, tick_label=['Miss', 'Hit'])
  # plt.title('Cache Aggregate Statistics')
  # plt.ylabel('Count')
  # plt.savefig('{}_cache_plot.png')
  plt.figure()
  plt.bar(np.arange(2), mmgr_avg[:2], yerr=mmgr_std[:2], tick_label=['FastMem', 'SlowMem'])
  plt.yscale('log')
  plt.title('Memory Manager Aggregate Statistics')
  plt.savefig('{}_mmgr_plot.png'.format(name))

args = parser.parse_args()

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