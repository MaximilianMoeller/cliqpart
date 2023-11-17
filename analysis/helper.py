from ast import literal_eval
from datetime import datetime, timedelta

def parse_time(time_string):
    return datetime.strptime(time_string, "%Y/%m/%d %H:%M:%S.%f")

def parse_dict(dict_string):
    dict_string = dict_string.replace('false', 'False')
    dict_string = dict_string.replace('true', 'True')
    return literal_eval(dict_string)


def highlight_values(table_row, mark_indices):
    for i in range(len(table_row)):
        if mark_indices[i]:
            table_row[i] = f"\\textbf{{{table_row[i]}}}"

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

            "random_normal_s25_0.5_1":      {'value': -2.6373181719659332e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_0_0.5":      {'value': -2.6136457831165082e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_0_2":        {'value': -9.5773197228380738e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_0.5_2":      {'value': -6.6233629665114762e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_0_1":        {'value': -5.2520026814023552e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_uniform_s25_-10_100":   {'value': -5.7514446246776380e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_uniform_s25_-100_100":  {'value': -2.4353329214592140e+03, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_binary_s25":            {'value': -47, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_uniform_s25_-1_1":      {'value': -2.8735260505090348e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_0.5_0.5":    {'value': -5.45983392244648, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_2_2":        {'value': -2.5893529195978708e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_2_0.5":      {'value': 0, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s25_2_1":        {'value': -2.3073216688670639e+00, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},

            "random_normal_s50_0.5_1":      {'value': -7.3323179002002320e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_0_0.5":      {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_0_2":        {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_0.5_2":      {'value': -1.9821372187934747e+02, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_0_1":        {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_uniform_s50_-10_100":   {'value': -2.0407377419357891e+02, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_uniform_s50_-100_100":  {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_binary_s50":            {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_uniform_s50_-1_1":      {'value': None, 'optmiality_proven': False, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_0.5_0.5":    {'value': -1.9014241663064709e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_2_2":        {'value': -5.8818801289591669e+01, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_2_0.5":      {'value': 0, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            "random_normal_s50_2_1":        {'value': -6.0397797864336562e+00, 'optmiality_proven': True, 'maximizing': False, 'scaling': 1},
            }

    if instance_name in optimal_values:
        return optimal_values[instance_name]
    else:
        print(f"{instance_name} could not be found in optimal_values_table!")
        exit(-1)
