import math
import subprocess
import time
import csv


def run_experiment(cmd_str, tries=math.inf):
    print("\nRunning experiment:\n{}".format(cmd_str))
    i = 0
    cmd = cmd_str.split(" ")
    while True:
        success = True
        i += 1
        print("Try #", i)
        try:
            start = time.time()
            subprocess.run(cmd, check=True)
            end = time.time()
        except subprocess.CalledProcessError as err:
            print("Subprocess failed because:\n{}".format(err))
            success = False
        except:
            print("Failed due to another reason.")
            break

        if not(i <= tries) or success:
            if success:
                time_taken = end-start
                print("Ran successfully in {}\n".format(time_taken))
            break

if __name__ == '__main__':
    pin = 'pin -t ../sim/obj-intel64/mem_trace.so -o ./results/hist/hotset/gups_hotset_ --'

    gups_bin = '../apps/gups/obj/gups_rndm_write.o'
    gups_configs =[
        # '1 1000 15 8 0 0',
        # '1 10000 15 8 0 0',
        '1 100000 15 8 45 10',
        # '1 1000000 15 8 0 0',
        # '1 10000000 15 8 0 0',
        # '1 100000000 15 8 0 0',
    ]

    # Memory Access of GUPS + Pin

    for config in gups_configs:
        gups_cmd = gups_bin + ' ' + config
        pin_cmd = pin + ' ' + gups_cmd 
        run_experiment(pin_cmd)

