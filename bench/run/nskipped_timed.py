#!/usr/bin/env python3
import math
import subprocess
import time
import csv
import os

import globals

from experiment import Experiment


if __name__ == '__main__':
    exp = Experiment(__file__, time=True, save_stdout=True)

    # Gups options
    patterns=["rndm"]
    gups_objs = globals.gups_obj(["rndm"], ["write"])
    iterations = [
        10 ** 5,
        10 ** 6,
        10 ** 7,
        10 ** 8,
    ]
    gups_opts = globals.opts("1 {} 30 0 100 100", iterations)

    # Pin Options
    takes = [
        "1",  # 100%
        "10",  # 10%
        "100",  # 1%
        "1000", #0.1%
    ]
    takes_opts = globals.opts("-take-every-nth {}", takes)
    pin_opts =  takes_opts

    runs = 5

    exp_names = globals.opts("takes-{}-th", takes)

    print(gups_objs)
    pin_obj = globals.pin_obj()

    for pattern, gups_obj in zip(patterns, gups_objs):
        for  exp_name, pin_opt, gups_opt in zip(exp_names, pin_opts, gups_opts):
            for run in range(runs):
                name = "{}/{}/{}".format(pattern,exp_name, run)
                out_dir = globals.out_dir(exp.root, name)
                exp.run(pin_obj, gups_obj, out_dir, pin_opts=pin_opt, app_opts=gups_opt)
