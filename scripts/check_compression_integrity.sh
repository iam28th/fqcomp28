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

/usr/bin/time -v ./fqzcomp28 c --input1 "$INP1" -o "$ARCHIVE" --threads "$THREADS" && \
        ./fqzcomp28 d --input "$ARCHIVE" --o1 "$DECOMP1" && \
        "$DIFFCMD" "$INP1" "$DECOMP1" \
        && cleanup
