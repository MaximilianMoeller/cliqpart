#!/usr/bin/env python3

import os
import csv
import math
import argparse

parser = argparse.ArgumentParser(description='Converts modularity clustering data into an instance of the Clique Partitioning Problem.')

args = parser.parse_args()

raw_data_dir = "raw_data/"

for raw_file_name in os.listdir(raw_data_dir):
    raw_file_path = raw_data_dir + raw_file_name
    instance_name = os.path.splitext(raw_file_name)[0]
    
    if not os.path.exists(instance_name + "/"):
        os.makedirs(instance_name + "/")

    output_csv_path = os.path.join(instance_name, "data.csv")
    output_toml_path = os.path.join(instance_name, "data.toml")

    with open(raw_file_path, 'r') as input_csv, open(output_csv_path, 'w') as output_csv, open(output_toml_path, 'w') as output_toml:

        reader = csv.reader(input_csv)

        data = []
        for row in reader:
            data.append(row)
        processed = [[int(l[0]), int(l[1]), float(l[2])] for l in data]
        max_index = max(max(processed, key=(lambda l: max(l[0:2])))[0:2])

        csv_data = [[0.0] * (max_index + 1) for _ in range(max_index +1)]

        for [start, end, weight] in processed:
            csv_data[start][end] = csv_data[end][start] = weight

        writer = csv.writer(output_csv)
        writer.writerows(csv_data)

        output_toml.write("graph_degree = " + str(max_index + 1)+"\n")
        output_toml.write("maximizing = true\n\n")
        output_toml.write("row_labels = 0\n")
        output_toml.write("column_labels = 0\n")

