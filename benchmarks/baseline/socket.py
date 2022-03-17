from framework import framework, plot
import time

file_prefix = "baseline_socket"
amount_sockets = 8


def measure():
    """One pifus writer (multiple sockets), one lwIP reader.
    One lwIP socket writer (multiple sockets), one lwIP reader."""
    framework.start_stack(affinity="0-1")

    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1)

    # make sure readers are up
    time.sleep(1)

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/baseline/writer/pifus_baseline_writer")
    framework.start_process(
        pifus_writer_path, args=f"192.168.1.201 -p 11337 -o {file_prefix}_pifus -c {amount_sockets}")

    framework.wait()
    framework.kill_all_processes()


def draw_plots():
    data = []

    for index in range(0, amount_sockets):
        data += framework.ts_to_latency_time_tuple(
            f"{file_prefix}_pifus_{index}_tx.txt", f"{file_prefix}_pifus_{index}_txed.txt", f"pifus{index}")

    # TODO: calculate metric for each batch (e.g. mean)
    plot.latency_scatter(data, output=f"{file_prefix}.png",
                         legend_title="API", xlabel="Time [s]", ylabel="Latency [ms]")
    plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
