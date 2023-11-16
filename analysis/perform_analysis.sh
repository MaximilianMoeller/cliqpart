#!/usr/bin/env bash

# exclude results for ‘violated-half’ and ‘violated-two’ in analysis!
find nonrandom_lp_results/ -mindepth 4 -name measurements.csv -not -path "*violated*" -exec ./analysis.py {} +
