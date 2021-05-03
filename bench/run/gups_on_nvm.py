#!/usr/bin/env python3
import math
import subprocess
import time
import csv
import os

import globals

from pathlib import Path

tries = 3


def run(gups_obj, out_dir, app_opts=""):
    Path(out_dir).mkdir(parents=True, exist_ok=True)

    cmd_str = "{} {}".format(gups_obj, app_opts)

    print("Running experiment: {}\t\t{}".format(cmd_str, out_dir))
    i = 0
    cmd = cmd_str.split(" ")
    while True:
        success = True
        i += 1
        print("Try #", i)
        try:
            start = time.time()
            os.environ["LD_PRELOAD"] = "libhemem.so"
            output = subprocess.run(cmd, check=True, capture_output=True)
            end = time.time()
            if time:
                with open("{}/e2e_time.out".format(out_dir), "w") as tf:
                    tf.write(str(end - start))
            print(output.stdout)
        except subprocess.CalledProcessError as err:
            print("Subprocess failed because:\n{}".format(err))
            success = False
        except Exception as e:
            print("Failed due to another reason -\n{}".format(e))
            break
        if not(i <= tries) or success:
            break


if __name__ == '__main__':
    exp_root = globals.experiment_name(__file__)
    # Gups options
    patterns = ["hotset"]
    exponents = [
        5,
        6,
        7,
        8,
    ]

    # Pin Options
    takes = [
        "1",    # 100%
        "10",   # 10%
        "100",  # 1%
        "1000", # 0.1%
    ]

    exp_names = globals.opts("takes-{}-th", takes)


    for pattern in patterns:
        gups_objs = globals.gups_obj(["rndm"], ["write"])
        for exp_name,  exponent in zip(exp_names, exponents):
            runs = 5
            for i in range(runs):
                gups_opt = "1 {} 30 45 10 50".format(10**exponent)
                name = "{}/{}/{}".format(pattern, exp_name, i)
                out_dir = globals.out_dir(exp_root, name)

                run(gups_objs[0], out_dir, app_opts=gups_opt)
