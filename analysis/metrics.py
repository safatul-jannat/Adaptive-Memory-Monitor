"""Run-level metrics for Adaptive Memory Monitor experiment CSVs.

All functions operate on a pandas DataFrame produced from the monitor CSV
(one row per sampled observation). Epoch timestamps are stored as numeric
seconds and are handled numerically; pandas datetime conversion is never
needed and never applied to raw epoch integers.
"""

import math

import pandas as pd

HIGH_RISK_LEVELS = ("HIGH", "CRITICAL")

RISK_LEVEL_ORDER = ("NORMAL", "SUSPICIOUS", "HIGH", "CRITICAL")


def total_samples(df):
    """Return the total number of rows (samples) in the run."""
    return len(df)


def samples_per_second(df):
    """Return samples / elapsed wall time between first and last timestamp.

    Elapsed time is computed directly from numeric epoch timestamps.
    Returns NaN when the frame has fewer than 2 rows or the duration is
    not strictly positive.
    """
    if df is None or len(df) < 2:
        return float("nan")

    try:
        timestamps = df["timestamp"].astype("float64")
    except (KeyError, ValueError):
        return float("nan")

    first = timestamps.iloc[0]
    last = timestamps.iloc[len(timestamps) - 1]
    duration = last - first

    if not math.isfinite(duration) or duration <= 0.0:
        return float("nan")

    return len(df) / duration


def average_risk(df):
    """Return the mean risk_score for the run (NaN for empty data)."""
    if df is None or len(df) == 0:
        return float("nan")

    risks = pd.to_numeric(df["risk_score"], errors="coerce")
    return float(risks.mean())


def risk_distribution(df):
    """Return per-level counts for NORMAL/SUSPICIOUS/HIGH/CRITICAL.

    Missing levels are reported as zero counts.
    """
    counts = {
        level: int((df["risk_level"] == level).sum())
        for level in RISK_LEVEL_ORDER
    }
    return counts


def _first_high_timestamp(df):
    """Return the timestamp of the first HIGH/CRITICAL row, or None."""
    if df is None or len(df) == 0:
        return None

    high = df[df["risk_level"].isin(HIGH_RISK_LEVELS)]
    if high.empty:
        return None

    return high.iloc[0]["timestamp"]


def detection_latency(df, anomaly_start_epoch):
    """Return detection latency relative to the anomaly start epoch.

    The detection event is the first row whose risk_level is HIGH or
    CRITICAL. Latency is computed as:

        first_detection_timestamp - anomaly_start_epoch

    Never relative to the first CSV row. Returns None when the anomaly
    timestamp is missing or no HIGH/CRITICAL row exists.
    """
    if anomaly_start_epoch is None:
        return None

    try:
        anomaly_epoch = float(anomaly_start_epoch)
    except (TypeError, ValueError):
        return None

    if not math.isfinite(anomaly_epoch):
        return None

    first_high = _first_high_timestamp(df)
    if first_high is None:
        return None

    try:
        detection_epoch = float(first_high)
    except (TypeError, ValueError):
        return None

    return detection_epoch - anomaly_epoch


def run_detected_abnormal(df):
    """Return True if any row reached HIGH or CRITICAL risk."""
    if df is None or len(df) == 0:
        return False

    return bool((df["risk_level"].isin(HIGH_RISK_LEVELS)).any())