"""Build run-level results and strategy summaries from final experiment artifacts.

Automatically discovers experiments under results/raw, results/perf and
results/meta by matching experiment IDs of the form:
    <strategy>_<workload>_runXX

Produces:
    results/tables/run_results.csv
    results/tables/strategy_summary.csv
"""

import os
import re
import sys

import pandas as pd

from metrics import (
    detection_latency,
    run_detected_abnormal,
    samples_per_second,
    total_samples,
)

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

RAW_DIR = os.path.join(BASE, "results", "raw")
PERF_DIR = os.path.join(BASE, "results", "perf")
META_DIR = os.path.join(BASE, "results", "meta")
TABLES_DIR = os.path.join(BASE, "results", "tables")

STRATEGIES = ("fixed1", "fixed5", "fixed10", "adaptive")
WORKLOADS = ("normal", "memory_intensive", "sustained_growth", "bursty")

EXPERIMENT_RE = re.compile(
    r"^(?P<strategy>fixed1|fixed5|fixed10|adaptive)_"
    r"(?P<workload>normal|memory_intensive|sustained_growth|bursty)_"
    r"run(?P<run>\d{2})$"
)

GROUND_TRUTH = {
    "normal": "negative",
    "memory_intensive": "negative",
    "sustained_growth": "positive",
    "bursty": "negative",
}


def parse_perf_file(path):
    """Parse a GNU time output file into a dictionary of fields."""
    fields = {}

    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            line = line.strip()

            if line.startswith("Percent of CPU this job got:"):
                value = line.split(":", 1)[1].strip()
                fields["cpu_percent"] = float(
                    value.replace("%", "").strip()
                )

            elif line.startswith("Maximum resident set size (kbytes):"):
                value = line.split(":", 1)[1].strip()
                fields["monitor_max_rss_kb"] = float(value)

            elif line.startswith("User time (seconds):"):
                value = line.split(":", 1)[1].strip()
                fields["user_time_sec"] = float(value)

            elif line.startswith("System time (seconds):"):
                value = line.split(":", 1)[1].strip()
                fields["system_time_sec"] = float(value)

            elif line.startswith("Elapsed (wall clock) time"):
                # GNU time format:
                # Elapsed (wall clock) time (h:mm:ss or m:ss): 1:59.56
                value = line.rsplit(":", 1)[1].strip()

                # rsplit above only captures the seconds part because
                # the format description itself contains ':'.
                # Instead extract the value after '): '.
                marker = "):"
                if marker in line:
                    value = line.split(marker, 1)[1].strip()

                fields["elapsed_wall_sec"] = parse_wallclock(value)

    return fields


def parse_wallclock(value):
    """Parse GNU time elapsed formats such as m:ss or h:mm:ss."""
    value = value.strip().strip("()")
    parts = [float(part) for part in value.split(":")]

    if len(parts) == 3:
        return parts[0] * 3600 + parts[1] * 60 + parts[2]

    if len(parts) == 2:
        return parts[0] * 60 + parts[1]

    return float("nan")


def parse_meta_file(path):
    """Parse an experiment metadata file into a dictionary."""
    fields = {}

    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            line = line.strip()

            if "=" in line:
                key, _, value = line.partition("=")
                fields[key.strip()] = value.strip()

    return fields


def experiment_ids(experiment_id):
    """Return the (strategy, workload, run) triple for an experiment ID."""
    match = EXPERIMENT_RE.match(experiment_id)

    if match is None:
        return None

    return (
        match.group("strategy"),
        match.group("workload"),
        match.group("run"),
    )


def build_run_records():
    """Discover and parse every valid run into one record per run."""
    records = []
    processed = 0

    if not os.path.isdir(RAW_DIR):
        return processed, records

    for filename in sorted(os.listdir(RAW_DIR)):
        if not filename.endswith(".csv"):
            continue

        experiment_id = filename[:-len(".csv")]
        parts = experiment_ids(experiment_id)

        if parts is None:
            continue

        strategy, workload, run = parts
        processed += 1

        csv_path = os.path.join(RAW_DIR, filename)
        perf_path = os.path.join(
            PERF_DIR,
            f"{experiment_id}_time.txt",
        )
        meta_path = os.path.join(
            META_DIR,
            f"{experiment_id}_meta.txt",
        )

        if not (
            os.path.isfile(perf_path)
            and os.path.isfile(meta_path)
        ):
            print(
                f"WARN: incomplete artifact set for "
                f"{experiment_id}; skipped"
            )
            continue

        try:
            df = pd.read_csv(csv_path)
        except Exception as error:  # noqa: BLE001
            print(
                f"WARN: could not read {csv_path}: "
                f"{error}; skipped"
            )
            continue

        perf = parse_perf_file(perf_path)
        meta = parse_meta_file(meta_path)

        ground_truth = GROUND_TRUTH.get(
            workload,
            meta.get("ground_truth", ""),
        )

        detected_high = run_detected_abnormal(df)

        latency = float("nan")

        if workload == "sustained_growth" and detected_high:
            anomaly = meta.get("anomaly_start_epoch", "")

            if anomaly.isdigit():
                latency = detection_latency(df, anomaly)

        user_time_sec = perf.get(
            "user_time_sec",
            float("nan"),
        )

        system_time_sec = perf.get(
            "system_time_sec",
            float("nan"),
        )

        # Main CPU-overhead metric.
        #
        # GNU Time reports CPU percentage as an integer, so very
        # lightweight runs often appear as 0%. User + system CPU
        # seconds preserves the measurable CPU work performed.
        cpu_time_sec = user_time_sec + system_time_sec

        records.append(
            {
                "strategy": strategy,
                "workload": workload,
                "run": run,
                "samples": total_samples(df),
                "samples_per_sec": samples_per_second(df),
                "cpu_percent": perf.get(
                    "cpu_percent",
                    float("nan"),
                ),
                "cpu_time_sec": cpu_time_sec,
                "monitor_max_rss_kb": perf.get(
                    "monitor_max_rss_kb",
                    float("nan"),
                ),
                "user_time_sec": user_time_sec,
                "system_time_sec": system_time_sec,
                "elapsed_wall_sec": perf.get(
                    "elapsed_wall_sec",
                    float("nan"),
                ),
                "detection_latency_sec": latency,
                "detected_high": detected_high,
                "ground_truth": ground_truth,
                "experiment_id": experiment_id,
            }
        )

    return processed, records


