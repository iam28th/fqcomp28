#!/usr/bin/bash

# runs cmake configuration step & clang-tidy on staged files 
# and stages these files again afterwards so that
# commit contains formatted files
# TODO: add unittests

echo "### pre-push start..."

run-clang-tidy -h 2> /dev/null 1>&2

ct_code=$?
if [[ $ct_code -ne 0 ]]; then
        # if clang-format is not present on the system,
        # only print warning message
        readonly MESSAGE="WARN: clang-tidy not found, static analysis skipped"
        echo $MESSAGE
else
        readonly staged_files=$(git diff --cached --name-only)
        cmake -S src -B build 2> /dev/null 1>&2 || echo "error during build"
        run-clang-tidy -p build -warnings-as-errors=* -j 4 "$staged_files" || exit 1
fi

echo "### ...pre-push end"
