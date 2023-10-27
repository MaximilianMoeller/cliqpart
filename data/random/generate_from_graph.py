#!/usr/bin/env python3
import csv
import argparse
import os
import random
import math

def generate_from_random_graph(instance, random_params):
    
    (size, num_clusters) = instance
    (pos_mean, pos_stddev, neg_mean, neg_stddev) = random_params
    
    def pos_gauss():
        return random.gauss(pos_mean, pos_stddev)
    def neg_gauss():
        return random.gauss(neg_mean, neg_stddev)
    
    
    # data creation
    cluster = [random.choice(range(0,num_clusters)) for _ in range(0,size)]
    data = [[0.0] * size for _ in range(0,size)]
    
    for i in range(1,size):
        for j in range(0,i):
            res = neg_gauss() if cluster[i] == cluster[j] else pos_gauss()
            data[i][j] = data[j][i] = res
    
    basedir = f"random_graph_s{size}_c{num_clusters}_{pos_mean},{pos_stddev}_{neg_mean},{neg_stddev}"
    
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

instances = [(25,2),(25,5),(25,10),
             (50,2),(50,5),(50,10),
             (75,2),(75,5),(75,10),
             (100,2),(100,5),(100,10),
             (150,2),(150,5),(150,10),
             (200,2),(200,5),(200,10),
             ]

random_configs = [(1,1,-1,1), # comparison
                  (0.5,1,-0.5,1), # closer together
                  (2,1,-2,1), # further apart
                  ]

num_instances = len(instances) * len(random_configs)
done = 0

for instance in instances:
    for config in random_configs:
        print(f"Progress: {math.floor(done/num_instances * 100)}%")
        generate_from_random_graph(instance, config)
        done += 1