def count_confusion(predictions, truths):
    """Return (TP, TN, FP, FN) for two boolean lists."""
    tp = sum(
        p and t
        for p, t in zip(predictions, truths)
    )

    tn = sum(
        (not p) and (not t)
        for p, t in zip(predictions, truths)
    )

    fp = sum(
        p and (not t)
        for p, t in zip(predictions, truths)
    )

    fn = sum(
        (not p) and t
        for p, t in zip(predictions, truths)
    )

    return tp, tn, fp, fn


def precision_recall_f1(tp, fp, fn):
    """Return precision, recall and F1 with safe denominators."""
    precision = (
        tp / (tp + fp)
        if (tp + fp) > 0
        else 0.0
    )

    recall = (
        tp / (tp + fn)
        if (tp + fn) > 0
        else 0.0
    )

    f1 = (
        2 * precision * recall / (precision + recall)
        if (precision + recall) > 0
        else 0.0
    )

    return precision, recall, f1


def build_strategy_summary(run_records):
    """Aggregate run records into exactly one row per strategy."""
    records = pd.DataFrame(run_records)

    if records.empty:
        return pd.DataFrame()

    rows = []

    for strategy in STRATEGIES:
        subset = records[
            records["strategy"] == strategy
        ]

        if subset.empty:
            continue

        predictions = subset[
            "detected_high"
        ].tolist()

        truths = (
            subset["ground_truth"] == "positive"
        ).tolist()

        tp, tn, fp, fn = count_confusion(
            predictions,
            truths,
        )

        precision, recall, f1 = precision_recall_f1(
            tp,
            fp,
            fn,
        )

        sustained_detected = subset[
            (subset["workload"] == "sustained_growth")
            & subset["detection_latency_sec"].notna()
        ]

        mean_latency = (
            sustained_detected[
                "detection_latency_sec"
            ].mean()
            if not sustained_detected.empty
            else float("nan")
        )

        rows.append(
            {
                "strategy": strategy,

                "mean_samples":
                    subset["samples"].mean(),

                "mean_samples_per_sec":
                    subset["samples_per_sec"].mean(),

                # Keep GNU Time's rounded CPU percentage
                # for reference.
                "mean_cpu_percent":
                    subset["cpu_percent"].mean(),

                # Use this as the primary CPU-overhead metric.
                "mean_cpu_time_sec":
                    subset["cpu_time_sec"].mean(),

                "mean_monitor_max_rss_kb":
                    subset[
                        "monitor_max_rss_kb"
                    ].mean(),

                "mean_sustained_detection_latency_sec":
                    mean_latency,

                "TP": tp,
                "TN": tn,
                "FP": fp,
                "FN": fn,
                "precision": precision,
                "recall": recall,
                "f1": f1,
            }
        )

    return pd.DataFrame(rows)


def main():
    processed, records = build_run_records()

    if not records:
        print("ERROR: no valid experiment records found")
        sys.exit(1)

    os.makedirs(TABLES_DIR, exist_ok=True)

    run_results_path = os.path.join(
        TABLES_DIR,
        "run_results.csv",
    )

    strategy_summary_path = os.path.join(
        TABLES_DIR,
        "strategy_summary.csv",
    )

    run_results = pd.DataFrame(records)

    run_results.to_csv(
        run_results_path,
        index=False,
    )

    strategy_summary = build_strategy_summary(
        records
    )

    strategy_summary.to_csv(
        strategy_summary_path,
        index=False,
    )

    print("=" * 60)
    print(
        "Adaptive Memory Monitor - Strategy Comparison"
    )
    print("=" * 60)

    print(
        f"Run files processed: {processed}"
    )

    print(
        f"Valid run records  : {len(run_results)}"
    )

    print(
        f"run_results.csv    : {run_results_path}"
    )

    print(
        f"strategy_summary.csv: "
        f"{strategy_summary_path}"
    )

    print()
    print(
        "Strategy summary (means across repetitions):"
    )

    print(
        strategy_summary.to_string(index=False)
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())