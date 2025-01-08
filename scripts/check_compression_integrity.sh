#!/bin/bash

ARCHIVE="archive.f2q8z"
DECOMP1="decomp1.fastq"
INP1="$1"
THREADS="$2"
if [[ -z "$THREADS" ]]
then
        THREADS=4
fi

DIFFCMD=cmp

cleanup()
{
        echo "no diff found"
        rm -f "$ARCHIVE" "$DECOMP1"
        return
}

/usr/bin/time -v ./fqcomp28 c --input1 "$INP1" -o "$ARCHIVE" --threads "$THREADS" && \
        ./fqcomp28 d --input "$ARCHIVE" --o1 "$DECOMP1" --threads "$THREADS" && \
        "$DIFFCMD" "$INP1" "$DECOMP1" \
        && cleanup
