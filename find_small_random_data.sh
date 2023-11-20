#!/usr/bin/env bash

find data/random/ -mindepth 1 -path "*s25*" -type d  -printf "%p/ "
