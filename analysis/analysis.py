#!/usr/bin/env python3

import os
from pathlib import Path
import csv
import argparse
from datetime import datetime, timedelta
from ast import literal_eval

import matplotlib.pyplot as plt

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

        times = [parse_time(row[0] + " " + row[1]) for row in headerless]
        messages = [parse_dict(row[6]) for row in headerless]


#       Format of the analysis dict
#        {
#            # in seconds
#            "total_time" = 0.5,
#            # every iteration gets a dictionary with the important features
#            "iterations" = [
#                {
#                    # in ms
#                    "LP_time" : 10,
#                    "obj_value": 110293,
#                    # list of tuples of the form (separator_abbreviation, violated_found, time in ms)
#                    "separators": [("Δ", 0), ("ST", 14)],
#                    "removed": 130,
#                    "integral": False}
#            ]
#        }

        path_parts = Path(file).parts
        print(path_parts)
        data_name = path_parts[-4]
        numDir = path_parts[-2]
        runConfig = path_parts[-3].split("-2023")[0]

        analysis = {
                "data_name": data_name,
                "run_config": runConfig,
                "run_number": numDir,
                "total_time": (times[-1] - times[0]) / timedelta(seconds=1),
                "iterations": []
                }

        iteration_dict = {}
        for (index, message) in enumerate(messages):
            if message['message'] == "LP_SOLVED":
                iteration_dict = {
                        "iteration": message["iteration"],
                        "obj_value": message["obj_value"],
                        "lp_time": (times[index] - times[index - 1]) / timedelta(milliseconds=1) if index > 0 else 0,
                        "lp_size": (analysis["iterations"][-1]["lp_size"] + analysis["iterations"][-1]["separators"][-1][1] - analysis["iterations"][-1]["removed"]) if len(analysis["iterations"]) > 0 else 0,
                        "separators": [],
                        "removed": 0,
                        "termination": {}
                        }

            elif message["message"] == "SEPARATOR":
                separator = message["separator"]
                violated = message["violated_found"]
                separation_time = (times[index] - times[index - 1]) / timedelta(milliseconds=1) if index > 0 else 0

                iteration_dict["separators"].append((separator, violated, separation_time))

            elif message["message"] == "CONSTRAINTS_REMOVED":
                iteration_dict["removed"] = message["removed"]
                analysis["iterations"].append(iteration_dict)

            elif message["message"] == "TERMINATION":
                iteration_dict["termination"] = message["cause"]
                analysis["iterations"].append(iteration_dict)

        analysis["last_objective"] = analysis["iterations"][-1]["obj_value"]
        return analysis

def plot(analysis):

    ilp_sol = 0.5498994699576757 * 1e3

    iterations = [it["iteration"] for it in analysis["iterations"]]
    lp_times = [it["lp_time"] for it in analysis["iterations"]]
    lp_sizes = [it["lp_size"] for it in analysis["iterations"]]
    objectives = [it["obj_value"] for it in analysis["iterations"]]
    gaps = [(abs(lp_sol - ilp_sol) / abs(lp_sol)) for lp_sol in objectives]
    separator_times = [sum([sep[2] for sep in it["separators"]]) for it in analysis["iterations"]]

    font = {'family': 'DejaVu Sans',
        'weight': 'medium',
        'size': 15}

    fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, 1, sharex=True, layout='constrained')

    ax1.set_ylabel("LP Solver Time / ms")
    ax1.plot(iterations, lp_times)
    ax2.set_ylabel("LP Size")
    ax2.plot(iterations, lp_sizes)
    ax3.set_ylabel("Gap of LP relaxation")
    ax3.plot(iterations, gaps)
    ax4.set_ylabel("Separator Time")
    ax4.plot(iterations, separator_times)

    plt.show()

def create_csvs(data_name, alist, ilp_solution):
    data = []
    data.append([f"\\texttt{{{data_name}}}"] + [f"{a['name']}" for a in alist])
    data.append(["iterations"] + [a["iterations"][-1]["iteration"] for a in alist])

    with open(data_name + "_analysis.csv", 'w') as output_csv:
        writer = csv.writer(output_csv)
        writer.writerows(data)

def sort_and_rename(alist):
    ord_sub = {
            "Δ-1": [0,"\\texttt{Δ}"],
            "Δ_no-maxcut-1": [1,"$\\texttt{Δ}_{\\infty}$"],
            "Δ_var-once-1": [2,"$\\texttt{Δ}^{\\leq 1}$"],
            "Δ_no-maxcut_var-once-1": [3,"$\\texttt{Δ}_{\\infty}^{\\leq 1}$"],
            "Δ_st1-1": [4,"1"],
            "Δ_st1-2": [5,"2"],
            "Δ_st1-3": [6,"3"],
            "Δ_st2-1": [7,"1"],
            "Δ_st2-2": [8,"2"],
            "Δ_st2-3": [9,"3"],
            "Δ_st12-1": [10,"1"],
            "Δ_st12-2": [11,"2"],
            "Δ_st12-3": [12,"3"],
            "Δ_half-1": [13,"\\texttt{Δ-half}"],
            "Δ_two-1": [14,"\\texttt{Δ-two}"],
            "Δ_circles-1": [15,"\\texttt{Δ-cycles}"],
            "all-1": [16,"1"],
            "all-2": [17,"2"],
            "all-3": [18,"3"],
            }
    alist.sort(key=lambda a: ord_sub[f"{a['run_config']}-{a['run_number']}"][0])
    for a in alist:
        a["name"] = ord_sub[f"{a['run_config']}-{a['run_number']}"][1]


def main():
    parser = argparse.ArgumentParser(description="Run on all 'measurements.csv' for one data set to create analysis.csv.")
    parser.add_argument('files', nargs='+')
    args = parser.parse_args()

    path_parts = Path(args.files[0]).parts
    data_name = path_parts[-4]

    alist = []
    for file in args.files:
        a = readin_measurements(file)
        alist.append(a)

    sort_and_rename(alist)
    create_csvs(data_name, alist, 0.5498994699576757 * 1e3)

if __name__ == "__main__":
    main()
