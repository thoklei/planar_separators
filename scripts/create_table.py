"""
This script analyses the experimental data by recreating table 1 from holzer et al.
"""
import pandas as pd
import argparse
from utils import create_table


def main(path):
    """
    Creates the table.
    """

    # read csv file
    df = pd.read_csv(path, sep=r'\s*,\s*', encoding='utf-8', engine='python')

    # create table
    base_algorithms = ["Dual", "DualFC", "HPN", "LTFC"]
    algorithms = [str(algo) + "_NE_DMD" for algo in base_algorithms]
    table_tex = create_table(df, algorithms)

    # just print resulting LaTex to console
    print(table_tex)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Creation of overview table.')
    parser.add_argument('--path', type=str, help='Path to data file')
    args = parser.parse_args()

    main(args.path)
