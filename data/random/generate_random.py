#!/usr/bin/env python3
import csv
import os
import random as rand

def generate_normal(size, mu, sigma):
    data = [[0.0] * size for _ in range(size)]
    
    for i in range(1,size):
        for j in range(0,i):
            data[i][j] = data[j][i] = rand.gauss(mu, sigma)


    basedir = f"random_normal_s{size}_{mu}_{sigma}"
    if not os.path.exists(basedir):
        os.makedirs(basedir)
    with open(basedir + '/data.csv', 'w') as file, open(basedir + '/data.toml', 'w') as descr:
        writer = csv.writer(file)
        writer.writerows(data)

        descr.write("graph_degree = " + str(size) + "\n")
        descr.write("maximizing = false\n\n")
        descr.write("value_offset = 0\n")
        descr.write("value_scaling = 1\n\n")
        descr.write("row_labels = 0\n")
        descr.write("column_labels = 0\n")

def generate_uniform(size, lb, ub):
    data = [[0.0] * size for _ in range(size)]
    
    for i in range(1,size):
        for j in range(0,i):
            data[i][j] = data[j][i] = rand.uniform(lb, ub)


    basedir = f"random_uniform_s{size}_{lb}_{ub}"
    if not os.path.exists(basedir):
        os.makedirs(basedir)
    with open(basedir + '/data.csv', 'w') as file, open(basedir + '/data.toml', 'w') as descr:
        writer = csv.writer(file)
        writer.writerows(data)

        descr.write("graph_degree = " + str(size) + "\n")
        descr.write("maximizing = false\n\n")
        descr.write("value_offset = 0\n")
        descr.write("value_scaling = 1\n\n")
        descr.write("row_labels = 0\n")
        descr.write("column_labels = 0\n")

def generate_binary(size):
    data = [[0.0] * size for _ in range(size)]

    for i in range(1,size):
        for j in range(0,i):
            data[i][j] = data[j][i] = rand.choice([-1, 1])


    basedir = f"random_binary_s{size}"
    if not os.path.exists(basedir):
        os.makedirs(basedir)
    with open(basedir + '/data.csv', 'w') as file, open(basedir + '/data.toml', 'w') as descr:
        writer = csv.writer(file)
        writer.writerows(data)

        descr.write("graph_degree = " + str(size) + "\n")
        descr.write("maximizing = false\n\n")
        descr.write("value_offset = 0\n")
        descr.write("value_scaling = 1\n\n")
        descr.write("row_labels = 0\n")
        descr.write("column_labels = 0\n")


sizes = [35, 50, 100, 150, 200, 250, 300, 350]

# thesis: the hardest ones will be the ones centered at 0 but no real difference with different standart deviations
# the ones centered at 2 should be fairly easy though
normal_configs = [(0, 1), (0, 0.5), (0, 2), # comparison
                  (0.5,1), (0.5,0.5), (0.5,2), # little positive sided
                  (2,1), (2,0.5), (2,2)] # heavily positive sided

# thesis: the hardest one will be (-1, 1), whereas symmetric versions with larger ranges will be slightly easier,
uniform_configs= [(-1, 1), (-10, 10), (-10, 100)]

for size in sizes:
    generate_binary(size)

    for config in normal_configs:
        generate_normal(size, *config)
    for config in uniform_configs:
        generate_uniform(size, *config)
