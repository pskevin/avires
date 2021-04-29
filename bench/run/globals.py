#!/usr/bin/env python3

import os
from itertools import product

dir_name = os.path.dirname(__file__)


gups_obj_dir = os.path.abspath(os.path.join(dir_name, "../../apps/gups/obj/"))
pin_obj_dir = os.path.abspath(os.path.join(dir_name, "../../src/obj-intel64/"))


obj_map = {
    "pin": "main.so",
    "gups": {
        "rndm":{
            "read": "gups_rndm_read.o",
            "write": "gups_rndm_write.o"
        },
        "seq":{
            "read": "gups_seq_read.o",
            "write":  "gups_seq_write.o"
        },
    }
};

def pin_obj():
    return os.path.join(pin_obj_dir, obj_map["pin"])

def gups_obj(patterns, types):
    objs = []
    for (pattern, type) in product(patterns, types):
        objs.append(os.path.join(gups_obj_dir, obj_map["gups"][pattern][type]))
    return objs

out_dir_root = os.path.abspath(os.path.join(dir_name,"../"))

print(out_dir_root)

def out_dir(exp_root, exp_name):
    return os.path.join(out_dir_root,exp_root,exp_name)

def experiment_name(file): return os.path.basename(file).split(".")[0]

def cmd(pin_obj,  app_obj, pin_opts="", app_opts=""): 
    if pin_opts == "":
        print("No pin opts")
        return 'pin -t {} -- {} {}'.format(pin_obj, app_obj,app_opts)
    return 'pin -t {} {} -- {} {}'.format(pin_obj,pin_opts,app_obj,app_opts)

def opts(fmt, *conf):
    return [fmt.format(*combination) for combination in product(*conf)]
