"""Generate the final experiment plots from real result tables.

Produces:
    results/graphs/samples_by_strategy.png
    results/graphs/detection_latency_by_strategy.png
    results/graphs/cpu_overhead_by_strategy.png
    results/graphs/sustained_growth_timeline.png
"""

import os

import matplotlib.pyplot as plt
import pandas as pd

from matplotlib import dates as mdates

BASE = os.path.dirname(
    os.path.dirname(os.path.abspath(__file__))
)

TABLES_DIR = os.path.join(
    BASE,
    "results",
    "tables",
)

GRAPHS_DIR = os.path.join(
    BASE,
    "results",
    "graphs",
)

RAW_DIR = os.path.join(
    BASE,
    "results",
    "raw",
)

STRATEGY_ORDER = [
    "fixed1",
    "fixed5",
    "fixed10",
    "adaptive",
]

PLOT_KWARGS = {
    "dpi": 150,
    "bbox_inches": "tight",
}


def save_figure(fig, filename):
    os.makedirs(
        GRAPHS_DIR,
        exist_ok=True,
    )

    path = os.path.join(
        GRAPHS_DIR,
        filename,
    )

    fig.savefig(
        path,
        **PLOT_KWARGS,
    )

    plt.close(fig)

    print(path)


def bar_chart(
    series,
    title,
    ylabel,
    filename,
):
    fig, ax = plt.subplots(
        figsize=(8, 5)
    )

    ax.bar(
        series.index,
        series.values,
    )

    ax.set_title(title)
    ax.set_xlabel("Strategy")
    ax.set_ylabel(ylabel)

    ax.set_xticks(
        range(len(series.index))
    )

    ax.set_xticklabels(
        series.index
    )

    for i, value in enumerate(
        series.values
    ):
        ax.text(
            i,
            value,
            f"{value:.3g}",
            ha="center",
            va="bottom",
        )

    fig.tight_layout()

    save_figure(
        fig,
        filename,
    )


def samples_by_strategy(summary):
    series = (
        summary
        .set_index("strategy")
        .loc[STRATEGY_ORDER][
            "mean_samples"
        ]
    )

    bar_chart(
        series.dropna(),
        "Mean samples per run by strategy",
        "Mean samples per run",
        "samples_by_strategy.png",
    )


def detection_latency_by_strategy(summary):
    column = (
        "mean_sustained_detection_latency_sec"
    )

    values = (
        summary
        .set_index("strategy")
        .loc[STRATEGY_ORDER][column]
    )

    fig, ax = plt.subplots(
        figsize=(8, 5)
    )

    present = values.dropna()

    missing = values[
        values.isna()
    ].index.tolist()

    ax.bar(
        present.index,
        present.values,
    )

    if missing:
        ax.axhline(
            0,
            linewidth=0.8,
        )

    ax.set_title(
        "Mean sustained-growth detection latency "
        "by strategy"
    )

    ax.set_xlabel("Strategy")

    ax.set_ylabel(
        "Detection latency (seconds)"
    )

    if missing:
        ax.set_ylim(bottom=0)

    for i, value in enumerate(
        present.values
    ):
        ax.text(
            i,
            value,
            f"{value:.1f}",
            ha="center",
            va="bottom",
        )

    fig.tight_layout()

    save_figure(
        fig,
        "detection_latency_by_strategy.png",
    )


def cpu_overhead_by_strategy(summary):
    """
    Plot CPU work using user + system CPU seconds.

    GNU Time reports CPU percentage as an integer and
    therefore rounds extremely lightweight monitor runs
    to 0%. CPU time preserves the measurable difference.
    """
    column = "mean_cpu_time_sec"

    if column not in summary.columns:
        raise ValueError(
            "mean_cpu_time_sec is missing from "
            "strategy_summary.csv. Run "
            "analysis/compare_strategies.py first."
        )

    series = (
        summary
        .set_index("strategy")
        .loc[STRATEGY_ORDER][column]
    )

    bar_chart(
        series.dropna(),
        "Mean monitor CPU time by strategy",
        "Mean monitor CPU time (seconds)",
        "cpu_overhead_by_strategy.png",
    )


def sustained_growth_timeline(
    preferred_id,
):
    path = os.path.join(
        RAW_DIR,
        f"{preferred_id}.csv",
    )

    if not os.path.isfile(path):
        matches = sorted(
            filename
            for filename in os.listdir(
                RAW_DIR
            )
            if filename.startswith(
                "adaptive_sustained_growth_run"
            )
            and filename.endswith(".csv")
        )

        if not matches:
            print(
                "ERROR: no adaptive sustained-growth "
                f"CSV found in {RAW_DIR}"
            )
            return

        path = os.path.join(
            RAW_DIR,
            matches[0],
        )

    df = pd.read_csv(path)

    timestamps = pd.to_datetime(
        df["timestamp"],
        unit="s",
    )

    rss_mb = (
        df["rss_kb"] / 1024.0
    )

    risk = df["risk_score"]

    fig, ax = plt.subplots(
        figsize=(10, 5)
    )

    ax.plot(
        timestamps,
        rss_mb,
        label="RSS (MB)",
    )

    ax.set_xlabel("Time")

    ax.set_ylabel("RSS (MB)")

    ax2 = ax.twinx()

    ax2.plot(
        timestamps,
        risk,
        alpha=0.8,
        label="Risk score",
        linewidth=1.5,
    )

    ax2.set_ylabel(
        "Risk score"
    )

    ax2.set_ylim(
        0,
        100,
    )

    ax.xaxis.set_major_formatter(
        mdates.DateFormatter(
            "%H:%M:%S"
        )
    )

    lines_1, labels_1 = (
        ax.get_legend_handles_labels()
    )

    lines_2, labels_2 = (
        ax2.get_legend_handles_labels()
    )

    ax.legend(
        lines_1 + lines_2,
        labels_1 + labels_2,
        loc="upper left",
    )

    ax.set_title(
        "Adaptive sustained-growth workload: "
        "RSS and risk over time\n"
        f"({os.path.basename(path)})"
    )

    fig.tight_layout()

    save_figure(
        fig,
        "sustained_growth_timeline.png",
    )


def main():
    summary_path = os.path.join(
        TABLES_DIR,
        "strategy_summary.csv",
    )

    if not os.path.isfile(
        summary_path
    ):
        print(
            f"ERROR: {summary_path} not found; "
            "run compare_strategies.py first"
        )
        return 1

    summary = pd.read_csv(
        summary_path
    )

    samples_by_strategy(
        summary
    )

    detection_latency_by_strategy(
        summary
    )

    cpu_overhead_by_strategy(
        summary
    )

    sustained_growth_timeline(
        "adaptive_sustained_growth_run01"
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())