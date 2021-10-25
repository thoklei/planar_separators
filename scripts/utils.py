"""
Utility functions and data to analyze data
"""
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

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
        "Dual": "x",
        "DualFC": "s",
        "HP": "*"}

core_algorithms = [alg for alg in cmap.keys() if not "_" in alg]
simple_postprocessors = [alg for alg in cmap.keys() if alg.count("_") <= 1]
complex_postprocessors = [alg for alg in cmap.keys() if alg.count("_") != 1]


def get_color(algorithm):
    """
    Gets the characteristic color for an algorithm, black by default.

    :param algorithm: the name of the algorithm
    :return: the characteristic color
    """
    if algorithm in cmap:
        return cmap[algorithm]
    else:
        print("WARNING: using unkown algorithm")
        return "#000000"


def get_marker(algorithm):
    """
    Gets the characteristic marker for an algorithm, dot by default.

    :param algorithm: the name of the algorithm
    :return: the characteristic marker
    """
    if algorithm in mmap:
        return mmap[algorithm]
    else:
        print("WARNING: using unkown algorithm")
        return "."


def filter_instances(instance_names, dir_names):
    """
    Filter instances by directory name, i.e. given a list of instance names and a list of directory names,
    it only retains the instances that are in those directories.

    :param instance_names: list of all instance names
    :param dir_names: list of the names of the target directories
    :return: filtered list of instance names
    """
    filtered = []
    for instance in instance_names:
        for directory in dir_names:
            if instance.startswith(directory):
                filtered.append(instance)
    return filtered


def analyze_separator_size(df, name, algorithms, instances):
    """
    Plots the relative separator sizes for core algorithms as a bar chart, across all instances

    :param df: the main dataframe
    :param name: the name of the resulting file
    :param algorithms: list of strings, algorithm identifiers
    :param instances: list of strings, instance identifiers
    """

    # maps algorithm to relative average separator size
    algo_results = dict()
    for algo in algorithms:
        algo_results[algo] = []

    for instance in instances:

        # reduce df to just this instance
        inst_df = df[df['instance'] == instance]
        mini = inst_df['sep_size'].min()

        if mini != 0:  # happens for e.g. San Francisco - we are ignoring those instances

            # for each algorithm, calculate average separator size and standard deviation
            for algo in algorithms:
                algo_df = inst_df[inst_df['algorithm'] == algo]
                mean_sep_size = algo_df['sep_size'].mean()
                algo_results[algo].append(mean_sep_size / mini)

    create_algo_plot(algo_results, name, "Average separator size relative to smallest known separator", "algorithm", "relative average separator size", True)


def analyze_instance_performance(df, name, instances, algorithms):
    """
    Creates a scatterplot to visualize relative algorithm performance for each instance.

    :param df: the main dataframe
    :param name: the name of the resulting file
    :param instances: list of strings, instance identifiers
    :param algorithms: list of strings, algorithm identifiers
    """

    clean_instances = [] # instances without those that ha

    # maps algorithm to dictionary instance -> relative average separator size
    algo_results = dict()
    for algo in algorithms:
        algo_results[algo] = {}

    for instance in instances:

        # reduce df to just this instance
        inst_df = df[df['instance'] == instance]
        mini = inst_df['sep_size'].min()

        if mini != 0:  # happens for e.g. San Francisco - we are ignoring those instances

            clean_instances.append(instance)

            # for each algorithm, calculate average separator size and standard deviation
            for algo in algorithms:
                algo_df = inst_df[inst_df['algorithm'] == algo]
                mean_sep_size = algo_df['sep_size'].mean()
                algo_results[algo][instance] = mean_sep_size / mini

    create_scatter_plot(clean_instances, algorithms, algo_results, name,
                        "Average separator size relative to smallest known separator per instance",
                        "instance", "relative average separator size", True)


