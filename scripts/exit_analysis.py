"""
This script analyses how the exit point correlates with separator size.
(Mostly to find this bug in DFC that causes huge average separator size)
"""
import pandas as pd
import numpy as np
import argparse


def show_exit_points(df, instances):

    for instance in instances:
        inst_df = df[df['instance'] == instance]

        data = inst_df['sep_size']
        exit_points = inst_df['exit']

        # map exit point to list of sizes
        results = {}
        for size, ex in zip(data, exit_points):
            if ex in results:
                results[ex].append(size)
            else:
                results[ex] = [size]

        print(f"=== {instance} ===")
        for ex in results:
            print(f"Average size for exit point {ex}: min: {np.min(results[ex])}, mean: {np.mean(results[ex])}")

        exit_map = {val: i for i, val in enumerate(inst_df['exit'].unique())}
        col = [exit_map[ex] for ex in exit_points]


def main(path):
    """
    Calls the different analysis methods.

    :param path: path to csv-file generated by experiments
    """

    # read csv file
    df = pd.read_csv(path, sep=r'\s*,\s*', encoding='utf-8', engine='python')

    # select algorithm
    df = df[df['algorithm'] == "DualFC_NE"]

    # select instances
    instances = ['table/grid/grid_100', 'table/rect/rect_500_20', 'table/sixgrid/sixgrid_237_20']#df['instance'].unique()
    show_exit_points(df, instances)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Data analysis and plotting.')
    parser.add_argument('--path', type=str, help='Path to data file')
    args = parser.parse_args()

    main(args.path)

