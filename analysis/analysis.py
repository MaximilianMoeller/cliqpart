#!/usr/bin/env python3

import os
from pathlib import Path
import csv
import argparse
from datetime import datetime, timedelta
import math

import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

from helper import parse_time, parse_dict, highlight_values, order_and_label_runConfigs, instance_optimal_info

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
        

def single_instance_analysis(instance_name, rc_list, update_csv):
    order_and_label_runConfigs(rc_list)
    raw_data, formatted_data = [], []

    ### first row ###
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
        # if the procedure terminatede because no constraint was found in the last iteration, exclude that iteration from the min
        min_cuts_helper = list(filter(lambda x: x!=0, [it[-1][1] for it in seps_by_it]))
        min_cuts.append(min(min_cuts_helper) if min_cuts_helper else 0)
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
    opt_info = instance_optimal_info(instance_name)
    opt_value, opt_scaling = opt_info["value"], opt_info["scaling"]
    objectives = [rc['last_objective'] / opt_scaling for rc in rc_list]
    no_gap = opt_value == None
    if no_gap:
        opt_value = math.inf
    gap_calc = lambda obj: abs(obj - (opt_value / opt_scaling)) / abs(opt_value / opt_scaling) if opt_value != 0 else abs(obj)
    gaps = list(map(gap_calc, objectives))

    raw_data.append(objectives.copy())

    if no_gap:
        raw_data.append([])
    else:
        raw_data.append(gaps.copy())

    integrals = [rc["termination"]["integral"] for rc in rc_list]
    if no_gap:
        gaps = ["—"] * len(objectives)
        mark_indices = [obj == min(objectives) if opt_info["maximizing"] else obj == max(objectives) for obj in objectives]
    else:
        best_gap = min(gaps)
        mark_indices = [abs(gap - best_gap) < 1e-9 for gap in gaps]

        for i in range(len(gaps)):
            (integral, mark) = (integrals[i], mark_indices[i])
            if integral:
                gaps[i] = f"$\\bm{{{gaps[i]:.1%}}}^{{*}}$"
            elif mark:
                gaps[i] = f"$\\bm{{{gaps[i]:.1%}}}$"
            else:
                gaps[i] = f"{gaps[i]:.1%}"

    objectives = list(map(lambda x: f"{x:.4g}", objectives))
    highlight_values(objectives, mark_indices)

    formatted_data.append(["objective"] + ["" for _ in range(0,19)])
    formatted_data.append(["bound"] + objectives)
    formatted_data.append(["gap"] + gaps)

    ### times, absolute, in lp, and normed ###
    # avoid division by 0
    times = [max(0.001, rc['total_time']) for rc in rc_list]
    # lp times come in milliseconds
    lp_times = [max(0.001, sum([it["lp_time"] for it in rc["iterations"]]) / 1e3) for rc in rc_list]
    normed_times = [t / times[0] for t in times]

    raw_data.append(times.copy())
    raw_data.append(normed_times.copy())
    raw_data.append(lp_times.copy())

    min_time = min(times)
    min_lp_time = min(lp_times)
    mark_indices = [time == min_time for time in times]
    mark_lp_indices = [time == min_lp_time for time in lp_times]

    times = list(map(lambda x: f"{x:.3f}", times))
    lp_times = list(map(lambda x: f"{x:.3f}", lp_times))
    normed_times = list(map(lambda x: f"{x:.2f}", normed_times))

    highlight_values(times, mark_indices)
    highlight_values(lp_times, mark_lp_indices)
    highlight_values(normed_times, mark_indices)

    formatted_data.append(["time"] + ["" for _ in range(0,19)])
    formatted_data.append(["total \\si[per-mode=symbol]{\\per\\second}"] + times)
    formatted_data.append(["normalized"] + normed_times)
    formatted_data.append(["lp time \\si[per-mode=symbol]{\\per\\second}"] + lp_times)

    if update_csv:
        with open("analysisCSVs/" + instance_name + "_analysis.csv", 'w') as output_csv:
            writer = csv.writer(output_csv, delimiter=";")
            writer.writerows(formatted_data)
    return raw_data

