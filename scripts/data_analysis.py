"""
This script analyses the experimental data.
More specifically:
    1. Plots the relative separator sizes as a bar chart, across all instances
        a) for core algorithms
        b) for simple postprocessing
        c) for combinations of postprocessing
    2. Plots the performance per instance as a scatter-plot
    3. Plots speed of core algorithms as violin- and boxplots
    4. Plots mean balance as bar-chart
    5. Plots the runtime development as line chart (0-1K nodes and 0-1M nodes)
"""
from distutils.util import strtobool
import pandas as pd
import argparse
import utils
import os
from utils import analyze_separator_size, analyze_instance_performance, analyze_separator_speed, \
    analyze_separator_balance, analyze_runtime_development


def main(path, target, post):
    """
    Calls the different analysis methods.

    :param path: path to csv-file generated by experiments
    :param target: path to folder to contain results
    :param post: whether to use best postprocessing for scatter plot or not
    """

    # read csv file
    df = pd.read_csv(path, sep=r'\s*,\s*', encoding='utf-8', engine='python')

    df = df[df['instance'] != 'table/diameter/diameter_3333']

    print(f"Analyzing instances ranging in size from {df['nodes'].min()} nodes to {df['nodes'].max()} nodes.")

    df = df.sort_values(by=['nodes'])
    instances = df['instance'].unique()

    present_algorithms = df['algorithm'].unique()
    algorithms = [alg for alg in utils.core_algorithms if alg in present_algorithms]

    # Which core algorithm yields the smallest relative separators?
    analyze_separator_size(df, "rel_sepsize_core", algorithms, instances, target)

    # Which simple postprocessor is better?
    analyze_separator_size(df, "rel_sepsize_simple_post", utils.simple_postprocessors, instances, target)

    # What about combinations of postproccesors?
    analyze_separator_size(df, "rel_sepsize_complex_post", utils.all_algs_and_post, instances, target)

    # Now, let's check the performance of all algorithms per instance in one huge plot.
    analyze_instance_performance(df, "per_instance", instances, algorithms, target, post)

    # Next, let's check runtime:
    analyze_separator_speed(df, "rel_speed_core", algorithms, instances, target)

    # Also analyse the average balance between components:
    analyze_separator_balance(df, "avg_balance", algorithms, target)
    # analyze_separator_balance(df, "avg_balance_pp", utils.dmd_ne, target)

    # Analyze runtime development
    analyze_runtime_development(df, "runtime_dev", algorithms, instances, 1000000, 'nodes', True, target)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Data analysis and plotting.')
    parser.add_argument('--source', type=str, help='Path to data file')
    parser.add_argument('--target', type=str, help='Path to folder with plots')
    parser.add_argument('--post', type=lambda x: bool(strtobool(x)), default=True,
                        help='Whether to use postprocessing for scatter plot')
    args = parser.parse_args()

    if not os.path.exists(args.target):
        os.mkdir(args.target)
    main(args.source, args.target, args.post)
