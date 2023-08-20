#!/usr/bin/env python3
import csv
import os
import random as rand

mu = 0
sigma = 1

lb = -10
ub = 10

size = 80

# data creation
data_u = [[0.0] * size for _ in range(0,size)]
data_g = [[0.0] * size for _ in range(0,size)]

for i in range(1,size):
    for j in range(0,i):
        data_u[i][j] = rand.uniform(lb, ub)
        data_u[j][i] = data_u[i][j]

        data_g[i][j] = rand.gauss(mu, sigma)
        data_g[j][i] = data_g[i][j]

if not os.path.exists("uniform"):
    os.makedirs("uniform")
if not os.path.exists("gauss"):
    os.makedirs("gauss")
with open('uniform/data.csv', 'w') as u_file, open('uniform/data.toml', 'w') as u_descr:
    u_writer = csv.writer(u_file)
    u_writer.writerows(data_u)

    u_descr.write("graph_degree = " + str(size))


with open('gauss/data.csv', 'w') as g_file, open('gauss/data.toml', 'w') as g_descr:
    g_writer = csv.writer(g_file)
    g_writer.writerows(data_g)

    g_descr.write("graph_degree = " + str(size))
