#!/usr/bin/env python3

from enum import StrEnum, auto
from math import inf as inf
import random
import itertools

threshold = 1e-6

class UV(StrEnum):
    u = auto()
    v = auto()

class H_Graph:
    
    t_node = tuple[UV, bool, int, int]
    t_arc = tuple[t_node, t_node]

    def __init__(self):
        self.arcs = {}
        self.dist = {}
        self.prev = {}


    def fill_from_weights(self, degree, weights):
        self.arcs = {}

        for i in range(degree):
            for j in range(degree):

                if i == j:
                    continue

                # for every edge {i,j} in G, add the gadget
                self.add_arc(
                        ((UV.u, False, j, i),
                        (UV.u, True, j, i)), - weights[(i,j)])
                self.add_arc(
                        ((UV.v, False, j, i),
                        (UV.v, True, j, i)), - weights[(i,j)])
                
                # for every adjacent pair of edges {i,j}, {j,k}
                for k in range(degree):
                    if k == i or k == j:
                        continue
                    self.add_arc(
                            ((UV.u, True, i, j),
                            (UV.v, False, j, k)), weights[(i,k)] + 0.5)
                    self.add_arc(
                            ((UV.v, True, j, k),
                            (UV.u, False, i, j)), weights[(i,k)] + 0.5)


    def add_arc(self, arc: t_arc, weight: int):
        (start, target) = arc
        (s_uv, s_n, s_i, s_j), (t_uv, t_n, t_i, t_j) = (start, target)

        if s_i == s_j or t_i == t_j:
            return
        elif s_i > s_j:
            self.add_arc(((s_uv, s_n, s_j, s_i), target), weight)
            return
        elif t_i > t_j:
            self.add_arc((start, (t_uv, t_n, t_j, t_i)), weight)
            return

        if not start in self.arcs:
            self.arcs[start] = {}

        if not target in self.arcs[start]:
            self.arcs[start][target] = weight
    
    def floyd_warshall(self):
        self.dist = {key: {key: 0} for key in self.arcs}
        self.prev = {key: {key: key} for key in self.arcs}

        for k1 in self.arcs:
            for k2 in self.arcs:
                if k1 == k2:
                    continue
                if k2 in self.arcs[k1]:
                    self.dist[k1][k2] = self.arcs[k1][k2]
                    self.prev[k1][k2] = k1
                else:
                    self.dist[k1][k2] = inf
                    self.prev[k1][k2] = None

        for k in self.arcs:
            for i in self.arcs:
                for j in self.arcs:
                    if self.dist[i][j] - threshold > self.dist[i][k] + self.dist[k][j]:
                        self.dist[i][j] = self.dist[i][k] + self.dist[k][j]
                        self.prev[i][j] = self.prev[k][j]

    def path(self, u, v):
        if self.prev[u][v] == None:
            return []
        p = [v]
        while u != v:
            v = self.prev[u][v]
            p.append(v)
        p.reverse()
        return p

    def find_violated(self):
        ps = []
        
        interesting_paths = {((UV.u, False, i, j),(UV.v, False, i,j)) for (_, _, i, j) in self.arcs}
        for (s,t) in interesting_paths:
            cost = self.dist[s][t]
            print(f"{s} -> {t}: {cost}")
            if cost < 0.5 - threshold:
                p = set([(n1, n2) for (_, _, n1, n2) in self.path(s,t)])
                ps.append(p)
                print(f"Violated! Path is :{p}, length {len(p)}")
        return ps

class weight_accesser:
    def __init__(self, random_weights):
        
        if not random_weights:
            # define the solution vector x here
            self.weights = [
                [0, 0.5, 0, 0, 0.5],
                [0.5, 0, 0.5, 0, 0],
                [0, 0.5, 0, 0.5, 0],
                [0, 0, 0.5, 0, 0.5],
                [0.5, 0, 0, 0.5, 0]]
            
            #self.weights = [
            #        [0/2, 1/2, 1/2, 1/2, 1/3, 1/2],
            #        [0/2, 0/2, 1/4, 1/4, 0/2, 1/3],
            #        [0/2, 0/2, 0/2, 0/2, 1/4, 1/4],
            #        [0/2, 0/2, 0/2, 0/2, 0/2, 1/2],
            #        [0/2, 0/2, 0/2, 0/2, 0/2, 1/3],
            #        [0/2, 0/2, 0/2, 0/2, 0/2, 0/2]]
        else:
            self.degree = 6

            self.weights = [[0]* self.degree for _ in range(self.degree)]
            for i in range(self.degree):
                for j in range(self.degree):
                    self.weights[i][j] = random.choice([0, 1/4, 1/3, 1/2, 2/3, 3/4, 1])

    def satisfies_triangles(self):
        for i in range(self.degree):
            for j in range(self.degree):
                for k in range(self.degree):
                    if self[(i,j)] + self[(i,k)] - self[(j,k)] > 1:
                        return False
        return True

    def __getitem__(self,key):
        (i, j) = key
        if i <= j:
            return self.weights[i][j]
        else:
            return self.weights[j][i]

def contains_odd_cycle(paths):
    for path in paths:
        if len(path) % 2 == 0:
            continue
        vertices = [elem for sublist in path for elem in sublist]
        counts = dict(list(map(lambda digit: (digit, vertices.count(digit)), set(vertices))))
        
        cycle = True
        for (digit, count) in counts.items():
            if count != 2:
                cycle = False
                break
        if cycle:
            return True
    return False


iteration, ws = 0, 0
random_weights = True

while True:
    ws += 1
    if ws % 100 == 0:
        print(f"generated ws: {ws}")
    w = weight_accesser(random_weights)
    if random_weights and not w.satisfies_triangles():
        continue

    iteration += 1
    if iteration % 10 == 0:
        print(f"iteration: {iteration}")

    print(w.weights)
    g = H_Graph()
    g.fill_from_weights(len(w.weights), w)
    g.floyd_warshall()
    
    ps = g.find_violated()
    print(f"Found {len(ps)} violated inequalities:")
    print(ps)
    if contains_odd_cycle(ps):
        print("Odd cycle found!!!")
        break
    if not random_weights:
        break
