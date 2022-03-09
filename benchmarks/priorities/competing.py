from framework import framework, plot
import time


def measure():
    """Three pifus writers (tap0), three lwIP readers (tap1-3)"""
    framework.start_stack()

    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(lwip_reader_path, args="192.168.1.201", tapif=1)
    framework.start_process(lwip_reader_path, args="192.168.1.202", tapif=2)
    framework.start_process(lwip_reader_path, args="192.168.1.203", tapif=3)

    # make sure readers are up
    time.sleep(1)

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/writer/pifus_writer")
    framework.start_process(
        pifus_writer_path, args="192.168.1.201 -p 11337 -l HIGH")
    framework.start_process(
        pifus_writer_path, args="192.168.1.202 -p 11337 -l MEDIUM")
    framework.start_process(
        pifus_writer_path, args="192.168.1.203 -p 11337 -l LOW")

    time.sleep(10)
    framework.kill_all_processes()


def draw_plots():
    high_data = framework.ts_to_latency_time_tuple(
        "writePRIORITY_HIGH.txt", "writtenPRIORITY_HIGH.txt", "High")
    medium_data = framework.ts_to_latency_time_tuple(
        "writePRIORITY_MEDIUM.txt", "writtenPRIORITY_MEDIUM.txt", "Medium")
    low_data = framework.ts_to_latency_time_tuple(
        "writePRIORITY_LOW.txt", "writtenPRIORITY_LOW.txt", "Low")

    data = low_data + medium_data + high_data

    plot.latency_scatter(data, output="competing.png",
                         legend_title="Priority", xlabel="Time [s]", ylabel="Latency [ms]")
    plot.latency_dataframe_stats(data, output="competing.txt")
