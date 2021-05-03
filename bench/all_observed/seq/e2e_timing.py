#!/usr/bin/env python3

import numpy as np

if __name__ == '__main__':

    # Gups Options
    x = [
        "100",
        "10",
        "1",
        "0.1"
    ]

    y = [
        np.mean([1493868026, 1466374092, 1485555744, 1484383756, 1486741024]),
        np.mean([2816764610, 2886036982, 2893119234, 2893619172]),
        np.mean([16478627022, 16974906242, 16978296866]),
        np.mean([160119594356, 151899981198]),
     ]

    np.save("/root/memsim/bench/all_observed/seq/all_observed", y)