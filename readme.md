Reimplementation of fqzcomp4 compression tool with caryless rangecoder replaced by huff0 codec & static mode for increased throughput

# Building

```bash
git clone git@github.com:iam28th/fqzcomp28.git
cd fqzcomp28
cmake -S src -B build 
cmake --build build
```
after that the executable is `./build/bin/fqzcomp28` 
TODO: copy/move executable to project root in CMake 

// gcc version ≥ 11 is needed (requirement comes from SeqAn3).

# Usage

# Comparison with fqzcomp

# Limitations 

## Headers
header in the dataset should have the same structure
its "fields" (that is, substrings between two non-alphanumeric characters) must be no more than 255 characters long
the last character in a header must be alphanumeric
