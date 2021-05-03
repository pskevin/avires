#!/usr/bin/env python3

from matplotlib import pyplot as plt
from matplotlib import ticker
from matplotlib.pyplot import figure

if __name__ == '__main__':

    # Pin Options
    x = [
        "100",
        "75",
        "50",
        "25",
        "10",
        "1",
    ]

    y = [
        38856097504,  # 100
        31571982364,   # 75
        22189782338,  # 50
        13339753062, # 25
        8196439614, # 10
        4062985550, # 1
    ]

    clock = 2340.852 * 10**6
    
    y = [t / clock for t in y]

    plt.rcParams["figure.dpi"] = 400

    fig = plt.figure()
    ax = fig.add_subplot(111)

    ax.bar(x, y)

    plt.ylabel('Time (s)')
    plt.xlabel('Observed Accesses (%)')
    plt.draw()
    plt.savefig("/root/memsim/bench/nskipped_patterns/e2e_timings.png")