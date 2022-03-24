from framework import framework, plot
import time
import os
import pandas as pd  # type: ignore[import]

file_prefix = "baseline_socket_reader"


def start_pifus_bench(amount_sockets: int):
    framework.start_stack(affinity="0")

    pifus_reader_path = framework.get_binary_path(
        "api/benchmarks/baseline/reader/pifus_baseline_reader"
    )
    framework.start_process(pifus_reader_path, affinity="2")

    # make sure readers are up
    time.sleep(1)

    lwip_netconn_path = framework.get_binary_path(
        "custom/benchmark/netconn-writer-random/netconn_writer_random",
        benchmark_build=True,
    )
    framework.start_process(
        lwip_netconn_path,
        args=f"192.168.1.200 -a 192.168.1.201 -p 11337 -o {file_prefix}_pifus -c {amount_sockets}",
        tapif=1,
    )

    framework.wait()
    framework.kill_all_processes()


def start_lwip_bench(amount_sockets: int):
    lwip_reader_path = framework.get_binary_path("lwip/custom/benchmark/reader/reader")
    framework.start_process(
        lwip_reader_path, args="192.168.1.201", tapif=1, affinity="2"
    )

    # make sure readers are up
    time.sleep(1)

    lwip_netconn_path = framework.get_binary_path(
        "custom/benchmark/netconn-writer-random/netconn_writer_random",
        benchmark_build=True,
    )
    framework.start_process(
        lwip_netconn_path,
        args=f"192.168.1.201 -p 11337 -o {file_prefix} -c {amount_sockets}",
        tapif=0,
    )

    framework.wait()
    framework.kill_all_processes()


def measure():
    """X lwIP netconn writer (multiple sockets), one lwIP reader.
    X lwIP netconn writer (multiple sockets), one pifus reader."""
    amount_sockets = [1, 2, 4, 8, 16, 32, 64, 120]
    mean_data = []

    for current_amount_sockets in amount_sockets:
        start_pifus_bench(current_amount_sockets)
        start_lwip_bench(current_amount_sockets)

        pifus_data = []
        netconn_data = []
        for index in range(0, current_amount_sockets):
            pifus_tx_file = f"{file_prefix}_pifus_{index}_tx.txt"
            pifus_txed_file = f"{file_prefix}_pifus_{index}_txed.txt"

            pifus_sock_data = framework.ts_to_latency_time_tuple(
                pifus_tx_file, pifus_txed_file, "pifus", warmup_count=10
            )
            pifus_data += pifus_sock_data

            print(f"{len(pifus_sock_data)} from sock")

            netconn_tx_file = f"{file_prefix}_{index}_tx.txt"
            netconn_txed_file = f"{file_prefix}_{index}_txed.txt"
            netconn_data += framework.ts_to_latency_time_tuple(
                netconn_tx_file, netconn_txed_file, "netconn", warmup_count=10
            )

            # clean up for next runs
            framework.remove_measurement_file(pifus_tx_file)
            framework.remove_measurement_file(pifus_txed_file)
            framework.remove_measurement_file(netconn_tx_file)
            framework.remove_measurement_file(netconn_txed_file)

        pifus_df = plot.latency_dataframe(pifus_data)
        mean_data.append(
            [
                "pifus",
                current_amount_sockets,
                pifus_df["latency"].mean(),
                pifus_df["latency"].median(),
                pifus_df["latency"].min(),
                pifus_df["latency"].max(),
                pifus_df["latency"].quantile(0.75),
                pifus_df["latency"].quantile(0.25),
            ]
        )

        netconn_df = plot.latency_dataframe(netconn_data)
        mean_data.append(
            [
                "lwip",
                current_amount_sockets,
                netconn_df["latency"].mean(),
                netconn_df["latency"].median(),
                netconn_df["latency"].min(),
                netconn_df["latency"].max(),
                netconn_df["latency"].quantile(0.75),
                netconn_df["latency"].quantile(0.25),
            ]
        )

        # just to see the data in the run
        plot.print_latency_dataframe_stats(pifus_data)
        plot.print_latency_dataframe_stats(netconn_data)

    df = pd.DataFrame(
        mean_data,
        columns=[
            "Type",
            "Amount of sockets",
            "Mean",
            "Median",
            "Min",
            "Max",
            "75%",
            "25%",
        ],
    )
    df.to_csv(f"{framework.MEASUREMENT_FOLDER}/{file_prefix}.txt")
    print(df)


def draw_plots():
    data = pd.read_csv(os.path.join(framework.MEASUREMENT_FOLDER, f"{file_prefix}.txt"))
    plot.lineplot(
        data,
        output=f"{file_prefix}.png",
        legend_title="API",
        xlabel="Amount of sockets",
        ylabel="Mean latency [us]",
        latency_unit="us",
        with_quartiles=True,
    )
