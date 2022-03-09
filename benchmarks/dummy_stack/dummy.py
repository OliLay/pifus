from framework import framework, plot
import time


def measure():
    """Dummy stack and three competing writers with different prios."""
    framework.start_stack()

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/dummy/pifus_dummy")
    framework.start_process(
        pifus_writer_path, args="-l HIGH")
    framework.start_process(
        pifus_writer_path, args="-l MEDIUM")
    framework.start_process(
        pifus_writer_path, args="-l LOW")

    time.sleep(10)
    framework.kill_all_processes()


def draw_plots():
    high_data = framework.ts_to_latency_time_tuple(
        "dummy_txPRIORITY_HIGH.txt", "dummy_txedPRIORITY_HIGH.txt", "High")
    medium_data = framework.ts_to_latency_time_tuple(
        "dummy_txPRIORITY_MEDIUM.txt", "dummy_txedPRIORITY_MEDIUM.txt", "Medium")
    low_data = framework.ts_to_latency_time_tuple(
        "dummy_txPRIORITY_LOW.txt", "dummy_txedPRIORITY_LOW.txt", "Low")

    data = low_data + medium_data + high_data

    plot.latency_scatter(data, output="dummy-competing.png", legend_title="Priority",
                         xlabel="Time [s]", ylabel="Latency [us]", latency_unit="us")
    plot.latency_dataframe_stats(data, output="dummy-competing.txt")
