"""
This script just aggregates the results of a run, so that I can look at absolute result values.
"""
import pandas as pd
import argparse


def main(source, target):
    """
    Calls the different analysis methods.

    :param source: path to csv-file generated by experiments
    :param target: path to csv-file with results
    """

    # read csv file
    df = pd.read_csv(source, sep=r'\s*,\s*', encoding='utf-8', engine='python')

    instances = df['instance'].unique()
    algorithms = df['algorithm'].unique()

    res_df = pd.DataFrame(columns=['Instance'] + list(algorithms))

    for instance in instances:

        idff = df[df['instance'] == instance]

        line = [instance]

        for algo in algorithms:

            adff = idff[idff['algorithm'] == algo]
            # print(f"Standard Deviation for instance {instance} and algo {algo}: {adff['sep_size'].std()}")

            mean = adff['sep_size'].mean()
            line.append(mean)

        res_df.loc[len(res_df)] = line

    print(res_df.head())
    res_df.to_csv(target)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Data analysis and plotting.')
    parser.add_argument('--source', type=str, help='Path to data file')
    parser.add_argument('--target', type=str, help='Path to data file')
    args = parser.parse_args()

    main(args.source, args.target)
