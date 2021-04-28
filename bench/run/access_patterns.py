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
    gups_objs = globals.gups_obj(["rndm", "seq", "rndm"], ["read"])
    gups_opts = [
        "1 10000 25 0 0 0", 
        "1 10000 25 0 0 0",
        "1 10000 25 40 20 50"
    ]

    pin_obj = globals.pin_obj()


    for exp_name, gups_obj, gups_opt in zip(patterns, gups_objs, gups_opts):
        out_dir = globals.out_dir(exp.root, exp_name)
        Path(out_dir).mkdir(parents=True, exist_ok=True)
        out_opt = "-o {}".format(out_dir)
        cmd = globals.cmd(pin_obj, gups_obj, pin_opts=out_opt, app_opts=gups_opt)
        exp.run(cmd)


"""

Running experiment:
pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/rndm -- /root/memsim/apps/gups/obj/gups_rndm_read.o 1 10000 25 0 0 0
Try # 1
b'End-to-End exection time - 1237589866\nWriting observed values.\nTime taken to write 550798000\n'
b'Region start: 140149515116544 or 0x7f771a106000 \tRegion size: 33554432\nAccessed 140149515116644\n'

Running experiment:
pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/seq -- /root/memsim/apps/gups/obj/gups_seq_read.o 1 10000 25 0 0 0
Try # 1
b'End-to-End exection time - 1233461630\nWriting observed values.\nTime taken to write 540357184\n'
b'Region start: 139907377709056 or 0x7f3eb98cc000 \tRegion size: 33554432\nAccessed 139907377709156\n'

Running experiment:
pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/hotset -- /root/memsim/apps/gups/obj/gups_rndm_read.o 1 10000 25 40 20 50
Try # 1
b'End-to-End exection time - 1198631130\nWriting observed values.\nTime taken to write 518766242\n'
b'Region start: 140510295560192 or 0x7fcb1a411000 \tRegion size: 33554432\nAccessed 140510295560292\n'
"""