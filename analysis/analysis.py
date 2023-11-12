#!/usr/bin/env python3

import os
import csv
import math
import argparse
from datetime import datetime, timedelta
from ast import literal_eval

def parse_time(time_string):
    return datetime.strptime(time_string, "%Y/%m/%d %H:%M:%S.%f")

def parse_dict(dict_string):
    dict_string = dict_string.replace('false', 'False')
    dict_string = dict_string.replace('true', 'True')
    return literal_eval(dict_string)

def readin_measurements(file):
    with open(file) as input_csv:
        reader = csv.reader(input_csv, delimiter=';')
        data = []
        for row in reader:
            data.append(row)
        headerless = data[1:]
        important = [[row[0] + " " + row[1], row[2], row[6]] for row in headerless]

        parsed = [[parse_time(row[0]), row[1], parse_dict(row[2])] for row in important]

        times = [row[0] for row in parsed if row[1] == "INFO"]
        infos = [row[2] for row in parsed if row[1] == "DEBUG"]
        debugs = [row[2] for row in parsed if row[1] == "DEBUG"]

        # wrong, since there can be multiple infos with the same iteration but always only one debug!
        for i in range(0, len(infos)):
            infos[i]["iteration"] -= 1
            infos[i]["time"] = times[i]
            infos[i]["removed"] = debugs[i]["constraints_removed"] if i < len(debugs) else 0

        return infos

def calc_times(measurements):
    measurements[0]['duration'] = 0
    for i in range(1, len(measurements)):
        measurements[i]['duration'] = (measurements[i]['time'] - measurements[i-1]['time']) / timedelta(milliseconds=1)
    return measurements

def print_as_table(measurements):
    print(f"It.\tValue\t\tAdded\tSep.\tRemoved\tDuration\n")
    for line in measurements:
        print(f"{line['iteration']}{'*' if line['integral'] else ''}\t{line['obj_value']}\t\t{line['violated_found']}\t{line['separator']}\t{line['removed']}\t{line['duration']}")

def output_as_csv(data, file_name):
    pass

def main():
    parser = argparse.ArgumentParser(description='Reads data from csv file.')
    parser.add_argument('file')

    args = parser.parse_args()

    measurements = readin_measurements(args.file)
    measurements = calc_times(measurements)
    print_as_table(measurements)

if __name__ == "__main__":
    main()
