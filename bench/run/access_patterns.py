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
    exp = Experiment(__file__)

    patterns = ["rndm", "seq", "hotset"]
    gups_objs = globals.gups_obj(["rndm", "seq", "rndm"], ["write"])
    gups_opts = [
        "1 1000000 30 0 0 0", 
        "1 1000000 30 0 0 0",
        "1 1000000 30 40 20 50"
    ]

    pin_obj = globals.pin_obj()


    for exp_name, gups_obj, gups_opt in zip(patterns, gups_objs, gups_opts):
        out_dir = globals.out_dir(exp.root, exp_name)
        exp.run(pin_obj, gups_obj, out_dir, app_opts=gups_opt)