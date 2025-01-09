All datasets are public and can be downloaded using `fasterq-dump` from `sra-toolkit`.

To (hopefully) reproduce or run on a different .fastq file, build `fqcomp28` and run `./run_benchmark.py dataset.fastq`.
The script downloads and builds `fqzcomp4` and `fqzcomp5`, and runs these tools, `fqcomp28`, and `gzip` on a specified dataset. 
Two .csv with results are produced - for single and for 8 threads.

--- 

## 8 threads

DRR296203_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  15.63 |                    36.73 |               4.417 |
| fqzcomp5 -5 |                  35.34 |                    20.48 |               4.436 |
| fqzcomp5 -3 |                  13.33 |                     8.49 |               4.301 |
| fqzcomp5 -1 |                  11.93 |                    16.5  |               4.254 |
| pigz        |                 137.9  |                    33.05 |               2.983 |

SRR065390_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  19.37 |                    36.36 |               5.874 |
| fqzcomp5 -5 |                  72.5  |                    86.86 |               5.891 |
| fqzcomp5 -3 |                  17.79 |                    16.68 |               5.602 |
| fqzcomp5 -1 |                  15.65 |                    16.64 |               5.359 |
| pigz        |                 130.49 |                    33.84 |               3.651 |

SRR12285250_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  10.82 |                    23.7  |               8.816 |
| fqzcomp5 -5 |                  69.69 |                    85.36 |               9.783 |
| fqzcomp5 -3 |                  15.51 |                    14.85 |               8.718 |
| fqzcomp5 -1 |                  13.45 |                    18.16 |               8.412 |
| pigz        |                 120.01 |                    29.82 |               5.372 |

SRR25230149_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                   4.15 |                     7.76 |               9.644 |
| fqzcomp5 -5 |                  24.55 |                    27.55 |              14.12  |
| fqzcomp5 -3 |                   4.68 |                     2.73 |               9.207 |
| fqzcomp5 -1 |                   3.36 |                     1.37 |               8.826 |
| pigz        |                  36.02 |                     5.64 |               5.659 |

SRR22543904_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                   1.58 |                     2.69 |               8.973 |
| fqzcomp5 -5 |                  11.79 |                    12.29 |              12.498 |
| fqzcomp5 -3 |                   2.3  |                     0.72 |               8.542 |
| fqzcomp5 -1 |                   1.08 |                     0.45 |               8.222 |
| pigz        |                  12.24 |                     1.73 |               5.528 |


## Single thread


DRR296203_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  39.08 |                    77.16 |               4.415 |
| fqzcomp4    |                  92.32 |                   114.73 |               4.557 |
| fqzcomp5 -5 |                 115.44 |                    75.52 |               4.436 |
| fqzcomp5 -3 |                  43.09 |                    25.72 |               4.301 |
| fqzcomp5 -1 |                  38.28 |                    25.96 |               4.254 |
| gzip        |                 733.44 |                    41.26 |               2.987 |

SRR065390_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  55.37 |                    92.88 |               5.847 |
| fqzcomp4    |                  97.93 |                   118.1  |               6.249 |
| fqzcomp5 -5 |                 226.05 |                   243.94 |               5.891 |
| fqzcomp5 -3 |                  57.94 |                    27.45 |               5.602 |
| fqzcomp5 -1 |                  44.5  |                    21.85 |               5.359 |
| gzip        |                 716.7  |                    43.49 |               3.657 |

SRR12285250_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  39.6  |                    74.8  |               8.813 |
| fqzcomp4    |                  76.08 |                    93.85 |               9.796 |
| fqzcomp5 -5 |                 194.56 |                   227.56 |               9.783 |
| fqzcomp5 -3 |                  41.11 |                    21.12 |               8.718 |
| fqzcomp5 -1 |                  35.24 |                    18.64 |               8.412 |
| gzip        |                 648.44 |                    45.18 |               5.378 |

SRR25230149_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                  15.68 |                    23.15 |               9.643 |
| fqzcomp4    |                  22.91 |                    28.97 |              12.815 |
| fqzcomp5 -5 |                  61.65 |                    68.7  |              14.12  |
| fqzcomp5 -3 |                  13.57 |                     5.32 |               9.207 |
| fqzcomp5 -1 |                   9.05 |                     4.07 |               8.826 |
| gzip        |                 203.55 |                    11.79 |               5.667 |

SRR22543904_1:

| Tool        |   Compression time (s) |   Decompression time (s) |   Compression ratio |
|:------------|-----------------------:|-------------------------:|--------------------:|
| **fqcomp28**    |                   4.37 |                     7.64 |               8.973 |
| fqzcomp4    |                   7.55 |                     9.63 |              10.635 |
| fqzcomp5 -5 |                  25.19 |                    24.15 |              12.498 |
| fqzcomp5 -3 |                   5.73 |                     1.56 |               8.542 |
| fqzcomp5 -1 |                   2.8  |                     1.25 |               8.222 |
| gzip        |                  68.77 |                     3.65 |               5.536 |
