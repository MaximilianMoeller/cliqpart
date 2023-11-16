#!/usr/bin/env bash

find data/random/ -mindepth 1 -type d -not -name "random_normal_s100_0_0.5" -not -name "*150*" -printf "%p/ "
