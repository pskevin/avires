import argparse
from matplotlib import pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--runtime_file', type=str, required=True)
parser.add_argument('--timesteps', type=int, required=True)
parser.add_argument('--title', type=str, required=True)
parser.add_argument('--out_name', type=str, required=True)

args = parser.parse_args()

with open(args.runtime_file) as runtime_file:
  timesteps = sum(1 for _ in runtime_file)

MAX_TIMESTEPS = args.timesteps

if timesteps > MAX_TIMESTEPS:
  timesteps = MAX_TIMESTEPS


runtimes = np.zeros((timesteps))

with open(args.runtime_file) as runtime_file:
  line_idx = 0
  for line in runtime_file:
    if line_idx > 0:
      runtime = int(line.strip().split(',')[0])

      # We found some memory accesses way outside of the application's space (probably b/c of pin). This causes the simulator to falsely add a lot of delay
      if runtime < 7000:
        runtimes[line_idx] = runtime
        
    line_idx += 1
    if line_idx >= timesteps:
      break

plt.scatter(np.arange(timesteps), runtimes, s=0.25)
plt.ylabel('Memory Access Runtime')
plt.xlabel('Application Time')
plt.title(args.title)
plt.savefig(args.out_name)
