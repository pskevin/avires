import math
import subprocess


gups = "pin -t ../sim/obj-intel64/mem_trace.so -o ./results/sim_small_ -- ../apps/gups/obj/gups.o 1 10000 9 8".split(" ")
gups = "pin -t ../sim/obj-intel64/mem_trace.so -o ./results/sim_small_linux -mm 1_ -- ../apps/gups/obj/gups.o 1 10000 9 8".split(" ")
# print(gups)

def run_experiment(cmd, tries=math.inf):
    while True:
        success = True
        tries -= 1
        print("\nRun ",tries)
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as err:
            print("Fail", err)
            success = False
        # except:
        #     print("Another bigger fail")

        if not(tries > 0) or success:
            if success:
                print("\n\n\nSUCCESS\n\n\n")
            break
        

run_experiment(gups, tries = 10)
