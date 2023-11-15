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
        instance_name = path_parts[-4]
        numDir = path_parts[-2]
        runConfig = path_parts[-3].split("-2023")[0]

        analysis = {
                "instance_name": instance_name,
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

def order_and_label_runConfigs(rc_list):
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
            "Δ_half-1": [13,"\\texttt{Δ-½}"],
            "Δ_two-1": [14,"\\texttt{Δ-2}"],
            "Δ_circles-1": [15,"\\texttt{Δ-c}"],
            "all-1": [16,"1"],
            "all-2": [17,"2"],
            "all-3": [18,"3"],
            }
    rc_list.sort(key=lambda a: ord_sub[f"{a['run_config']}-{a['run_number']}"][0])
    for a in rc_list:
        a["label"] = ord_sub[f"{a['run_config']}-{a['run_number']}"][1]

def instance_optimal_info(instance_name):
    optimal_values = {
            # grötschel wakabayashi
            "cetacea" : {'value': -967, 'optmiality_proven': True, 'maximizing': False},
            "wild_cats" : {'value': -1304, 'optmiality_proven': True, 'maximizing': False},

            # constructed
            "violated_half" : {'value': -4, 'optmiality_proven': True, 'maximizing': False},
            "violated_2" : {'value': -3, 'optmiality_proven': True, 'maximizing': False},

            # modularity clustering
            "football" : {'value': 6.1332494165298363e+02, 'optmiality_proven': True, 'maximizing': True},
            "adjnoun" : {'value': 314.2048442906573, 'optmiality_proven': False, 'maximizing': True},
            "polbooks" : {'value': 5.4076747857117175e+02, 'optmiality_proven': True, 'maximizing': True},
            "lesmis" : {'value': 5.8373891747783489e+02, 'optmiality_proven': True, 'maximizing': True},
            "dolphins" : {'value': 5.4991891143546547e+02, 'optmiality_proven': True, 'maximizing': True},
            "karate" : {'value': 4.6959237343852726e+02, 'optmiality_proven': True, 'maximizing': True},

            # organoids
            "organoid_40_soft" : {'value': 2.5080850000000023e+03, 'optmiality_proven': True, 'maximizing': True},
            "organoid_40_medium" : {'value': 4.9381149999999934e+03, 'optmiality_proven': True, 'maximizing': True},
            "organoid_40_hard" : {'value': 1.4510972999999998e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_80_soft" : {'value': 1.0279305000000011e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_80_medium" : {'value': 1.9974411999999982e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_80_hard" : {'value': 5.5988517999999989e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_100_soft" : {'value': 1.2815680000000011e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_100_medium" : {'value': 2.5669648999999969e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_100_hard" : {'value': 7.5138894000000044e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_160_soft" : {'value': 2.4628807000000023e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_160_medium" : {'value': 4.7814863999999958e+04, 'optmiality_proven': True, 'maximizing': True},
            "organoid_160_hard" : {'value': 1.3533561700000061e+05, 'optmiality_proven': True, 'maximizing': True},
            }

    if instance_name in optimal_values:
        return optimal_values[instance_name]
    else:
        print(f"{instance_name} could not be found in optimal_values_table!")
        exit(-1)

def plot(analysis):

    # todo get from instance_optimal_info()
    ilp_sol = 0

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

def highlight_value(table_row, value):
    for i in range(len(table_row)):
        if table_row[i] == value:
            table_row[i] = f"\\textbf{{{table_row[i]}}}"
        

def single_instance_analysis(instance_name, rc_list):
    order_and_label_runConfigs(rc_list)
    opt_info = instance_optimal_info(instance_name)
    data = []
    # first row
    data.append([""] + [f"{rc['label']}" for rc in rc_list])
    
    iterations = ["# it."] + [rc["iterations"][-1]["iteration"] for rc in rc_list]
    highlight_value(iterations, min(iterations[1:]))
    data.append(iterations)

    # norm to best times -> fixed width in table
    times = ["time"] + [float(f"{rc['total_time']:.2f}") for rc in rc_list]
    highlight_value(times, min(times[1:]))
    data.append(times)

    # norm to best objective (calc gap) -> fixed width in table
    objectives = ["objective"] + [rc['last_objective'] for rc in rc_list]
    best_relaxation = min(objectives[1:]) if opt_info["maximizing"] else max(objectives[1:])

    gaps = list(map(lambda x: abs(x - opt_info["value"]) / abs(x), objectives[1:]))
    gaps = ["gap"] + list(map(lambda x: float(f"{x:.2f}"),gaps))

    highlight_value(gaps, min(gaps[1:]))
    data.append(gaps)

    with open(instance_name + "_analysis.csv", 'w') as output_csv:
        writer = csv.writer(output_csv)
        writer.writerows(data)


def main():
    parser = argparse.ArgumentParser(prog="CliqPartAnalysis",
                                     description="Analysis tool for results obtained from the cliqpart executable.")
    parser.add_argument('-m',
                        '--multi-instances',
                        dest='multiParse',
                        action='store_true',
                        help="Use this flag if you supply measurements for more than one instance.")
    parser.add_argument('files', nargs='+')
    args = parser.parse_args()

    if not args.multiParse:
        path_parts = Path(args.files[0]).parts
        instance_name = path_parts[-4]

        rc_list = []
        for file in args.files:
            a = readin_measurements(file)
            rc_list.append(a)

        single_instance_analysis(instance_name, rc_list)

if __name__ == "__main__":
    main()
