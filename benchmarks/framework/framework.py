from dataclasses import dataclass
import subprocess
import os
import glob
from pathlib import Path
from typing import List, Optional

BUILD_FOLDER = "build"
MEASUREMENT_FOLDER = "measurements"
RESULTS_FOLDER = "results"

_running_processes: List[subprocess.Popen] = []


def get_binary_path(subpath: str) -> str:
    return f"../{BUILD_FOLDER}/{subpath}"


def start_stack() -> subprocess.Popen:
    shm_files = glob.glob('/dev/shm/*')
    for file in shm_files:
        os.remove(file)

    stack_path = get_binary_path("lwip/custom/stack/stack")
    return start_process(stack_path, tapif=0)


def start_process(path: str, args: str = "", tapif: Optional[int] = None) -> subprocess.Popen:
    env = {}

    if tapif is not None:
        env["PRECONFIGURED_TAPIF"] = f"tap{tapif}"

    cmd = [os.path.abspath(path)] + args.split()

    Path(MEASUREMENT_FOLDER).mkdir(parents=True, exist_ok=True)

    proc = subprocess.Popen(cmd, env=env, cwd=MEASUREMENT_FOLDER)
    _running_processes.append(proc)
    return proc


@dataclass
class LatencyTimeTuple:
    latency: int
    time: int
    type: str


def ts_to_latency_time_tuple(first_file_path: str, second_file_path: str, type: str) -> List[LatencyTimeTuple]:
    tuples: List[LatencyTimeTuple] = []
    base_offset: Optional[int] = None

    with open(os.path.join(MEASUREMENT_FOLDER, first_file_path)) as first_file:
        with open(os.path.join(MEASUREMENT_FOLDER, second_file_path)) as second_file:
            for line in second_file:
                first_line = first_file.readline()
                if not line or not first_line:
                    print("Empty line")
                    break

                first = int(first_line)
                second = int(line)

                if base_offset is None:
                    base_offset = first

                tuples.append(LatencyTimeTuple(
                    type=type, latency=second - first, time=first - base_offset))

    return tuples


def kill_all_processes():
    for proc in _running_processes:
        proc.kill()
    _running_processes.clear()
