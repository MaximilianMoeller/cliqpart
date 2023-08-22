#!/usr/bin/env python3
import csv
import os
import random

def pos_gauss():
    return random.gauss(1,1)
def neg_gauss():
    return random.gauss(-1,1)

size = 250
num_clusters = 5

# data creation
cluster = [random.choice(range(0,num_clusters)) for _ in range(0,size)]
data = [[0.0] * size for _ in range(0,size)]

for i in range(1,size):
    for j in range(0,i):
           res = neg_gauss() if cluster[i] == cluster[j] else pos_gauss()
           data[i][j] = data[j][i] = res

if not os.path.exists("from_graph"):
    os.makedirs("from_graph")
with open('from_graph/data.csv', 'w') as file, open('from_graph/data.toml', 'w') as descr:
    writer = csv.writer(file)
    writer.writerows(data)

    descr.write("graph_degree = " + str(size))
