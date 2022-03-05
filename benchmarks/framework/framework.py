import subprocess
import os
import glob
from typing import List, Optional

BUILD_FOLDER = "build"

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

    cmd = [path] + args.split()

    proc = subprocess.Popen(cmd, env=env)
    _running_processes.append(proc)
    return proc


def compute_latency_from_stamps(first_file_path: str, second_file_path: str) -> List[int]:
    latencies = []
    with open(first_file_path) as first_file:
        with open(second_file_path) as second_file:
            for line in second_file:
                first = int(first_file.readline())
                second = int(line)
                latencies.append(second - first)

    return latencies


def kill_all_processes():
    for proc in _running_processes:
        proc.kill()
    _running_processes.clear()
