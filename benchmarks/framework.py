import subprocess
import os
import glob
from typing import List, Optional

BUILD_FOLDER = "build"


def get_binary_path(subpath: str) -> str:
    return f"{BUILD_FOLDER}/{subpath}"


def start_stack():
    shm_files = glob.glob('/dev/shm/*')
    for file in shm_files:
        os.remove(file)

    stack_path = get_binary_path("lwip/custom/stack/stack")
    start_process(stack_path, [], 0)


def start_process(path: str, args: List[str], tapif: Optional[int] = None) -> subprocess.Popen:
    env = {}

    if tapif is not None:
        env["PRECONFIGURED_TAPIF"] = f"tap{tapif}"

    return subprocess.Popen(path, args, env=env)
