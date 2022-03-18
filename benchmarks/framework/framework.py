import subprocess
import os
import glob
import time
from pathlib import Path
from typing import List, Optional
from dataclasses import dataclass

BUILD_FOLDER = "build"
MEASUREMENT_FOLDER = "measurements"
RESULTS_FOLDER = "results"

RUNTIME_SECONDS = 120

_running_processes: List[subprocess.Popen] = []


def get_binary_path(subpath: str, benchmark_build = False) -> str:
    if benchmark_build:
        return f"lwip-sys/{BUILD_FOLDER}/{subpath}"   
    else:
        return f"../{BUILD_FOLDER}/{subpath}"

def wait():
    time.sleep(RUNTIME_SECONDS)

def remove_measurement_file(file: str):
    path = os.path.join(MEASUREMENT_FOLDER, file)
    if os.path.exists(path):
        os.remove(path)

def start_stack(affinity: Optional[str] = None) -> subprocess.Popen:
    shm_files = glob.glob('/dev/shm/*')
    for file in shm_files:
        os.remove(file)

    stack_path = get_binary_path("lwip/custom/stack/stack")
    return start_process(stack_path, tapif=0, affinity=affinity)


def start_process(path: str, args: str = "", tapif: Optional[int] = None, affinity: Optional[str] = None) -> subprocess.Popen:
    env = {}

    if tapif is not None:
        env["PRECONFIGURED_TAPIF"] = f"tap{tapif}"

    taskset = []
    if affinity:
        taskset = ["taskset", "-a", "-c", str(affinity)]

    cmd = taskset + [os.path.abspath(path)] + args.split()
    print (cmd)

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

    if not os.path.exists(os.path.join(MEASUREMENT_FOLDER, second_file_path)):
        print(f"Txed {second_file_path} not found!")
        return []

    with open(os.path.join(MEASUREMENT_FOLDER, first_file_path)) as first_file:
        with open(os.path.join(MEASUREMENT_FOLDER, second_file_path)) as second_file:
            for line in second_file:
                if not line:
                    break
                
                first_line = first_file.readline()
                if not first_line:
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
