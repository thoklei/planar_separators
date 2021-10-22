"""
Utility functions and data to analyze data
"""
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import argparse

# mapping algorithm names to color
cmap = {"LT": "#092cbe",
        "LT_NE": "#536bd2",
        "LT_DMD": "#9dabe5",
        "LT_DMD_NE": "#9dabe5",
        "LT_NE_DMD": "#9dabe5",
        "LTFC": "#be0986",
        "LTFC_NE": "#d253aa",
        "LTFC_DMD": "#e59dcf",
        "LTFC_DMD_NE": "#9dabe5",
        "LTFC_NE_DMD": "#9dabe5",
        "Dual": "#be9b09",
        "Dual_NE": "#d2b953",
        "Dual_DMD": "#e5d79d",
        "Dual_DMD_NE": "#9dabe5",
        "Dual_NE_DMD": "#9dabe5",
        "DualFC": "#09be40",
        "DualFC_NE": "#53d279",
        "DualFC_DMD": "#9de5b3",
        "DualFC_DMD_NE": "#9dabe5",
        "DualFC_NE_DMD": "#9dabe5",
        "HP": "#df150e",
        "HP_NE": "#e95b56",
        "HP_DMD": "#f2a19f",
        "HP_DMD_NE": "#9dabe5",
        "HP_NE_DMD": "#9dabe5"}

# maps core algorithm to scatterplot marker
mmap = {"LT": "o",
        "LTFC": "^",
        "Dual": "",
        "DualFC": "s",
        "HP": "*",
}

core_algorithms = [alg for alg in cmap.keys() if not "_" in alg]
simple_postprocessors = [alg for alg in cmap.keys() if alg.count("_") <= 1]
complex_postprocessors = [alg for alg in cmap.keys() if alg.count("_") != 1]

def create_algo_plot(results, name, title, xlabel, ylabel, show):
    """
    Plots a dictionary mapping algorithms to some value as a bar chart.

    :param results: dictionary mapping algorithm name to value
    :param name: filename under which this plot should be stored
    :param title: title of the diagram
    :param xlabel: labels of x-axis
    :param ylabel: labels of y-axis
    :param show: whether to show the plot or not
    """

    colors = [cmap[alg] for alg in list(results.keys())]

    plt.figure()
    xs = [i for i in range(len(results))]
    plt.title(title)
    plt.ylabel(ylabel)
    plt.xlabel(xlabel)
    plt.bar(xs, [results[alg][0] for alg in results], tick_label=list(results.keys()), width=0.6, color=colors)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/"+name+".png")
    if show:
        plt.show()


def create_scatter_plot(instances, algorithms, results, name, title, xlabel, ylabel, show):
    """
    Plots a dictionary mapping instance names to values for each algorithm.

    :param results: dictionary algorithm ->(dictionary of instance->performance)
    :param name: filename under which this plot should be stored
    :param title: title of the diagram
    :param xlabel: labels of x-axis
    :param ylabel: labels of y-axis
    :param show: whether to show the plot or not
    """
    plt.figure()
    xs = [i for i in range(len(results))]
    plt.title(title)
    plt.ylabel(ylabel)
    plt.xlabel(xlabel)

    xs = range(len(instances))
    for algo in algorithms:
        ys = [results[algo][inst] for inst in instances]
        plt.scatter(xs, ys, c=cmap[algo], marker=mmap[algo])
    plt.xticks(xs, instances, rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/"+name+".png")
    if show:
        plt.show()