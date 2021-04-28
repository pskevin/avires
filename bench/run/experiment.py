#!/usr/bin/env python3

import globals
import subprocess
import time

class Experiment:
    def __init__(self, file, tries = 3, time=False):
        self.root = globals.experiment_name(file)
        self.tries = tries

    def run(self, cmd_str):
        print("\nRunning experiment:\n{}".format(cmd_str))
        i = 0
        cmd = cmd_str.split(" ")
        while True:
            success = True
            i += 1
            print("Try #", i)
            try:
                output = subprocess.run(cmd, check=True, capture_output=True)
                print(output.stderr)
                print(output.stdout)
            except subprocess.CalledProcessError as err:
                print("Subprocess failed because:\n{}".format(err))
                success = False
            except:
                print("Failed due to another reason.")
                break
            if not(i <= self.tries) or success:
                break

        