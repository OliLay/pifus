import os
from pathlib import Path
from typing import List, Optional

import pandas as pd # type: ignore[import]
import seaborn as sns # type: ignore[import]
import matplotlib.pyplot as plt # type: ignore[import]

from framework.framework import LatencyTimeTuple, RESULTS_FOLDER

sns.set_theme()
pd.options.display.float_format = '{:.1f}'.format
Path(RESULTS_FOLDER).mkdir(parents=True, exist_ok=True)

def set_legend(ax, legend_info: Optional[str]):
    if legend_info:
        ax.legend(title=legend_info)

def plotify_path(path: str) -> str:
    return os.path.join(RESULTS_FOLDER, path)

def latency_dataframe(data: List[LatencyTimeTuple]) -> pd.DataFrame:
    return pd.DataFrame([vars(data_point) for data_point in data])


def latency_scatter(data: List[LatencyTimeTuple], output: str, legend_title: Optional[str] = None,
                    xlabel: Optional[str] = None, ylabel: Optional[str] = None):
    df = latency_dataframe(data)

    plt.figure(figsize=(12, 5))

    # in the plot, show latency in us -> ms
    df['latency'] = df['latency'].div(1000).round(2)

    # in the plot, show time in us -> s
    df['time'] = df['time'].div(1000000).round(2)

    ax = sns.scatterplot(data=df, x="time", y="latency", hue="type",
                         legend='full', s=10, edgecolor="none")
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    set_legend(ax, legend_title)
    plt.savefig(plotify_path(output), bbox_inches='tight')


def latency_dataframe_stats(data: List[LatencyTimeTuple], output: str):
    df = latency_dataframe(data)

    with open(plotify_path(output), "w") as output_file:
        print(df.groupby('type')["latency"].describe().unstack(
            1), file=output_file)
