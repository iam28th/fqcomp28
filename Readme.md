Reimplementation of fqzcomp4 compression tool with caryless rangecoder replaced by fse codec & static mode, aiming at increased throughput for cases when speed is overly important, such as disk-based reads reordering and real-time compression.

# Build from source

```bash
git clone git@github.com:iam28th/fqcomp28.git
cd fqcomp28
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

./fqcomp28 --help-all
```

# Usage

```bash
./fqcomp28 c --input reads.fastq --output archive.fqc --threads 4
./fqcomp28 d --input archive.fqc --output1 decompressed.fastq --threads 4
```

# Comparison with fqzcomp{4,5}

`fqcomp28` seemingly offers a better compression ratio/speed tradeoff than `fqzcomp4` and `fqzcomp5` with default mode (-5), but looks inferior to faster modes of `fqzcomp5` (-3 and -1), particulary because of longer decompression times (the cause of which is [under investigation](https://github.com/iam28th/fqcomp28/issues/1)).

All tools noticeably outperform `gzip`/`pigz` in both speed and compression ratio.

See [Results.md](benchmark/Results.md) for details.

# Limitations
- headers
    - must all have same structure (same number of "fields" consisting of alphanumeric characters with non-alphanumeric separators between the "fields");
    - a "field" can be no more than 255 characters long;
    - each header must end with an alphanumeric character;
- read lengths must fit into uint32_t (that is, be <= 4294967296);
- bases must be one of `ACGTN`;
- quality headers (that is, repeated headers on the 3rd line of the record, after `+`), if present, are discarded in decompressed data (but otherwise compression is lossless).

# References 

- [fqzcomp4](https://github.com/jkbonfield/fqzcomp)
- [fqzcomp5](https://github.com/jkbonfield/fqzcomp5)
- [FiniteStateEntropy](https://github.com/Cyan4973/FiniteStateEntropy)
- [libbsc](https://github.com/IlyaGrebnov/libbsc)
