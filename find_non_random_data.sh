#!/usr/bin/env bash

find data/ -type d -not -path "*/random/*" -exec test -e '{}'/data.csv -a -e '{}'/data.toml \; -printf "%p/ "
