#!/usr/bin/env python3

import globals
import subprocess
import time

from pathlib import Path


class Experiment:
    def __init__(self, file, tries = 3, time=False, save_stdout=False):
        self.root = globals.experiment_name(file)
        self.tries = tries
        self.time = time
        self.save_stdout = save_stdout

    def run(self, pin_obj, gups_obj, out_dir, pin_opts="", app_opts=""):
        print("\nChecking output directory {} exists".format(out_dir))
        Path(out_dir).mkdir(parents=True, exist_ok=True)
        out_opt = "-o {}".format(out_dir)
        if pin_opts == "":
            # pin_opts = out_opt
            pin_opts = ""
        else:
            pin_opts = out_opt + " " + pin_opts

        cmd_str = globals.cmd(pin_obj, gups_obj, pin_opts=pin_opts, app_opts=app_opts)

        print("Running experiment: \n{}".format(cmd_str))
        i = 0
        cmd = cmd_str.split(" ")
        while True:
            success = True
            i += 1
            print("Try #", i)
            try:
                start, end = 0, 0
                if(self.save_stdout):
                    with open("{}/stdout.out".format(out_dir), "w") as stdf:
                        start = time.time()
                        subprocess.run(cmd, check=True, stdout=stdf)
                        end = time.time()    
                else:
                    start = time.time()
                    output = subprocess.run(cmd, check=True, capture_output=True)
                    end = time.time()
                    print(output.stdout)
                if self.time:
                    with open("{}/e2e_time.out".format(out_dir), "w") as tf:
                       tf.write(str(end - start))
            except subprocess.CalledProcessError as err:
                print("Subprocess failed because:\n{}".format(err))
                success = False
            except Exception as e:
                print("Failed due to another reason -\n{}".format(e))
                break
            if not(i <= self.tries) or success:
                break

        