#!/usr/bin/env python3

import argparse
import enum
import math
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.pyplot import figure
from matplotlib.ticker import ScalarFormatter


parser = argparse.ArgumentParser()

parser.add_argument("--gen-combined-timing",  type=bool, default=False)
parser.add_argument("--gen-invidual-access-patterns",  type=bool, default=False)
parser.add_argument("--gen-individual-freq-hist",  type=bool, default=False)
parser.add_argument("--gen-combined-freq-hist",  type=bool, default=False)


parser.add_argument("--pin-nskipped-time-path",  type=str, required=True)
parser.add_argument("--pin-all-time-path",  type=str, required=True)
parser.add_argument("--app-on-nvm-time-path",  type=str, required=True)

parser.add_argument('--names', nargs="+", type=str, required=True)
parser.add_argument('--addr-paths', nargs="+", type=str, required=True)
parser.add_argument('--region-starts', nargs="+", type=int, required=True)
parser.add_argument('--region-sizes', nargs="+", type=int, required=True)


args = parser.parse_args()

if __name__ == '__main__':
    if(not (len(args.addr_paths) == len(args.names) == len(args.region_starts) == len(args.region_sizes))):
        exit("Number of values for names({}), addr-paths({}), region-starts({}), region-sizes({}) arguments must be the same."\
            .format(args.addr_paths,args.names,args.region_starts,args.region_sizes))

    out_root_path = "/".join(args.pin_nskipped_time_path.split("/")[:-1])

    if args.gen_combined_timing:
        print("Plotting Combined Time Comparisons")
        clock = 4 * 10**6
        bar_width = 0.35
        x = np.arange(len(args.names))

        font_size = 16
        plt.rcParams["figure.figsize"] = (8,8)
        plt.rcParams["figure.dpi"] = 400
        plt.rcParams['font.size'] = font_size

        fig, ax = plt.subplots()
        ax.grid(True, which="major", axis="y", color="k", alpha=0.75)
        ax.set_axisbelow(True)
        color = ["C{}".format(i) for i in range(3)]

        app_time = np.load(args.app_on_nvm_time_path)
        app_time = app_time / clock
        mean, std = app_time.mean(1), app_time.std(1)
        rect_app = ax.bar(x - bar_width/3, mean, bar_width/3, label='GUPS on NVM', color=color[0])


        pin_nskipped_time = np.load(args.pin_nskipped_time_path)
        pin_nskipped_time = pin_nskipped_time / clock
        mean, std = pin_nskipped_time.mean(1), pin_nskipped_time.std(1)
        rect_skipped_pin = ax.bar(x, mean, bar_width/3, label='Partially observed Memsim-simulated GUPS', color=color[1])

        pin_all_time = np.load(args.pin_all_time_path)
        pin_all_time = pin_all_time / clock
        rect_all_pin = ax.bar(x + bar_width/3, pin_all_time, bar_width/3, label='Completely observed Memsim-simulated GUPS', color=color[2])



        ax.set_ylabel('Execution Latency (ms)', fontsize=font_size+2)
        ax.set_yscale("log")

        ax.set_xlabel('Number of GUPS iterations', fontsize=font_size+2)

        ax.set_xticks(x)
        ax.set_xticklabels(["100000", "1000000", "10000000", "100000000"], fontsize=font_size+2)

        ax.legend(fontsize=font_size-2)

        padding = 0.5
        for i, r in enumerate(rect_skipped_pin):
            ax.text(r.get_x() + r.get_width()/2, (r.get_y()+r.get_height())*1.1, \
                "{}".format(args.names[i]), \
                ha='center', va='bottom', fontsize=font_size-2, rotation=90, color=color[1], weight="bold", style="oblique")

        # Save the figure and show
        out_path = out_root_path + "/timed.png"
        print("Saving combined time comparisons at {}".format(out_path))
        plt.savefig(out_path)
        plt.close(fig)


    if  args.gen_invidual_access_patterns or args.gen_individual_freq_hist or args.gen_combined_freq_hist:
            
        # Load all addrs
        page_size = 4*1024
        addrs = [0 for i in range(len(args.addr_paths))]
        page_nums = [0 for i in range(len(args.addr_paths))]
        density = [0 for i in range(len(args.addr_paths))]
    
        for i, (path, start, size) in enumerate(zip(args.addr_paths, args.region_starts, args.region_sizes)):
            print(i, path, start, size)
            end = start+size
            data = np.genfromtxt(path+"/addrs.out", dtype=np.uint64, delimiter=',', names=True) 
            addrs[i] = data['vaddr'][(data['vaddr'] >= start ) & (data['vaddr'] <= end)]
    
            print("Observed Accesses {}".format(addrs[i].shape))
            
            page_nums[i] = np.floor((addrs[i] - start)/ page_size)

            if args.gen_invidual_access_patterns:
                print("Plotting Individual Access Pattern")
                y = page_nums[i]
                x = np.arange(page_nums[i].shape[0])


                plt.rcParams["figure.figsize"] = (8,8)
                plt.rcParams["figure.dpi"] = 400
                plt.rcParams['font.size'] = 16
                

                fig, ax = plt.subplots()

                plt.ylabel('Page Number', fontsize=18)
                y_limit = int(math.floor(math.log10(page_nums[i].max())))
                ax.ticklabel_format(axis='y', style='sci', scilimits=(y_limit,y_limit))
                ax.yaxis.major.formatter._useMathText = True
                
                plt.xlabel('Timesteps', fontsize=18)
                ax.ticklabel_format(axis='x', style='sci', scilimits=(6,7))
                ax.xaxis.major.formatter._useMathText = True
                
                # For dense plots s=0.3, else s=1
                ax.scatter(x, y, s=0.3, marker=',', linewidths=0) 
                
                out_path = path+"/scatter.png"
                print("Saving Individual Access Pattern at {}".format(out_path))
                plt.savefig(out_path)
                plt.close(fig)
        

        if args.gen_combined_freq_hist:
            print("Plotting Combined Histogram")
            font_size = 24
            plt.rcParams["figure.figsize"] = (10,25)
            plt.rcParams["figure.dpi"] = 400
            plt.rcParams['font.size'] = font_size

            fig, ax = plt.subplots()
        
        
            plt.ylabel('Number of Pages', fontsize=font_size+2)
            ax.ticklabel_format(axis='y', style='sci', scilimits=(4,5))
            ax.yaxis.major.formatter._useMathText = True
            # ax.set_yscale("log")
            
            plt.xlabel('Access Frequency', fontsize=font_size+2)
            # ax.set_xscale("log")

            ax.grid(True, which="major", axis="both", color="k", alpha=0.75)
            ax.set_axisbelow(True)

            for i in range(len(args.names)):
                # 0 = Page Number, 1 = Occurences of a Page Number
                page_num_count = np.unique(page_nums[i], return_counts=True) 
                #  0 = Occurences of a Page Number, 1 = Frequency of a Count
                count_frequency = np.unique(page_num_count[1], return_counts=True)
                x, y = count_frequency[0], count_frequency[1]

                x_before, y_before = np.array([0, 0]), np.array([0,y[0]])
                x_after, y_after = np.array([x[-1]]), np.array([0])

                y_ = np.append(np.append(y_before,y),y_after)
                x_ = np.append(np.append(x_before,x), x_after)

                color = 'C{}'.format(i)

                label = args.names[i]
                if args.names[i] == "100%":
                    label += " of 100000"
                elif args.names[i] == "10%":
                    label += " of 1000000"
                elif    args.names[i] == "1%":
                    label += " of 10000000"
                else:
                    label += " of 100000000"

                ax.plot(x_, y_, color=color, label=label, linewidth=3)
                ax.fill_between(x_, y_, color=color, alpha=0.2)
                
            plt.legend(fontsize=font_size-2)
        
            out_path = out_root_path + "/hist.png"
            print("Saving combined frequency histograms at {}".format(out_path))
            plt.savefig(out_path)
            plt.close(fig)