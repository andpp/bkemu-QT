#!/bin/bash

iconv "$1" -f UTF-16 -t UTF-8 | grep " IDS_" | sort | awk 'BEGIN {id = ""; str = ""} {id = id $1 ",\n"; $1=""; str = str substr($0, 1, length($0)-1) ",\n"} END { print id; print str;}'
