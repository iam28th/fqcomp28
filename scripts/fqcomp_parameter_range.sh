#!/bin/bash

# Runs compression with all values from the range and saves log to a file
#
# Usage:
# ./fqzcomp_parameter_range.sh file.fastq key val1,val2,val3,...
#
# Example:
# ./fqzcomp_parameter_range.sh file.fastq -S 1,4,8

if [[ $# -ne 3 ]]
then
    echo "Not enough arguments" && exit 1
fi

input1="$1"
key="$2"
values="$3"

THREADS=8

# remove .fq|.fastq suffixes
dataset=$(basename "$input1")
dataset=${dataset%.fq}
dataset=${dataset%.fastq}

# remove 1 or 2 leading dashes
keyname=${key##-}
keyname=${keyname##-}

IFS=','; for val in $values; do
    logfile="log_$dataset"_"$keyname""_$val"
    cmd="/usr/bin/time -v fqcomp28 c --threads "$THREADS" --i1 $input1 -o out.fqc $key $val 2> $logfile 1>&2 "
    echo "$cmd"
    eval "$cmd"
    if [[ $? -ne 0 ]]
    then
        echo "error at $key = $val"
        exit 1
    fi
done
