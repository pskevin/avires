#!/usr/bin/env python3

from matplotlib import pyplot as plt
from matplotlib import ticker
from matplotlib.pyplot import figure

if __name__ == '__main__':

    # Pin Options
    x = [
        "100",
        # "99",
        # "90",
        "75",
        "50",
        "25",
        "10",
        "1",
    ]


    y = [
        85.42831039428711,  # 100
        # 84.33391380310059,  # 99
        # 77.6902859210968,   # 90
        65.3280394077301,   # 75
        48.76440668106079,  # 50
        23.408289909362793, # 25
        10.664235353469849, # 10
        2.5745015144348145, # 1
    ]
    
    plt.rcParams["figure.dpi"] = 400

    fig = plt.figure()
    ax = fig.add_subplot(111)

    ax.bar(x, y)

    plt.ylabel('Time (s)')
    plt.xlabel('Observed Accesses (%)')
    plt.draw()
    plt.savefig("/root/memsim/bench/nskipped_patterns/e2e_timings.png")