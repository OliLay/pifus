import os
from pathlib import Path
from typing import List, Optional, Tuple

import pandas as pd  # type: ignore[import]
import seaborn as sns  # type: ignore[import]
import matplotlib.pyplot as plt  # type: ignore[import]

from framework.framework import LatencyTimeTuple, RESULTS_FOLDER

sns.set_theme()
pd.options.display.float_format = "{:.1f}".format
Path(RESULTS_FOLDER).mkdir(parents=True, exist_ok=True)


def set_legend(ax, legend_info: Optional[str]):
    if legend_info:
        ax.legend(title=legend_info)


def plotify_path(path: str) -> str:
    return os.path.join(RESULTS_FOLDER, path)


def latency_dataframe(data: List[LatencyTimeTuple]) -> pd.DataFrame:
    return pd.DataFrame([vars(data_point) for data_point in data])


def lineplot(
    df: pd.DataFrame,
    output: str,
    latency_unit: str = "ms",
    legend_title: Optional[str] = None,
    xlabel: Optional[str] = None,
    ylabel: Optional[str] = None,
    quartile_types: Optional[Tuple[str, str]] = None,
):
    plt.figure(figsize=(12, 5))

    if latency_unit == "ms":
        # in the plot, show mean in us -> ms
        df["Mean"] = df["Mean"].div(1000).round(2)
        df["25%"] = df["25%"].div(1000).round(2)
        df["75%"] = df["75%"].div(1000).round(2)

    ax = sns.lineplot(
        data=df,
        x="Amount of sockets",
        y="Mean",
        hue="Type",
        style="Type",
        markers=True,
        dashes=False,
    )
    if quartile_types is not None:
        ax.fill_between(
            data=df.loc[df["Type"] == quartile_types[0]],
            x="Amount of sockets",
            y1="25%",
            y2="75%",
            color="blue",
            alpha=0.3,
        )
        ax.fill_between(
            data=df.loc[df["Type"] == quartile_types[1]],
            x="Amount of sockets",
            y1="25%",
            y2="75%",
            color="orange",
            alpha=0.3,
        )

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    ax.set(xscale="log")
    ticks = [1, 2, 4, 8, 16, 32, 64, 120]
    plt.xticks(ticks, ticks)

    set_legend(ax, legend_title)
    plt.legend(loc="upper left")
    plt.savefig(f"{plotify_path(output)}.png", bbox_inches="tight")
    plt.savefig(f"{plotify_path(output)}.pgf", bbox_inches="tight")


def latency_scatter(
    data: List[LatencyTimeTuple],
    output: str,
    legend_title: Optional[str] = None,
    xlabel: Optional[str] = None,
    ylabel: Optional[str] = None,
    latency_unit: str = "ms",
    latency_scale: str = "linear",
):
    df = latency_dataframe(data)

    plt.figure(figsize=(12, 5))

    if latency_unit == "ms":
        # in the plot, show latency in us -> ms
        df["latency"] = df["latency"].div(1000).round(2)

    # in the plot, show time in us -> s
    df["time"] = df["time"].div(1000000).round(2)

    ax = sns.scatterplot(
        data=df,
        x="time",
        y="latency",
        hue="type",
        legend="full",
        s=5,
        edgecolor="none",
        alpha=0.8,
    )
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    if latency_scale == "log":
        ax.set(yscale="log")

    set_legend(ax, legend_title)
    plt.legend(loc="upper right")

    plt.savefig(f"{plotify_path(output)}.png", bbox_inches="tight")
    plt.savefig(f"{plotify_path(output)}.pgf", bbox_inches="tight")


def print_latency_dataframe_stats(data: List[LatencyTimeTuple]):
    df = latency_dataframe(data)

    result = df.groupby("type")["latency"].describe().unstack(1)
    print(result)


def latency_dataframe_stats(data: List[LatencyTimeTuple], output: str):
    df = latency_dataframe(data)

    result = df.groupby("type")["latency"].describe().unstack(1)
    print(result)
    with open(plotify_path(output), "w") as output_file:
        print(result, file=output_file)
