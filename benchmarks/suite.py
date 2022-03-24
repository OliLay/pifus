from multiprocessing import Process
from typing import List

import priorities.competing
import dummy_stack.multiple_async
import dummy_stack.multiple_sync
import dummy_stack.single
import dummy_stack.sockets_async
import baseline.raw
import baseline.netconn
import baseline.socket_reader

modules = [
    baseline.socket_reader,
    baseline.netconn,
    baseline.raw,
    dummy_stack.multiple_sync,
    dummy_stack.multiple_async,
    dummy_stack.single,
    priorities.competing,
]


def measure():
    for module in modules:
        module.measure()


def plot():
    processes: List[Process] = []
    for module in modules:
        process = Process(target=module.draw_plots, args=())
        processes.append(process)

        process.start()

    for process in processes:
        process.join()


if __name__ == "__main__":
    measure()
    plot()

    exit(0)
