#!/bin/bash

ARCHIVE="archive.f2q8z"
DECOMP1="decomp1.fastq"
INP1="$1"

# TODO use cmp for large files
DIFFCMD=diff

cleanup() 
{
        echo "no diff found"
        rm -f "$ARCHIVE" "$DECOMP1"
        return 
}

./fqzcomp28 c --input1 "$INP1" -o "$ARCHIVE"
./fqzcomp28 d --input "$ARCHIVE" --o1 "$DECOMP1"
"$DIFFCMD" "$INP1" "$DECOMP1" && cleanup
