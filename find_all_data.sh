#!/usr/bin/env bash

find data/ -type d -exec test -e '{}'/data.csv -a -e '{}'/data.toml \; -printf "%p/ "
