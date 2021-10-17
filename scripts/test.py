"""
This script plots some data to compare old and new triangulation method.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl


def main():
    x = [5000,  10000, 20000,  40000, 80000]
    data_old = [319.9, 1133,  2047.6, 7607.9, 29506.8]
    data_new = [103.4, 218.8, 241.5, 402.3, 921.4]

    plt.figure()
    mpl.style.use('seaborn')
    plt.title("triangulate (red) vs triangulateFast (green)")
    plt.xlabel("nodes")
    plt.ylabel("time (ms)")
    plt.plot(x, data_old, 'C2')
    plt.plot(x, data_new, 'C1')
    plt.savefig("../results/plots/triangulation.png")
    plt.show()


if __name__ == "__main__":
    main()
