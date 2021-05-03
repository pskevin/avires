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
        np.mean([5492542056, 5541095738, 5594693386, 5504239160, 5492542056]),
        np.mean([40376767274, 38728696938, 39648886620, 39467274170]),
        np.mean([366441751744, 379476305224, 360420022442]),
        np.mean([3641945366124, 3575919041330]), # Update Later
    ]


    
    np.save("/root/memsim/bench/all_observed/hotset/all_observed", y)