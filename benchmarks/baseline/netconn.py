from framework import framework, plot
import time

file_prefix = "baseline_netconn"


def start_pifus_bench(amount_sockets: int):
    framework.start_stack(affinity="0-2")

    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1, affinity="4")

    # make sure readers are up
    time.sleep(1)

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/baseline/writer/pifus_baseline_writer")
    framework.start_process(
        pifus_writer_path,
        args=f"192.168.1.201 -p 11337 -o {file_prefix}_pifus -c {amount_sockets}",
        affinity="6")

    framework.wait()
    framework.kill_all_processes()


def start_netconn_bench(amount_sockets: int):
    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1)

    # make sure readers are up
    time.sleep(1)

    lwip_netconn_path = framework.get_binary_path(
        "custom/benchmark/netconn-writer/netconn_writer", benchmark_build=True)
    framework.start_process(
        lwip_netconn_path, args=f"192.168.1.201 -p 11337 -o {file_prefix} -c {amount_sockets}", tapif=0)

    framework.wait()
    framework.kill_all_processes()


def measure():
    """X pifus writer (multiple sockets), one lwIP reader.
    X lwIP netconn writer (multiple sockets), one lwIP reader."""
    amount_sockets = [1, 2, 4, 8, 16, 32]

    for current_amount_sockets in amount_sockets:
        start_pifus_bench(current_amount_sockets)
        start_netconn_bench(current_amount_sockets)

        pifus_data = []
        netconn_data = []
        for index in range(0, current_amount_sockets):
            pifus_data += framework.ts_to_latency_time_tuple(
                f"{file_prefix}_pifus_{index}_tx.txt", f"{file_prefix}_pifus_{index}_txed.txt", f"pifus")
            netconn_data += framework.ts_to_latency_time_tuple(
                f"{file_prefix}_{index}_tx.txt", f"{file_prefix}_{index}_txed.txt", f"netconn")

        plot.latency_dataframe_stats(pifus_data, output=f"{file_prefix}.txt")
        plot.latency_dataframe_stats(netconn_data, output=f"{file_prefix}.txt")

        # TODO: delete previous measurements, then measure again.
        # TODO: save mean or other metric, then make plot for all runs


def draw_plots():
    pass
    #data = []

    # for index in range(0, amount_sockets):
    #   data += framework.ts_to_latency_time_tuple(
    #       f"{file_prefix}_pifus_{index}_tx.txt", f"{file_prefix}_pifus_{index}_txed.txt", f"pifus{index}")

    # TODO: calculate metric for each batch (e.g. mean)
    # plot.latency_scatter(data, output=f"{file_prefix}.png",
    #                    legend_title="API", xlabel="Time [s]", ylabel="Latency [ms]")
    #plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
