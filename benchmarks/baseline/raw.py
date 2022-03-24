from framework import framework, plot
import time

file_prefix = "baseline_raw"


def measure():
    """One pifus writer, one lwIP reader."""
    framework.start_stack(affinity="0")

    lwip_reader_path = framework.get_binary_path("lwip/custom/benchmark/reader/reader")
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1, affinity="2"
    )

    # make sure readers are up
    time.sleep(1)

    pifus_writer_path = framework.get_binary_path("api/benchmarks/writer/pifus_writer")
    framework.start_process(
        pifus_writer_path,
        args=f"192.168.1.201 -p 11337 -l HIGH -o {file_prefix}_pifus",
        affinity="7",
    )

    framework.wait()
    framework.kill_all_processes()

    time.sleep(2)

    # lwIP
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1, affinity="0"
    )
    # make sure readers are up
    time.sleep(1)

    lwip_writer_path = framework.get_binary_path("lwip/custom/benchmark/writer/writer")
    framework.start_process(
        lwip_writer_path, args=f"-o {file_prefix}_lwip", tapif=0, affinity="2"
    )

    framework.wait()
    framework.kill_all_processes()


def draw_plots():
    pifus_data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_pifus_tx.txt", f"{file_prefix}_pifus_txed.txt", "pifus"
    )
    lwip_data = framework.ts_to_latency_time_tuple(
        f"{file_prefix}_lwip_tx.txt", f"{file_prefix}_lwip_txed.txt", "lwIP raw"
    )

    data = pifus_data + lwip_data

    plot.latency_scatter(
        data,
        output=f"{file_prefix}.png",
        legend_title="API",
        xlabel="Time [s]",
        ylabel="Latency [us]",
        latency_unit="us",
        latency_scale="log",
    )
    plot.latency_dataframe_stats(data, output=f"{file_prefix}.txt")