def plot(analysis, file_prefix):

    separator_names= [
        "\\texttt{Δ}",
        "$\\texttt{Δ}_{\\infty}$",
        "$\\texttt{Δ}^{\\leq 1}$",
        "$\\texttt{Δ}_{\\infty}^{\\leq 1}$",
        "\\texttt{Δ-st-1}",
        "\\texttt{Δ-st-2}",
        "\\texttt{Δ-st-12}",
        "\\texttt{Δ-½}",
        "\\texttt{Δ-2}",
        "\\texttt{Δ-c}",
        "\\texttt{all}"]

    #fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, 1, sharex=True, layout='constrained')
    #ax1.set_ylabel("LP Solver Time / ms")
    #ax1.plot(iterations, lp_times)
    #ax2.set_ylabel("LP Size")
    #ax2.plot(iterations, lp_sizes)
    #ax3.set_ylabel("Gap of LP relaxation")
    #ax3.plot(iterations, gaps)
    #ax4.set_ylabel("Separator Time")
    #ax4.plot(iterations, separator_times)

    # analysis rows
    # [0] [-12] iterations
    # [1] [-11] non-Δ-calls
    # [2] [-10] total cuts
    # [3] [-9] max cuts
    # [4] [-8] min cuts
    # [5] [-7] non-Δ-cuts
    # [6] [-6] removed cuts
    # [7] [-5] bound
    # [8] [-4] gap
    # [9] [-3] total time
    # [10] [-2] normalized time
    # [11] [-1] lp time

    # maybe should be filtered for when more complicated separators are actually called?
    # -> nope, defeats point of run configuration!
    normed_times = list(zip(*[instance[-2] for instance in analysis]))
    # filter out instances for which the gap is not known
    gaps = list(zip(*filter(lambda x: len(x) > 0, [instance[-4] for instance in analysis])))

    # still contains three lines per run config containing st-separators
    (triangle_rows, st1_rows, st2_rows, st12_rows, circle_rows, all_rows) = tuple([normed_times[0:4]] + [normed_times[i:i+3] for i in range(4, len(normed_times), 3)])
    #print(*st1_rows, sep='\n')
    time_data = triangle_rows \
                + [list(map(lambda x: sum(x) / len(x), zip(*st1_rows)))] \
                + [list(map(lambda x: sum(x) / len(x), zip(*st2_rows)))] \
                + [list(map(lambda x: sum(x) / len(x), zip(*st12_rows)))] \
                + circle_rows \
                + [list(map(lambda x: sum(x) / len(x), zip(*all_rows)))]

    (triangle_rows, st1_rows, st2_rows, st12_rows, circle_rows, all_rows) = tuple([gaps[0:4]] + [gaps[i:i+3] for i in range(4, len(gaps), 3)])
    gap_data = triangle_rows \
                + [list(map(lambda x: sum(x) / len(x), zip(*st1_rows)))] \
                + [list(map(lambda x: sum(x) / len(x), zip(*st2_rows)))] \
                + [list(map(lambda x: sum(x) / len(x), zip(*st12_rows)))] \
                + circle_rows \
                + [list(map(lambda x: sum(x) / len(x), zip(*all_rows)))]
                 

    plt.rcParams.update({
    "text.usetex": True,
    "pgf.texsystem": "lualatex",
    #"axes.ymargin": 0.15,
    "figure.figsize": (6.4, 7.2),
    })

    ### plot of normalized times and linear relative gaps ###
    fig, (ax_time, ax_gap) = plt.subplots(2, 1, sharex=True, layout='constrained')
    ax_time.boxplot(time_data,
                    labels=separator_names,
                    widths=0.6,
                    medianprops=dict(color='#0069b4'),
                    positions=list(map(lambda x: 1.5*x, range(1, len(time_data) + 1))))
    ax_time.set_yscale('log')
    ax_time.yaxis.grid(True, which='major')
    ax_time.set_ylabel('Running times normalized wrt.\\ \\texttt{Δ}')

    ax_gap.boxplot(gap_data,
                   labels=separator_names,
                   widths=0.6,
                   showfliers=True,
                   medianprops=dict(color='#0069b4'),
                   positions=list(map(lambda x: 1.5*x, range(1, len(time_data) + 1))))
    ax_gap.yaxis.grid(True)
    ax_gap.yaxis.set_major_formatter(mtick.PercentFormatter(1.0))
    ax_gap.set_ylabel('Relative gaps to the best known solution')
    ax_gap.set_xlabel('Run configuration')

    plt.savefig('analysisCSVs/' + file_prefix + 'time_and_gap_bars.pgf')
    print("Saving time_and_gap_bars.pgf")

    ### separate normalized times plot ### 
    plt.rcParams.update({
    "text.usetex": True,
    "pgf.texsystem": "lualatex",
    #"axes.ymargin": 0.15,
    "figure.figsize": (6.4, 3.6)
    })

    fig, ax_time = plt.subplots(layout='constrained')
    ax_time.boxplot(time_data,
                    labels=separator_names,
                    widths=0.6,
                    medianprops=dict(color='#0069b4'),
                    positions=list(map(lambda x: 1.5*x, range(1, len(time_data) + 1))))
    ax_time.set_yscale('log')
    ax_time.yaxis.grid(True, which='major')
    ax_time.set_ylabel('Running times normalized wrt.\\ \\texttt{Δ}')

    plt.savefig('analysisCSVs/' + file_prefix + 'time_bars.pgf')
    print("Saving time_bars.pgf")

    ### separate linear relative gaps ### 
    plt.rcParams.update({
    "text.usetex": True,
    "pgf.texsystem": "lualatex"
    #"axes.ymargin": 0.15,
    #"figure.figsize": (6.4, 7.2)
    })

    fig, ax_gap = plt.subplots(layout='constrained')
    ax_gap.boxplot(gap_data,
                   labels=separator_names,
                   widths=0.6,
                   showfliers=True,
                   medianprops=dict(color='#0069b4'),
                   positions=list(map(lambda x: 1.5*x, range(1, len(time_data) + 1))))
    ax_gap.yaxis.grid(True)
    ax_gap.yaxis.set_major_formatter(mtick.PercentFormatter(1.0))
    ax_gap.set_ylabel('Relative gaps to the best known solution')
    ax_gap.set_xlabel('Run configuration')

    plt.savefig('analysisCSVs/' + file_prefix + 'lin_gap_bars.pgf')
    print("Saving lin_gap_bars.pgf")

    ### separate logarithmic relative gaps ### 
    plt.rcParams.update({
    "text.usetex": True,
    "pgf.texsystem": "lualatex"
    #"axes.ymargin": 0.15,
    #"figure.figsize": (6.4, 7.2)
    })

    fig, ax_gap = plt.subplots(layout='constrained')
    ax_gap.boxplot(gap_data,
                   labels=separator_names,
                   widths=0.6,
                   showfliers=True,
                   medianprops=dict(color='#0069b4'),
                   positions=list(map(lambda x: 1.5*x, range(1, len(time_data) + 1))))
    ax_gap.set_yscale('log')
    ax_gap.yaxis.grid(True)
    ax_gap.set_ylabel('Relative gaps to the best known solution')
    ax_gap.set_xlabel('Run configuration')

    plt.savefig('analysisCSVs/' + file_prefix + 'log_gap_bars.pgf')
    print("Saving log_gap_bars.pgf")

def main():
    parser = argparse.ArgumentParser(prog="CliqPartAnalysis",
                                     description="Analysis tool for results obtained from the cliqpart executable.")
    parser.add_argument('files', nargs='+')
    parser.add_argument('-u',
                        dest='update',
                        action='store_true',
                        help="Use this flag to update csv files.")
    parser.add_argument('-n', '--name',
                        dest='name',
                        required=True,
                        help="A prefix that plot files will get.")
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
        raw_multi_instance_data.append(single_instance_analysis(instance_name, rc_list, args.update))

    plot(raw_multi_instance_data, args.name)

if __name__ == "__main__":
    main()
