#!/usr/bin/env python3

import os
import csv
import math
import argparse

parser = argparse.ArgumentParser(description='Converts modularity clustering data and an objective value to the corresponding obejctive value of the equivalent CPP instance.')

parser.add_argument('instance', type=str)
parser.add_argument('obj_val', type=float)

parser.add_argument('-o', '--other', action='store_true', help="use this to convert multicut obj_value to cpp obj_values. Enter negative numbers like ' -1.5e5' (notice the whitespace)")

args = parser.parse_args()

obj_val = float(args.obj_val)

print(f"Obj_val offset: {obj_val}.")

data_file = "raw_data/" + args.instance + ".csv"
with open(data_file, 'r') as input_csv:
    reader = csv.reader(input_csv)
    data = []
    for row in reader:
        data.append(row)
    processed = [[int(l[0]), int(l[1]), float(l[2])] for l in data]

    obj_sum = 0
    for row in processed:
        obj_sum += row[2]
    obj_sum *=1e3
    print(f"Obj_sum: {obj_sum}")

    if(args.other):
        print(f"CPP value for instance {args.instance} and Multicut obj_value {obj_val} would be {obj_sum - obj_val}.")
    else:
        print(f"Multicut value for instance {args.instance} and CPP obj_value {obj_val} would be {obj_sum - obj_val}.")
