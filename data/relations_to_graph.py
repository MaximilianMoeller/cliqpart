#!/usr/bin/env python3
import csv
import argparse
import math
import os

parser = argparse.ArgumentParser(description='Converts relational data into an instance of the Clique Partitioning Problem.')

parser.add_argument("file", type=str, metavar="FILE", help="Path to the raw data.")
parser.add_argument("rel_count", type=int, metavar="REL", help="Number of relations in the raw data.")
parser.add_argument("obj_count",   type=int, metavar="OBJ", help="Number of entities in the raw data.")
parser.add_argument("header_rows",   type=int, metavar="ROW", help="Number of header rows.")
parser.add_argument("header_columns",   type=int, metavar="COL", help="Number of header columns.")

parser.add_argument("--id_row", type=int, metavar="ID", help="Which row (if any) should be used as IDs in the produced data.")

args = parser.parse_args()

output_csv_path = os.path.join(os.path.dirname(args.file), "data.csv")
output_toml_path = os.path.join(os.path.dirname(args.file), "data.toml")

with open(args.file, 'r') as raw_data, open(output_csv_path, 'w') as output_csv, open(output_toml_path, 'w') as output_toml:
    reader = csv.reader(raw_data)

    # the csv file read into one 2D array
    data = []
    for row in reader:
        data.append(row)

    # a list of ids, if --id_row was specified, else just 1..OBJ
    ids = list(list(zip(*data[args.header_rows:]))[args.id_row]) if args.id_row else list(range(1, args.obj_count + 1))
    # gurobi cannot print variables with a whitespace in their name
    if args.id_row:
        ids = list(map(lambda x: x.replace(" ", "_"), ids))
   
    # the relational part of the data extracted, i.e. a matrix with OBJ rows (first index) and REL columns (second index)
    relations = [row[args.header_columns:] for row in data[args.header_rows:]]
    # the same, but parsed into integers or math.nan, respectively
    int_relations = [list(map(lambda x: int(x) if x.isdigit() else math.nan, row)) for row in relations]
    
    # the weights of the CPP instance
    graph_data = [[0] * args.obj_count for _ in range(args.obj_count)]

    for i in range(args.obj_count):
        for j in range(i):
            w = 0
            for k in range(args.rel_count):
                if math.isnan(int_relations[i][k]) or math.isnan(int_relations[j][k]):
                    continue
                else:
                    w += -1 if int_relations[i][k] == int_relations[j][k] else 1
            graph_data[i][j] = graph_data[j][i] = w

    csv_data = [["id"] + ids]
    csv_data += [ [ids[index]] + row for (index,row) in enumerate(graph_data)]

    writer = csv.writer(output_csv)
    writer.writerows(csv_data)

    output_toml.write("graph_degree = " + str(args.obj_count)+"\n")
    output_toml.write("row_labels = 1\n")
    output_toml.write("column_labels = 1\n")