def extract_short_instance_name(full_instance_name):
    """
    Extracts the short instance name from the full instance name with directory.

    :param full_instance_name: the full name of the instance
    :return: the shorter name
    """
    start = full_instance_name.rfind("/")
    return full_instance_name[start:]


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

    colors = [get_color(alg) for alg in list(results.keys())]

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

    :param instances: list of instance names
    :param algorithms: list of algorithm names
    :param results: dictionary algorithm ->(dictionary of instance->performance)
    :param name: filename under which this plot should be stored
    :param title: title of the diagram
    :param xlabel: labels of x-axis
    :param ylabel: labels of y-axis
    :param show: whether to show the plot or not
    """
    plt.figure()
    plt.title(title)
    plt.ylabel(ylabel)
    plt.xlabel(xlabel)

    xs = range(len(instances))
    for algo in algorithms:
        ys = [results[algo][inst] for inst in instances]
        plt.scatter(xs, ys, c=get_color(algo), marker=get_marker(algo))
    plt.xticks(xs, [extract_short_instance_name(inst) for inst in instances], rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/"+name+".png")
    if show:
        plt.show()


# def analyze_runtime(df, algorithms):
#     instances = df['instance'].unique()
#
#     results = dict()
#     for alg in algorithms:
#         results[alg] = []
#
#     # df['rel_sep_size'] = df.apply (lambda row: func(row), axis=1)
#
#     for instance in instances:
#         inst_df = df[df['instance'] == instance]
#         mini = max(inst_df['time'].min(), 0.5)
#
#         assert(mini > 0)
#
#         def relative(row):
#             return row['time'] / mini
#
#         inst_df['rel_time'] = inst_df.apply(lambda row: relative(row), axis=1)
#
#         print(inst_df)
#
#         for alg in algorithms:
#             results[alg].append(inst_df[inst_df['algorithm'] == alg]['rel_time'].values[0])
#
#         print(results)
#
#     colors = [cmap[alg] for alg in algorithms]
#
#     plt.figure()
#     xs = [i for i in range(len(algorithms))]
#     plt.title("average runtime relative to fastest algorithm")
#     plt.ylabel("average runtime")
#     plt.xlabel("algorithm")
#     plt.bar(xs, [np.mean(results[alg]) for alg in algorithms], tick_label=algorithms, width=0.6, color=colors)
#     plt.xticks(rotation=45, ha='right')
#     plt.tight_layout()
#     plt.savefig("../results/plots/runtime.png")
#     plt.show()
#
# def analyze_runtime_development(dataframe):
#     algorithms = ['LT', 'LTFC', 'Dual', 'DualFC', 'HP']
#
#     def get_runtimes(df, algorithm):
#         runtimes = []
#         averages = []
#         df_alg = df[df['algorithm'] == algorithm]
#         df_alg = df_alg.sort_values('nodes')
#         # print(df_alg)
#         instances = [inst for inst in df_alg['instance'].unique() if 'random' in inst]
#
#         for inst in instances:
#             runtimes.append(df_alg.loc[df_alg['instance'] == inst, 'time'].values[0] / 1000000)
#
#         # print(runtimes)
#
#         for i in range(0, len(runtimes), 3):
#             averages.append(np.mean([runtimes[i], runtimes[i+1], runtimes[i+2]]))
#
#         return averages
#
#     sizes = dataframe[dataframe['instance'].str.contains('random')].sort_values('nodes')['nodes'].unique()
#     print("sizes:", sizes)
#     plt.figure()
#     for alg in algorithms:
#         plt.plot(sizes, get_runtimes(dataframe, alg), color=cmap[alg], label=alg)
#     plt.title("runtime development of core algorithms")
#     plt.xlabel("instance size (nodes)")
#     plt.ylabel("runtime (s)")
#     plt.legend()
#     plt.tight_layout()
#     plt.savefig("../results/plots/runtime_dev.png")
#     plt.show()
#
#
# def analyze_individual_instance(df, name):
#
#     df = df[df['instance'] == name]
#     algorithms = df['algorithm'].unique()
#
#     diam = not ('diameter' in name)  # plot diameter bounds only for sensible instances
#
#     print(df)
#
#     data = [df.loc[df['algorithm'] == alg, 'mean_sep_size'].values[0] for alg in algorithms]  # average sep size
#     minis = [df.loc[df['algorithm'] == alg, 'min_sep_size'].values[0] for alg in algorithms]
#     diam_lb = df['diam_lB'].iloc[0]
#     diam_ub = df['diam_uB'].iloc[0]
#     n = df['nodes'].iloc[0]
#
#     colors = [utils.cmap[alg] for alg in algorithms]
#
#     plt.figure()
#     xs = [i for i in range(len(algorithms))]
#     plt.title("separator size for " + name + " (" + str(n) + " nodes)")
#     plt.ylabel("average separator size (nodes)")
#     plt.xlabel("algorithm and postprocessor")
#     print("results: ", [df.loc[df['algorithm'] == alg, 'mean_sep_size'].values[0] for alg in algorithms])
#     if diam:
#         plt.axhline(2*diam_lb+1, c='green')
#         plt.axhline(2*diam_ub+1, c='red')
#     plt.bar(xs, data, tick_label=algorithms, width=0.6, color=colors, zorder=0)
#     plt.scatter(xs, minis, c='white', marker='_', zorder=10)
#     plt.xticks(rotation=45, ha='right')
#     plt.tight_layout()
#     plt.savefig("../results/plots/instances/"+name+".png")
#     plt.close()
#     # plt.show()

