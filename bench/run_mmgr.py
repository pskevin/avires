import math
import subprocess
import time
import csv
from itertools import permutations

def get_permutation(l1, l2):
    print(l1, l2)
    print(list(permutations(l1,len(l2))))
    return [list(zip(x,l2)) for x in permutations(l1,len(l2))]


def run_experiment(cmd_str, tries=math.inf):
    print("\nRunning experiment:\n{}".format(cmd_str))
    i = 0
    cmd = cmd_str.split(" ")
    while True:
        success = True
        i += 1
        print("Try #", i)
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as err:
            print("Subprocess failed because:\n{}".format(err))
            success = False
        except:
            print("Failed due to another reason.")
            break

        if not(i <= tries) or success:
            if success:
                print("Ran successfully")
            break

if __name__ == '__main__':
    pin = 'pin -t {} {} {} -- {} {}'

    mmgrs = {
        'simple': '-mm 0',
        'linux': '-mm 1'
    }

    pin_bins = {
        'rndm': '../sim/obj-intel64/mem_trace_rndm.so',
        'seq': '../sim/obj-intel64/mem_trace_seq.so'
    }

    gups_bins = {
            'rndm': [
                '../apps/gups/obj/gups_rndm_write.o',
                '../apps/gups/obj/gups_rndm_read.o',
                '../apps/gups/obj/gups_hotset.o'
            ],
            'seq': [
            '../apps/gups/obj/gups_seq_write.o',
            '../apps/gups/obj/gups_seq_read.o',
            ]
    }

    for (pin_type, gups_type) in zip(pin_bins, gups_bins):
        pin_bin = pin_bins[pin_type]
        for gups_bin in gups_bins[gups_type]:
            experiment = gups_bin.split('gups_')[-1].split('.')[0]
            for (mmgr, flag) in mmgrs.items():
                name = '{}_{}'.format(experiment, mmgr)
                out = '-o ./results/{}/gups_{}_'.format(name, name)
                if experiment == 'hotset':
                    cmd = pin.format(pin_bin, flag, out, gups_bin, '1 100000 15 8 35 30 90')
                else:
                    cmd = pin.format(pin_bin, flag, out, gups_bin, '1 100000 15 8 0 0 0')
                run_experiment(cmd)
