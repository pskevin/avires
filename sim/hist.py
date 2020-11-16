

import numpy as np
from matplotlib import pyplot as plt

# load the data with NumPy function loadtxt
data = np.loadtxt("addr_hist", delimiter=":", skiprows=-1)
x=np.array([i for i in range(1, len(data)+1)])
y=data[:,1]

plt.bar(x,data[:,1],align='center')
plt.show()
# theta, num_bins = read("addr_hist")
# n, bins, patches = plt.hist(theta, num_bins, range=[0,180], normed = True,      histtype='bar',facecolor='green')
# plt.xlabel(r'$\theta$($\degree$)')
# plt.ylabel(r'$P(\theta)$')
# plt.show()