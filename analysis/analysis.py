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

        path_parts = Path(file).parts
        print(path_parts)
        instance_name = path_parts[-4]
        numDir = path_parts[-2]
        runConfig = path_parts[-3].split("-2023")[0]

        analysis = {
                "instance_name": instance_name,
                "run_config": runConfig,
                "run_number": numDir,
                "iterations": [],

                "total_time": (times[-1] - times[0]) / timedelta(seconds=1),
                "termination": {},
                "last_objective": None,
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
                analysis["termination"] = message["cause"]
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
            "cetacea" : {'value': -967, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "wild_cats" : {'value': -1304, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},

            # constructed
            "violated_half" : {'value': -4, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "violated_2" : {'value': -3, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},

            # modularity clustering
            "football" : {'value': 6.1332494165298363e+02, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "adjnoun" : {'value': 314.2048442906573, 'optmiality_proven': False, 'maximizing': True, 'scaling': 1e+03},
            "polbooks" : {'value': 5.4076747857117175e+02, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "lesmis" : {'value': 5.8373891747783489e+02, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "dolphins" : {'value': 5.4991891143546547e+02, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "karate" : {'value': 4.6959237343852726e+02, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},

            # organoids
            "organoid_40_soft" : {'value': 2.5080850000000023e+03, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_40_medium" : {'value': 4.9381149999999934e+03, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_40_hard" : {'value': 1.4510972999999998e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_80_soft" : {'value': 1.0279305000000011e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_80_medium" : {'value': 1.9974411999999982e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_80_hard" : {'value': 5.5988517999999989e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_100_soft" : {'value': 1.2815680000000011e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_100_medium" : {'value': 2.5669648999999969e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_100_hard" : {'value': 7.5138894000000044e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_160_soft" : {'value': 2.4628807000000023e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_160_medium" : {'value': 4.7814863999999958e+04, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            "organoid_160_hard" : {'value': 1.3533561700000061e+05, 'optmiality_proven': True, 'maximizing': True, 'scaling': 1e+03},
            }

    if instance_name in optimal_values:
        return optimal_values[instance_name]
    else:
        print(f"{instance_name} could not be found in optimal_values_table!")
        exit(-1)


def highlight_values(table_row, mark_indices):
    for i in range(len(table_row)):
        if mark_indices[i]:
            table_row[i] = f"\\textbf{{{table_row[i]}}}"
        

def single_instance_analysis(instance_name, rc_list):
    order_and_label_runConfigs(rc_list)
    opt_info = instance_optimal_info(instance_name)
    raw_data, formatted_data = [], []
    ### first row ##
    separator_names = [f"{rc['label']}" for rc in rc_list]
    formatted_data.append([""] + separator_names)
    
    ### iterations ###
    # extract
    iterations = [rc["iterations"][-1]["iteration"] for rc in rc_list]
    raw_data.append(iterations.copy())
    # find best
    mark_indices = [it == min(iterations) for it in iterations]
    # format
    iterations = list(map(lambda x: f"{{{x:,}}}", iterations))
    # highlight
    highlight_values(iterations, mark_indices)
    # add
    formatted_data.append(["# it."] + iterations)

    ### calls to non-Δ-separators ###
    calls = [sum([len(iteration["separators"]) - 1 for iteration in rc["iterations"]]) for rc in rc_list]
    raw_data.append(calls.copy())
    formatted_data.append(["# $\\centernot{\\texttt{Δ}}$-calls"] + calls)

    ### cutting planes ###
    # separators: [(run_configs) [(iterations) [(separators) (abbreviation, violated, time)]]]
    separators = [[iteration["separators"] for iteration in rc["iterations"]] for rc in rc_list]
    # extract
    total_cuts, max_cuts, min_cuts, non_triangle_cuts = [], [], [], []
    for rc in rc_list:
        seps_by_it = [iteration["separators"] for iteration in rc["iterations"]]
        total_cuts.append(sum([it[-1][1] for it in seps_by_it]))
        max_cuts.append(max([it[-1][1] for it in seps_by_it]))
        min_cuts.append(min(filter(lambda x: x!=0, [it[-1][1] for it in seps_by_it])))
        non_triangle_cuts.append(sum([it[-1][1] if it[-1][0] != "Δ" else 0 for it in seps_by_it]))

    total_removed = [sum([it["removed"] for it in rc["iterations"]]) for rc in rc_list]

    raw_data.append(total_cuts.copy())
    raw_data.append(max_cuts.copy())
    raw_data.append(min_cuts.copy())
    raw_data.append(non_triangle_cuts.copy())
    raw_data.append(total_removed.copy())

    # find best
    mark_added = [cuts == min(total_cuts) for cuts in total_cuts]
    # format
    total_cuts = list(map(lambda x: f"{{{x:,}}}", total_cuts))
    max_cuts = list(map(lambda x: f"{{{x:,}}}", max_cuts))
    min_cuts = list(map(lambda x: f"{{{x:,}}}", min_cuts))
    non_triangle_cuts = list(map(lambda x: f"{{{x:,}}}", non_triangle_cuts))
    total_removed = list(map(lambda x: f"{{{x:,}}}", total_removed))
    # highlight
    highlight_values(total_cuts, mark_added)
    # add
    formatted_data.append(["cuts"] + ["" for _ in range(0,19)])
    formatted_data.append(["# total"] + total_cuts)
    formatted_data.append(["# max"] + max_cuts)
    formatted_data.append(["# min"] + min_cuts)
    formatted_data.append(["# $\\centernot{\\texttt{Δ}}$-cuts"] + non_triangle_cuts)
    formatted_data.append(["# removed"] + total_removed)

    ### objectives, absolute and gaps ###
    objectives = [rc['last_objective'] / opt_info["scaling"] for rc in rc_list]
    gaps = list(map(lambda x: abs(x - (opt_info["value"] / opt_info["scaling"])) / abs(opt_info["value"] / opt_info["scaling"]), objectives))

    raw_data.append(objectives.copy())
    raw_data.append(gaps.copy())

    integrals = [rc["termination"]["integral"] for rc in rc_list]

    best_gap = min(gaps)
    mark_indices = [abs(gap - best_gap) < 1e-9 for gap in gaps]

    objectives = list(map(lambda x: f"{x:.4g}", objectives))

    highlight_values(objectives, mark_indices)
    for i in range(len(gaps)):
        (integral, mark) = (integrals[i], mark_indices[i])
        if integral:
            gaps[i] = f"$\\bm{{{gaps[i]:.1%}}}^{{*}}$"
        elif mark:
            gaps[i] = f"$\\bm{{{gaps[i]:.1%}}}$"
        else:
            gaps[i] = f"{gaps[i]:.1%}"

    formatted_data.append(["objective"] + ["" for _ in range(0,19)])
    formatted_data.append(["bound"] + objectives)
    formatted_data.append(["gap"] + gaps)

    ### times, absolute and normed ###
    # avoid division by 0
    times = [max(0.001, rc['total_time']) for rc in rc_list]
    rel_times = [t / times[0] for t in times]

    raw_data.append(times.copy())
    raw_data.append(rel_times.copy())

    min_time = min(times)
    mark_indices = [time == min_time for time in times]

    times = list(map(lambda x: f"{x:.3f}", times))
    rel_times = list(map(lambda x: f"{x:.2f}", rel_times))

    highlight_values(times, mark_indices)
    highlight_values(rel_times, mark_indices)

    formatted_data.append(["time"] + ["" for _ in range(0,19)])
    formatted_data.append(["seconds"] + times)
    formatted_data.append(["relative"] + rel_times)

    with open("analysisCSVs/" + instance_name + "_analysis.csv", 'w') as output_csv:
        writer = csv.writer(output_csv, delimiter=";")
        writer.writerows(formatted_data)
    return raw_data

def plot(analysis):

    separator_names = ["Δ", "Δ_∞", "Δ^<=1", "Δ_∞^<=1", 1, 2, 3, 1, 2, 3, 1, 2, 3, "Δ-half", "Δ-2", "Δ-c", 1, 2, 3]
    font = {'family': 'DejaVu Sans',
        'weight': 'medium',
        'size': 15}

    #fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, 1, sharex=True, layout='constrained')

    #ax1.set_ylabel("LP Solver Time / ms")
    #ax1.plot(iterations, lp_times)
    #ax2.set_ylabel("LP Size")
    #ax2.plot(iterations, lp_sizes)
    #ax3.set_ylabel("Gap of LP relaxation")
    #ax3.plot(iterations, gaps)
    #ax4.set_ylabel("Separator Time")
    #ax4.plot(iterations, separator_times)

    # maybe should be filtered for when more complicated separators are actually called?
    # -> nope, defeats point of run configuration!
    rel_times = list(zip(*[instance[-1] for instance in analysis]))[1:]

    fig, axs = plt.subplots()
    axs.boxplot(rel_times)
    axs.set_yscale('log')
    axs.set_xticklabels(separator_names[1:], rotation=45)

    plt.show()

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

    instances = {}
    for file in args.files:
        instance_name = Path(file).parts[-4]
        if instance_name not in instances:
            instances[instance_name] = []

        measurement = readin_measurements(file)
        instances[instance_name].append(measurement)

    raw_multi_instance_data = []
    for (instance_name, rc_list) in instances.items():
        raw_multi_instance_data.append(single_instance_analysis(instance_name, rc_list))

    plot(raw_multi_instance_data)

if __name__ == "__main__":
    main()
