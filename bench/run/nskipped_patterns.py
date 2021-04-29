#!/usr/bin/env python3
import math
import subprocess
import time
import csv
import os

import globals

from experiment import Experiment


if __name__ == '__main__':
    exp = Experiment(__file__, time=True)

    # Gups options
    gups_objs = globals.gups_obj(["rndm"], ["write"])
    gups_opts = ["1 1000000 30 45 10 50"]


    # Pin Options
    takes = [
        "100",  # 1%
        "10",  # 10%
        "4",  # 25%
        "2",  # 50%
    ]
    takes_opts = globals.opts("-take-every-nth {}", takes)

    skips = [
        "4",  # 75%
        "10",  # 90%
        "100",  # 99%
        "1" # 100% (default)
    ]
    skips_opts = globals.opts("-skip-every-nth {}", skips)

    pin_opts =  skips_opts + takes_opts

    exp_names = globals.opts("skips-{}-th", skips) + globals.opts("takes-{}-th", takes)

    print(exp_names)

    pin_obj = globals.pin_obj()

    for  gups_obj, gups_opt in zip(gups_objs, gups_opts):
        for exp_name, pin_opt in zip(exp_names, pin_opts):
            out_dir = globals.out_dir(exp.root, exp_name)
            exp.run(pin_obj, gups_obj, out_dir, pin_opts=pin_opt, app_opts=gups_opt)
