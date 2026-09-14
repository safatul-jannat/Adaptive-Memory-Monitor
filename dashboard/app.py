"""Minimal Flask dashboard for Adaptive Memory Monitor results.

Visualization only. It never reads /proc and never calculates risk or
sampling policy; the C monitor remains the source of all OS logic.

CSV source:
    - DASHBOARD_CSV env var if set
    - otherwise the most recent CSV under results/raw/

The CSV is re-read on every request so a running monitor that is actively
appending to the file is visualized live.

Run:
    DASHBOARD_CSV=/tmp/demo.csv python3 dashboard/app.py
    python3 dashboard/app.py
"""

import io
import os

import pandas as pd
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import dates as mdates

from flask import Flask, render_template, send_from_directory

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

RAW_DIR = os.path.join(BASE, "results", "raw")
TABLES_DIR = os.path.join(BASE, "results", "tables")
GRAPHS_DIR = os.path.join(BASE, "results", "graphs")

LATEST_FIELDS = (
    ("PID", "pid"),
    ("Process name", "process_name"),
    ("RSS (KB)", "rss_kb"),
    ("VMSize (KB)", "vmsize_kb"),
    ("Risk score", "risk_score"),
    ("Risk level", "risk_level"),
    ("Sampling interval (s)", "sampling_interval"),
    ("Strategy", "strategy"),
)

REQUIRED_PLOTS = (
    "samples_by_strategy.png",
    "detection_latency_by_strategy.png",
    "cpu_overhead_by_strategy.png",
    "sustained_growth_timeline.png",
)

app = Flask(__name__)


def resolve_csv_path():
    """Return the CSV to visualize or None."""
    configured = os.environ.get("DASHBOARD_CSV", "")

    if configured and os.path.isfile(configured):
        return configured

    if os.path.isdir(RAW_DIR):
        candidates = [
            os.path.join(RAW_DIR, name)
            for name in os.listdir(RAW_DIR)
            if name.endswith(".csv")
        ]

        if candidates:
            return max(candidates, key=os.path.getmtime)

    return None


def load_csv():
    """Return (dataframe, csv_path) or (None, None) if unavailable."""
    path = resolve_csv_path()

    if path is None:
        return None, None

    try:
        return pd.read_csv(path), path
    except Exception:  # noqa: BLE001 - file may be mid-write
        return None, None


def latest_record(df):
    """Return the latest sample as a list of (label, value) pairs."""
    if df is None or len(df) == 0:
        return []

    row = df.iloc[-1]
    return [(label, row.get(column)) for label, column in LATEST_FIELDS]


@app.route("/")
def index():
    df, path = load_csv()
    summary = None
    summary_path = os.path.join(TABLES_DIR, "strategy_summary.csv")

    if os.path.isfile(summary_path):
        try:
            summary = pd.read_csv(summary_path)
        except Exception:  # noqa: BLE001
            summary = None

    samples = len(df) if df is not None else 0

    return render_template(
        "index.html",
        csv_path=path,
        latest=latest_record(df),
        samples=samples,
        summary=summary,
        required_plots=REQUIRED_PLOTS,
        graphs_dir=GRAPHS_DIR,
    )


@app.route("/rss_timeline.png")
def rss_timeline():
    """Serve a fresh RSS-over-time plot generated from the current CSV."""
    df, path = load_csv()

    if df is None or len(df) < 2:
        return "No CSV data available", 404

    timestamps = pd.to_datetime(df["timestamp"], unit="s")
    rss_mb = df["rss_kb"] / 1024.0

    fig, ax = plt.subplots(figsize=(8, 4))
    ax.plot(timestamps, rss_mb, color="#4C72B0")
    ax.set_xlabel("Time (UTC)")
    ax.set_ylabel("RSS (MB)")
    ax.set_title(f"RSS timeline - {os.path.basename(path)}")
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))

    fig.tight_layout()

    buffer = io.BytesIO()
    fig.savefig(buffer, format="png", dpi=120)
    plt.close(fig)
    buffer.seek(0)

    return buffer.getvalue(), 200, {"Content-Type": "image/png"}


@app.route("/graphs/<path:filename>")
def graphs(filename):
    """Serve one of the final comparison plots."""
    return send_from_directory(GRAPHS_DIR, filename)


if __name__ == "__main__":
    url = "http://127.0.0.1:5000"
    print(f"Adaptive Memory Monitor dashboard: {url}")
    app.run(host="127.0.0.1", port=5000, debug=False, threaded=True)