#!/usr/bin/bash

# runs clang-format on patches in staged files,
# and stages these files again afterwards so that
# commit contains formatted files

echo "### pre-commit start..."

git clang-format -h 2> /dev/null 1>&2
cf_code=$?
if [[ $cf_code -ne 0 ]]; then
        # if clang-format is not present on the system,
        # only print warning message
        readonly MESSAGE="WARN: clang-format not found, auto-formatting skipped"
        echo $MESSAGE
else
        readonly staged_files=$(git diff --cached --name-only)
        git clang-format
        for f in $staged_files; do
                if [ -f "$f" ]
                then
                        git add $f
                fi
        done
fi

echo "### ...pre-commit end"
