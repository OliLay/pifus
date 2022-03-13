from framework import framework, plot
import time

file_prefix = "dummy_async_competing"

def measure():
    """Stack and three competing dummies with different prios."""
    framework.start_stack(affinity="0-2")

    pifus_dummy_path = framework.get_binary_path(
        "api/benchmarks/dummy/pifus_dummy_async")
    framework.start_process(
        pifus_dummy_path, args=f"-l HIGH -o {file_prefix}_HIGH", affinity="3-7")
    framework.start_process(
        pifus_dummy_path, args=f"-l MEDIUM -o {file_prefix}_MEDIUM", affinity="3-7")
    framework.start_process(
        pifus_dummy_path, args=f"-l LOW -o {file_prefix}_LOW", affinity="3-7")

    framework.wait()
    framework.kill_all_processes()


def draw_plots():
    high_data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_HIGH_tx.txt", f"{file_prefix}_HIGH_txed.txt", "High")
    medium_data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_MEDIUM_tx.txt", f"{file_prefix}_MEDIUM_txed.txt", "Medium")
    low_data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_LOW_tx.txt", f"{file_prefix}_LOW_txed.txt", "Low")

    data = low_data + medium_data + high_data
    
    plot.latency_scatter(data, output=f"{file_prefix}.png", legend_title="Priority",
                         xlabel="Time [s]", ylabel="Latency [us]", latency_unit="us")
    plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
