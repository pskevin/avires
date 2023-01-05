# Avires
Avires: Simulating Tiered Memory Architectures
using Dynamic Binary Instrumentation

Experimentation Setup:
1. Install Intel PIN tool and add to system path (following the instructions at https://software.intel.com/sites/landingpage/pintool/docs/98275/Pin/html/index.html#BuildingOutOfKit)

From the top level:
2. `cd apps/gups && make` 
3. `cd sim/ && make`
Note, if you have to re-compile the simulator (e.g. after changing constants), you have to `make clean` before `make` because of the dynamic linking.

4. Run pin commands. Some sample commands can be seen in `run_experiments.sh`.
