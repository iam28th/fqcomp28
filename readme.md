Reimplementation of fqzcomp4 compression tool with caryless rangecoder replaced by fse codec & static mode for increased throughput

# Building

```bash
git clone git@github.com:iam28th/fqzcomp28.git
cd fqzcomp28
cmake -S . -B build
cmake --build build

./fqzcomp28 -h
```

// TODO invistigate compatable gcc/clang

# Usage

# Comparison with fqzcomp

# Limitations

- read lengths currently must be <= 32767
- bases must be one of `ACGTN`
- headers
    - must all have same structure
    - "fields" (that is, substrings between two non-alphanumeric characters) must be no more than 255 characters long
    - the last character in a header must be alphanumeric
- current implementation is single-threaded
