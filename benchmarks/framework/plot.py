import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from framework.framework import LatencyTimeTuple
from typing import List


def load_latency_data(file: str) -> pd.DataFrame:
    return index(pd.read_csv(file, names=["latency"]))


def index(df: pd.DataFrame) -> pd.DataFrame:
    df['index'] = range(1, len(df) + 1)
    return df


def latency_scatter(data: List[LatencyTimeTuple], output: str):
    df = pd.DataFrame([vars(data_point) for data_point in data])
    print (df)

    fig, ax = plt.subplots(figsize=(15,5))
    marker_size = 8

    sns.scatterplot(data=df, x="time", y="latency", hue="type", legend=True, s=marker_size)
    #ax.set_title('dbus')
   # ax.set_yscale('log')

   # ticks = [50, 100, 200, 400, 800, 1600]
   # plt.yticks(ticks, ticks)

    plt.savefig(output, bbox_inches='tight')
    plt.show()
