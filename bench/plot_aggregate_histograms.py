import argparse
from matplotlib import pyplot as plt
import numpy as np
import os

parser = argparse.ArgumentParser()

parser.add_argument('--agg_dir', type=str, required=True)

def compute_mmgr_plot(agg_dir, name):
  mmgr_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_mmgr_agg.out'.format(name)))
  mmgr_stats = []
  for line in mmgr_agg_file:
    if line.split(',')[0] != 'FASTMEM':
      mmgr_stats.append((int(line.strip().split(',')[0]), int(line.strip().split(',')[1]), int(line.strip().split(',')[2])))

  mmgr_stats = np.array(mmgr_stats)
  mmgr_avg = np.average(mmgr_stats, axis=0)
  mmgr_std = np.std(mmgr_stats, axis=0)

  plt.figure()
  plt.bar(np.arange(2), mmgr_avg[:2], yerr=mmgr_std[:2], tick_label=['FastMem', 'SlowMem'])
  # plt.yscale('log')
  plt.ylim(0, 1000000)
  plt.yticks([200000 * i for i in range(5)])
  plt.title('Memory Manager Aggregate Statistics')
  plt.savefig('{}_mmgr_plot.png'.format(name))

args = parser.parse_args()

mmgr_experiments = [
  (args.agg_dir, 'hotset_linux'),
  (args.agg_dir, 'hotset_simple'),
  (args.agg_dir, 'rndm_write_linux'),
  (args.agg_dir, 'rndm_write_simple'),
  (args.agg_dir, 'seq_write_linux'),
  (args.agg_dir, 'seq_write_simple'),
]

for dir, name in mmgr_experiments:
  compute_mmgr_plot(dir, name)

def compute_hierarchy_plot(agg_dir, names, labels):
  assert(len(names) == len(labels))

  cache_avgs = []
  cache_stds = []
  tlb_avgs = []
  tlb_stds = []

  for i in range(len(names)):
    name = names[i]
    label = labels[i]
    cache_hits = []
    cache_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_cache_agg.out'.format(name)))
    for line in cache_agg_file:
      if line.split(',')[0] != 'MISS':
        miss = float(line.strip().split(',')[0])
        hit = float(line.strip().split(',')[1])
        cache_hits.append( hit / (hit + miss) )

    tlb_hits = []
    tlb_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_tlb_agg.out'.format(name)))
    for line in tlb_agg_file:
      if line.split(',')[0] != 'MISS':
        miss = float(line.strip().split(',')[0])
        hit = float(line.strip().split(',')[1])
        tlb_hits.append( hit / (hit + miss) )

    cache_hits = np.array(cache_hits)
    tlb_hits = np.array(tlb_hits)

    cache_avg = np.average(cache_hits)
    cache_std = np.std(cache_hits)

    tlb_avg = np.average(tlb_hits)
    tlb_std = np.std(tlb_hits)
    cache_avgs.append(cache_avg)
    cache_stds.append(cache_std)
    tlb_avgs.append(tlb_avg)
    tlb_stds.append(tlb_std)
  
  plt.figure()
  plt.bar(np.arange(len(names)), cache_avgs, yerr=cache_stds, tick_label=labels)
  plt.title('Cache')
  plt.ylabel('Hit Rate')
  plt.savefig('cache_plot.png')

  plt.figure()
  plt.bar(np.arange(len(names)), tlb_avgs, yerr=tlb_stds, tick_label=labels)
  plt.title('TLB')
  plt.ylabel('Hit Rate')
  plt.savefig('tlb_plot.png')

compute_hierarchy_plot(dir, ['hotset_simple', 'rndm_write_simple', 'seq_write_simple'], ['Hotset', 'Random', 'Sequential'])

def compute_runtime_plot(agg_dir, names, labels, access):
  assert(len(names) == len(labels))

  runtime_avgs = []
  runtime_stds = []

  for i in range(len(names)):
    name = names[i]
    label = labels[i]

    runtime_agg_file = open(os.path.join(agg_dir, name, 'gups_{}_runtime_agg.out'.format(name)))
    runtime_stats = []
    for line in runtime_agg_file:
      if line.split(',')[0] != 'RUNTIME':
        runtime_stats.append((int(line.strip().split(',')[0])))

    runtime_stats = np.array(runtime_stats)

    runtime_avg = np.average(runtime_stats)
    runtime_std = np.std(runtime_stats)

    runtime_avgs.append(runtime_avg)
    runtime_stds.append(runtime_std)
  
  plt.figure()
  plt.bar(np.arange(len(names)), runtime_avgs, yerr=runtime_stds, tick_label=labels)
  plt.title('Memory Access Runtime')
  plt.yscale('log')
  plt.yticks([5e8, 1e9, 2e9])
  plt.savefig('runtime_{}_plot.png'.format(access))

runtime_experiments = [
  (args.agg_dir, ['hotset_linux', 'hotset_simple'],  ['linux', 'simple'], 'hotset'),
  (args.agg_dir, ['rndm_write_linux', 'rndm_write_simple'],  ['linux', 'simple'], 'rndm'),
  (args.agg_dir, ['seq_write_linux', 'seq_write_simple'], ['linux', 'simple'], 'seq'),
]

for exp in runtime_experiments:
  compute_runtime_plot(*exp)