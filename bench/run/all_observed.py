#!/usr/bin/env python3
import math
import subprocess
import time
import csv
import os

import globals

from experiment import Experiment

from pathlib import Path

if __name__ == '__main__':
    exp = Experiment(__file__, save_stdout=True)

    gups_objs = ["hotset"]
    gups_exponents = [5,6,7,8]

    pin_obj = globals.pin_obj()

    for pattern in gups_objs:
        gups_obj = globals.gups_obj(["rndm"], ["write"])
        for ex in gups_exponents:
            gups_opt = "1 {} 30 45 10 50".format(10**ex)
            runs = 10-ex
            for run in range(runs):
                exp_name = '{}/{}/{}'.format(pattern, ex, run)   
                out_dir = globals.out_dir(exp.root, exp_name)
                exp.run(pin_obj, gups_obj[0], out_dir, app_opts=gups_opt)
