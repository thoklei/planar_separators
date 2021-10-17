"""
This script analyses the experimental data.
More specifically:
    1.
"""
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import argparse

# mapping algorithm names to color
cmap = {"LT": "#092cbe",
        "LT_NE": "#536bd2",
        "LT_DMD": "#9dabe5",
        "LTFC": "#be0986",
        "LTFC_NE": "#d253aa",
        "LTFC_DMD": "#e59dcf",
        "Dual": "#be9b09",
        "Dual_NE": "#d2b953",
        "Dual_DMD": "#e5d79d",
        "DualFC": "#09be40",
        "DualFC_NE": "#53d279",
        "DualFC_DMD": "#9de5b3",
        "HP": "#df150e",
        "HP_NE": "#e95b56",
        "HP_DMD": "#f2a19f"}


def analyze_separator_size(df, name):
    """

    :param df:
    :param name:
    :return:
    """

    # extract instance and algorithm names
    instances = df['instance'].unique()
    algorithms = df['algorithm'].unique()

    results = dict()  # maps algorithm to average separator size
    for alg in algorithms:
        results[alg] = []

    # df['rel_sep_size'] = df.apply (lambda row: func(row), axis=1)

    for instance in instances:
        inst_df = df[df['instance'] == instance]
        mini = inst_df['min_sep_size'].min()
        print("Minimum for ", instance, ": ", mini)

        if mini != 0:

            def relative(row):
                return row['mean_sep_size'] / mini

            inst_df['rel_sep_size'] = inst_df.apply(lambda row: relative(row), axis=1)

            print(inst_df)

            for alg in algorithms:
                results[alg].append(inst_df[inst_df['algorithm'] == alg]['rel_sep_size'].values[0])

            print(results)

    colors = [cmap[alg] for alg in algorithms]

    # plot the results and store the figure
    plt.figure()
    xs = [i for i in range(len(algorithms))]
    plt.title("average separator size relative to smallest known separator")
    plt.ylabel("average separator size")
    plt.xlabel("algorithm and postprocessor")
    plt.bar(xs, [np.mean(results[alg]) for alg in algorithms], tick_label=algorithms, width=0.6, color=colors)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/"+name+".png")
    plt.show()


def analyze_runtime(df, algorithms):
    instances = df['instance'].unique()

    results = dict()
    for alg in algorithms:
        results[alg] = []

    # df['rel_sep_size'] = df.apply (lambda row: func(row), axis=1)

    for instance in instances:
        inst_df = df[df['instance'] == instance]
        mini = max(inst_df['time'].min(), 0.5)

        assert(mini > 0)

        def relative(row):
            return row['time'] / mini

        inst_df['rel_time'] = inst_df.apply(lambda row: relative(row), axis=1)

        print(inst_df)

        for alg in algorithms:
            results[alg].append(inst_df[inst_df['algorithm'] == alg]['rel_time'].values[0])

        print(results)

    colors = [cmap[alg] for alg in algorithms]

    plt.figure()
    xs = [i for i in range(len(algorithms))]
    plt.title("average runtime relative to fastest algorithm")
    plt.ylabel("average runtime")
    plt.xlabel("algorithm")
    plt.bar(xs, [np.mean(results[alg]) for alg in algorithms], tick_label=algorithms, width=0.6, color=colors)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/runtime.png")
    plt.show()

def analyze_runtime_development(dataframe):
    algorithms = ['LT', 'LTFC', 'Dual', 'DualFC', 'HP']

    def get_runtimes(df, algorithm):
        runtimes = []
        averages = []
        df_alg = df[df['algorithm'] == algorithm]
        df_alg = df_alg.sort_values('nodes')
        # print(df_alg)
        instances = [inst for inst in df_alg['instance'].unique() if 'random' in inst]

        for inst in instances:
            runtimes.append(df_alg.loc[df_alg['instance'] == inst, 'time'].values[0] / 1000000)

        # print(runtimes)

        for i in range(0, len(runtimes), 3):
            averages.append(np.mean([runtimes[i], runtimes[i+1], runtimes[i+2]]))

        return averages

    sizes = dataframe[dataframe['instance'].str.contains('random')].sort_values('nodes')['nodes'].unique()
    print("sizes:", sizes)
    plt.figure()
    for alg in algorithms:
        plt.plot(sizes, get_runtimes(dataframe, alg), color=cmap[alg], label=alg)
    plt.title("runtime development of core algorithms")
    plt.xlabel("instance size (nodes)")
    plt.ylabel("runtime (s)")
    plt.legend()
    plt.tight_layout()
    plt.savefig("../results/plots/runtime_dev.png")
    plt.show()


def analyze_individual_instance(df, name):

    df = df[df['instance'] == name]
    algorithms = df['algorithm'].unique()

    diam = not ('diameter' in name)  # plot diameter bounds only for sensible instances

    print(df)

    data = [df.loc[df['algorithm'] == alg, 'mean_sep_size'].values[0] for alg in algorithms] # average sep size
    minis = [df.loc[df['algorithm'] == alg, 'min_sep_size'].values[0] for alg in algorithms]
    diam_lb = df['diam_lB'].iloc[0]
    diam_ub = df['diam_uB'].iloc[0]
    n = df['nodes'].iloc[0]

    colors = [cmap[alg] for alg in algorithms]

    plt.figure()
    xs = [i for i in range(len(algorithms))]
    plt.title("separator size for " + name + " (" + str(n) + " nodes)")
    plt.ylabel("average separator size (nodes)")
    plt.xlabel("algorithm and postprocessor")
    print("results: ", [df.loc[df['algorithm'] == alg, 'mean_sep_size'].values[0] for alg in algorithms])
    if diam:
        plt.axhline(2*diam_lb+1, c='green')
        plt.axhline(2*diam_ub+1, c='red')
    plt.bar(xs, data, tick_label=algorithms, width=0.6, color=colors, zorder=0)
    plt.scatter(xs, minis, c='white', marker='_', zorder=10)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("../results/plots/instances/"+name+".png")
    plt.close()
    # plt.show()


def main(path):
    """
    Calls the different analysis methods.

    :param path: path to csv-file generated by experiments
    """

    # read csv file
    df = pd.read_csv(path, sep=r'\s*,\s*', encoding='utf-8')
    # df = df[df['nodes'] < 100000]

    #
    analyze_separator_size(df, "sep_size")
    # #
    algorithms = ['LT', 'LTFC', 'Dual', 'DualFC', 'HP']
    analyze_runtime(df, algorithms)
    # # #
    analyze_runtime_development(df)

    # df_berlin = df[df['instance'] == 'Berlin']
    # analyze_separator_size(df_berlin, "berlin")

    for inst in df['instance'].unique():
        analyze_individual_instance(df, inst)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Data analysis and plotting.')
    parser.add_argument('--path', type=str, help='Path to data file')
    args = parser.parse_args()

    main(args.path)
