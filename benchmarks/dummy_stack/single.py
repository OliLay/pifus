from framework import framework, plot
import time

file_prefix = "dummy_single"


def measure():
    """Stack and one dummy, only inserting one operation after one has returned."""
    framework.start_stack()

    pifus_dummy_path = framework.get_binary_path(
        "api/benchmarks/dummy/pifus_dummy_single"
    )
    framework.start_process(pifus_dummy_path, args=f"-l HIGH -o {file_prefix}")

    framework.wait()
    framework.kill_all_processes()


def draw_plots():
    data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_tx.txt", f"{file_prefix}_txed.txt", "High"
    )

    plot.latency_scatter(
        data,
        output=file_prefix,
        legend_title="",
        xlabel="Time [s]",
        ylabel="Latency [us]",
        latency_unit="us",
        latency_scale="log",
    )
    plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
