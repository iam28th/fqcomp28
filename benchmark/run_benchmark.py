#!/bin/python

import csv
import dataclasses
import os
import subprocess as sp
import sys
from os import path

ARCHIVE = "out.compr"
DECOMPRESSED_NAME = "decomp.fastq"
THREADS = 8


@dataclasses.dataclass
class Result:
    name: str = ""
    ctime: float = 0
    dtime: float = 0
    total_cr: float = 0


@dataclasses.dataclass
class Tool:
    name: str = ""
    binary: str = ""
    ccmd: str = ""
    dcmt: str = ""


def measure_tool(tool: Tool, dataset) -> Result:
    ret = Result()
    ret.name = tool.name

    cproc = run_process(tool.ccmd, gnu_time=True, verbose=True)
    dproc = run_process(tool.dcmd, gnu_time=True, verbose=True)

    ret.ctime = get_elapsed_time(cproc)
    ret.dtime = get_elapsed_time(dproc)
    ret.total_cr = calc_total_cr(dataset, ARCHIVE)

    os.unlink(ARCHIVE)
    os.unlink(DECOMPRESSED_NAME)

    return ret


def calc_total_cr(dataset: str, archive: str) -> float:
    return round(path.getsize(dataset) / path.getsize(archive), 3)


def run_process(cmd: str, gnu_time=True, verbose=True) -> sp.CompletedProcess:
    if gnu_time:
        cmd = "/usr/bin/time -v " + cmd
    if verbose:
        print(cmd)
    return sp.run(cmd, capture_output=True, check=True, shell=True)


def get_elapsed_time(proc: sp.CompletedProcess) -> float:
    stderr_lines = proc.stderr.decode("utf8").splitlines()
    for line in stderr_lines:
        line = line.strip()
        if not line.startswith("Elapsed"):
            continue
        timestr = line.split()[-1]
        if timestr.count(":") == 1:  # m:ss
            m, s = map(float, timestr.split(":"))
            return m * 60 + s
        else:  # h:mm:ss
            h, m, s = map(float, timestr.split(":"))
            return h * 3600 + m * 60 + s
    return -1


def fqzcomp4(dataset: str) -> Tool:
    tool = Tool()
    tool.name = "fqzcomp4"
    tool.binary = "./fqzcomp4/fqzcomp"
    tool.ccmd = f"{tool.binary} -X -P {dataset} {ARCHIVE}"
    tool.dcmd = f"{tool.binary} -d -X -P {ARCHIVE} {DECOMPRESSED_NAME}"
    return tool


def fqzcomp5(dataset: str, level: int = 5, threads: int = 1) -> Tool:
    tool = Tool()
    tool.name = "fqzcomp5 -" + str(level)
    tool.binary = "./fqzcomp5/fqzcomp5"
    tool.ccmd = f"{tool.binary} -{level} -t {threads} {dataset} {ARCHIVE}"
    tool.dcmd = f"{tool.binary} -d -t {threads} {ARCHIVE} {DECOMPRESSED_NAME}"
    return tool


def fqcomp28(dataset: str, threads: int = 1) -> Tool:
    tool = Tool()
    tool.name = "fqcomp28"
    tool.binary = "../fqcomp28"
    tool.ccmd = f"{tool.binary} c --input1 {dataset} --output {ARCHIVE} -t {threads}"
    tool.dcmd = f"{tool.binary} d -i {ARCHIVE} --o1 {DECOMPRESSED_NAME} -t {threads}"
    return tool


def gzip(dataset: str) -> Tool:
    tool = Tool()
    tool.name = "gzip"
    tool.binary = "gzip"
    tool.ccmd = f"{tool.binary} --keep {dataset} -c > {ARCHIVE}"
    tool.dcmd = f"{tool.binary} -d --keep {ARCHIVE} -c > {DECOMPRESSED_NAME}"
    return tool


def pigz(dataset: str, threads: int = 1) -> Tool:
    tool = Tool()
    tool.name = "pigz"
    tool.binary = "pigz"
    tool.ccmd = f"{tool.binary} --keep {dataset} -p {threads} -c > {ARCHIVE}"
    tool.dcmd = (
        f"{tool.binary} -d --keep {ARCHIVE} -p {threads} -c > {DECOMPRESSED_NAME}"
    )
    return tool


def exec_exists(binary: str) -> bool:
    if path.exists(binary):
        return True
    proc = sp.run(f"which {binary}", capture_output=True, shell=True)
    return len(proc.stdout) != 0


def write_results_to_csv(dataset: str, results: list[Result], suffix: str):
    assert len(results) > 0

    outname = path.basename(dataset)
    outname = path.splitext(outname)[0] + f"_{suffix}.csv"

    with open(outname, "w") as fout:
        writer = csv.DictWriter(
            fout,
            dialect="unix",
            quoting=csv.QUOTE_MINIMAL,
            fieldnames=dataclasses.asdict(results[0]).keys(),
        )
        writer.writeheader()

        for res in results:
            writer.writerow(dataclasses.asdict(res))


def run_benchmark(dataset: str, tools: list[Tool], file_suffix: str):
    results = []
    for tool in tools:
        if exec_exists(tool.binary):
            res = measure_tool(tool, dataset)
            results.append(res)
            print(res)
        else:
            print(tool.binary + " not found, skipping...", file=sys.stderr)

    write_results_to_csv(dataset, results, file_suffix)


sp.run("./install_tools.sh", shell=True, check=True)

dataset = sys.argv[1]

single_thread = [fqcomp28(dataset, threads=1)]
single_thread.append(fqzcomp4(dataset))
for level in [1, 3, 5]:
    single_thread.append(fqzcomp5(dataset, level, threads=1))
single_thread.append(gzip(dataset))

parallel = [fqcomp28(dataset, threads=THREADS)]
for level in [1, 3, 5]:
    parallel.append(fqzcomp5(dataset, level, threads=THREADS))
parallel.append(pigz(dataset, threads=THREADS))

run_benchmark(dataset, single_thread, "t1")
run_benchmark(dataset, parallel, "t" + str(THREADS))
