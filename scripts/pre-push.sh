#!/usr/bin/bash

# 0. CMake configuration step
# 1. clang-tidy on all .cpp files in src/
# 2. Unittests via CTest

# how many threads are used for clang-tidy & ctest
JOBS=4
CMAKE_BINARY_DIR=./build

echo "### pre-push start..."

cmake -S . -B "$CMAKE_BINARY_DIR" 2> /dev/null 1>&2
if [[ $? -ne 0 ]]; then
        echo "error during build"
        exit 1
fi

run-clang-tidy -h 2> /dev/null 1>&2

ct_code=$?
if [[ $ct_code -ne 0 ]]; then
        # if clang-tidy is not present on the system,
        # only print warning message
        readonly MESSAGE="WARN: clang-tidy not found, static analysis skipped"
        echo $MESSAGE
else
        # TODO: better file selection & run only on files that were changed since divergance
        # -Wno-unknown-warning-option is needed because clang-tidy doesn't know about -Wnrvo
        run-clang-tidy -p build -j "$JOBS" -quiet src/*.cpp -extra-arg=-Wno-unknown-warning-option  || exit 1
fi

cd "$CMAKE_BINARY_DIR/test" && make -j "$JOBS" && ctest --schedule-random --parallel "$JOBS" || exit 1

echo "### ...pre-push end"
