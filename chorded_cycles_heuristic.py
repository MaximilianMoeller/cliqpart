#!/usr/bin/env python3

import random

degree = 10
max_size = 9
overlap = 0


def intervals(degree, max_len, overlap):
    nodes = list(range(degree))
    print(f"Indices for degree = {degree}: {nodes}")
    
    for circle_len in circle_lengths(max_len):

        # positive values spacify how many elements should overlap -> same amount of overlap for all circle_lengths
        # while negative values specify how many values shoudl NOT overlap -> same pattern for all circle_lengths
        if overlap >= 0 and overlap < circle_len:
            offset = circle_len - overlap
        elif overlap < 0 and overlap > - circle_len:
            offset = - overlap
        else:
            print(f"Not valid: overlap = {overlap}, circle_length = {circle_len}.")
            continue

        # only one way of never overlapping (should be equal to parametrized with overlap=0)
        # non_overlapping = [nodes[start:start+circle_len] for start in range(0, degree - circle_len + 1, circle_len)]
        # maximally overlapping nodes (should be equal to parametrized with overlap=-1)
        # overlapping = [nodes[start:start+circle_len] for start in range(0, degree - circle_len + 1)]
    
        intervals = [nodes[start:start+circle_len] for start in range(0, degree - circle_len + 1, offset)]
        print(f"Intervals for degree = {degree}, circle_length = {circle_len} and overlap = {overlap}: {intervals}")


def circle_lengths(max_len):
    for i in range(5, max_len + 1, 2):
        yield i



