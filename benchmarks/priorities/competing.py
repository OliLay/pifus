from framework import framework, plot
import time

file_prefix = "write_competing"

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
        pifus_writer_path, args=f"192.168.1.201 -p 11337 -l HIGH -o {file_prefix}_HIGH")
    framework.start_process(
        pifus_writer_path, args=f"192.168.1.202 -p 11337 -l MEDIUM -o {file_prefix}_MEDIUM")
    framework.start_process(
        pifus_writer_path, args=f"192.168.1.203 -p 11337 -l LOW -o {file_prefix}_LOW")

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

    plot.latency_scatter(data, output=f"{file_prefix}.png",
                         legend_title="Priority", xlabel="Time [s]", ylabel="Latency [ms]")
    plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
