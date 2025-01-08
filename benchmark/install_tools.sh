#!/bin/bash

function clone_and_make() {
    local dir
    dir="$1"

    local url
    url="$2"

    if [[ ! -e "$dir" ]]
    then
        git clone --recurse-submodules "$url" "$dir"
        (cd "$dir" && make -j)
    fi
}

clone_and_make "./fqzcomp4" git@github.com:jkbonfield/fqzcomp.git
clone_and_make "./fqzcomp5" git@github.com:jkbonfield/fqzcomp5.git
