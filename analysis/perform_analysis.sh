#!/usr/bin/env bash

# exclude results for ‘violated-half’ and ‘violated-two’ in analysis!
find nonrandom_lp_results/ random_lp_results/ -mindepth 4 -name measurements.csv -not -path "*violated*" -exec ./analysis.py -u -n "all_" {} +
find nonrandom_lp_results/ -mindepth 4 -name measurements.csv -not -path "*violated*" -exec ./analysis.py -n "non_random_" {} +
find random_lp_results/ -mindepth 4 -name measurements.csv -not -path "*violated*" -exec ./analysis.py -n "random_" {} +
