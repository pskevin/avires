#!/usr/bin/env python3

import argparse
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import ticker
from matplotlib.pyplot import figure


parser = argparse.ArgumentParser()

parser.add_argument('--path', type=str, required=True)
parser.add_argument('--region-start', type=int, required=True)
parser.add_argument('--region-size', type=int, required=True)

args = parser.parse_args()

if __name__ == '__main__':
    print("Plotting Access Patterns from {} using the addrs.out file".format(args.path))
    data = np.genfromtxt(args.path+"/addrs.out", dtype=np.uint64, delimiter=',', names=True) 

    start = args.region_start
    end = start + args.region_size

    v = data['vaddr'][(data['vaddr'] >= start ) & (data['vaddr'] <= end)]
    page_size = 4*1024
    

    page_nums = np.floor((v - start)/ page_size)
    y = page_nums
    x = np.arange(page_nums.shape[0])



    plt.rcParams["figure.figsize"] = (8,8)
    plt.rcParams["figure.dpi"] = 400
    plt.rcParams['font.size'] = 16

    fig = plt.figure()
    ax = fig.add_subplot(111)

    plt.ylabel('Page Number', fontsize=18)
    ax.ticklabel_format(axis='y', style='sci', scilimits=(0,5))
    ax.yaxis.major.formatter._useMathText = True

    plt.xlabel('Timesteps', fontsize=18)
    ax.ticklabel_format(axis='x', style='sci', scilimits=(6,7))
    ax.xaxis.major.formatter._useMathText = True

    # For dense plots s=0.3, else s=1
    ax.scatter(x, y, s=0.3, marker=',', linewidths=0) 

    # plt.gcf().subplots_adjust(left=0.135)
    # plt.gcf().subplots_adjust(bottom=0.15)
    plt.draw()
    plt.savefig(args.path+"/addrs.png")