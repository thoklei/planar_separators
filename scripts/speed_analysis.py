"""
This script analyses the runtime development, to see if all algorithms are linear.
"""
import pandas as pd
import os
import argparse
import utils
from utils import analyze_separator_speed, analyze_runtime_development, filter_instances


def main(path, target):
    """
    Calls the different analysis methods.

    :param path: path to csv-file generated by experiments
    :param target: where to save the plots
    """

    # read csv file
    df = pd.read_csv(path, sep=r'\s*,\s*', encoding='utf-8')

    # How does the speed develop?
    # sorted_instances = df[df['instance'].str.contains('random')].sort_values('nodes')['instance'].unique()  # random
    sorted_instances = df.sort_values('nodes')['instance'].unique()  # all

    for measure in ['nodes', 'edges']:
        # only analyze the smaller ones up to 1000 nodes
        analyze_runtime_development(df, "runtime_development", utils.core_algorithms, sorted_instances, 1001, measure, True, target)

        # again for all instances
        analyze_runtime_development(df, "runtime_development", utils.core_algorithms, sorted_instances, 1000000, measure, True, target)

    # Har-Peled is significantly slower here, no mincing words - actually, might be reasonable to try this with larger
    # instances, just to make sure that we are not accidentally superlinear.


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Data analysis and plotting.')
    parser.add_argument('--path', type=str, help='Path to data file')
    parser.add_argument('--target', type=str, help='Path to plot folder')
    args = parser.parse_args()

    if not os.path.exists(args.target):
        os.mkdir(args.target)

    main(args.path, args.target)
