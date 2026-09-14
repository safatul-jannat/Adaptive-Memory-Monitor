"""Run-level inspection tool for a single experiment CSV.

Usage:
    python3 analysis/analyze_results.py <results/raw/<id>.csv> [anomaly_start_epoch]

Prints total samples, samples per second, average risk, risk distribution,
whether HIGH/CRITICAL ever occurred, and (given an anomaly start epoch) the
detection latency relative to that epoch.
"""

import sys

import pandas as pd

from metrics import (
    average_risk,
    detection_latency,
    risk_distribution,
    run_detected_abnormal,
    samples_per_second,
    total_samples,
)


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analysis/analyze_results.py <csv_path> [anomaly_start_epoch]")
        return 1

    csv_path = sys.argv[1]
    anomaly_start_epoch = sys.argv[2] if len(sys.argv) > 2 else None

    df = pd.read_csv(csv_path)

    print("=" * 44)
    print("Memory Monitor Run Analysis")
    print(f"file: {csv_path}")
    print("=" * 44)

    print(f"rows / total samples : {total_samples(df)}")
    print(f"samples per second   : {samples_per_second(df)}")
    print(f"average risk         : {average_risk(df)}")

    print("risk distribution:")
    distribution = risk_distribution(df)
    for level, count in distribution.items():
        print(f"  {level:<12}: {count}")

    detected = run_detected_abnormal(df)
    print(f"HIGH/CRITICAL reached: {detected}")

    if anomaly_start_epoch is not None:
        print(
            "detection latency (vs anomaly start): "
            f"{detection_latency(df, anomaly_start_epoch)} s"
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())