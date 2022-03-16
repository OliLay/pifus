from multiprocessing import Process
from typing import List

import priorities.competing
import dummy_stack.multiple_async
import dummy_stack.multiple_sync
import dummy_stack.single
import dummy_stack.sockets_async
import baseline.raw


def measure():
    baseline.raw.measure()
    dummy_stack.multiple_async.measure()
    dummy_stack.multiple_sync.measure()
    dummy_stack.single.measure()
    priorities.competing.measure()


def plot():
    modules = [baseline.raw, dummy_stack.multiple_sync, dummy_stack.multiple_async,
               dummy_stack.single, priorities.competing]
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
