#!/usr/bin/env python3

import numpy as np

if __name__ == '__main__':

    # Gups Options
    x = [
        "100",
        "10",
        "1",
        "0.1"
    ]

    y = [
        np.mean([5569253388, 5654740014, 5514956252, 5568195476, 5388374388]),
        np.mean([40689063100, 38927799334, 38778977512, 39269245366]),
        np.mean([367560476904, 382000407124, 350876134140]),
        np.mean([3641945366124, 3575919041330]),
    ]


    
    np.save("/root/memsim/bench/all_observed/rndm/all_observed", y)