#!/usr/bin/env bash
find . -name "optimal.sol" -exec rm '{}' \;
find . -name "optimal.log" -exec rm '{}' \;
find . -name "gurobi.log" -exec rm '{}' \;
find . -name "measurements.csv" -exec rm '{}' \;
