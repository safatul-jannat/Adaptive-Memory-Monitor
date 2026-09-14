#!/usr/bin/env bash
#
# run_experiment.sh — run one Adaptive Memory Monitor experiment.
#
# Usage:
#   ./experiments/run_experiment.sh <strategy> <workload> <run_number>
#
# Example:
#   ./experiments/run_experiment.sh adaptive sustained_growth 01
#
# Strategies: fixed1 fixed5 fixed10 adaptive
# Workloads:  normal memory_intensive sustained_growth bursty
#
# Produces, for experiment id <strategy>_<workload>_runXX:
#   results/raw/<id>.csv
#   results/perf/<id>_time.txt
#   results/meta/<id>_meta.txt
#   results/meta/<id>_workload.log

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
cd "$ROOT" || exit 1

# ---------------------------------------------------------------- arguments

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <strategy> <workload> <run_number>" >&2
    echo "  strategies: fixed1 fixed5 fixed10 adaptive" >&2
    echo "  workloads:  normal memory_intensive sustained_growth bursty" >&2
    echo "  run_number: two digits, e.g. 01 02 03" >&2
    exit 1
fi

STRATEGY="$1"
WORKLOAD="$2"
RUN="$3"

case "$STRATEGY" in
    fixed1|fixed5|fixed10|adaptive) ;;
    *)
        echo "ERROR: invalid strategy '$STRATEGY'" >&2
        echo "Allowed: fixed1 fixed5 fixed10 adaptive" >&2
        exit 2
        ;;
esac

case "$WORKLOAD" in
    normal|memory_intensive|sustained_growth|bursty) ;;
    *)
        echo "ERROR: invalid workload '$WORKLOAD'" >&2
        echo "Allowed: normal memory_intensive sustained_growth bursty" >&2
        exit 2
        ;;
esac

if ! [[ "$RUN" =~ ^[0-9]{2}$ ]]; then
    echo "ERROR: invalid run number '$RUN' (expected two digits, e.g. 01)" >&2
    exit 2
fi

# ------------------------------------------------------- workload durations

case "$WORKLOAD" in
    normal)           DURATION=60 ;;
    memory_intensive) DURATION=65 ;;
    sustained_growth) DURATION=120 ;;
    bursty)           DURATION=82 ;;
esac

case "$WORKLOAD" in
    sustained_growth) GROUND_TRUTH="positive" ;;
    *)                GROUND_TRUTH="negative" ;;
esac

# --------------------------------------------------------------- GNU time

GNU_TIME=""
if [ -x /usr/bin/time ]; then
    GNU_TIME=/usr/bin/time
elif [ -n "${GNUTIME_BIN:-}" ] && [ -x "$GNUTIME_BIN" ]; then
    GNU_TIME="$GNUTIME_BIN"
elif [ -x "$ROOT/experiments/tools/time" ]; then
    GNU_TIME="$ROOT/experiments/tools/time"
fi

if [ -z "$GNU_TIME" ]; then
    echo "ERROR: GNU time (/usr/bin/time) not found." >&2
    echo "Install it (e.g. 'apt-get install time') or point GNUTIME_BIN at a GNU time binary." >&2
    exit 2
fi

if ! "$GNU_TIME" --version 2>/dev/null | grep -q "GNU Time"; then
    echo "ERROR: $GNU_TIME is not GNU time; refusing to use shell builtin time." >&2
    exit 2
fi

# -------------------------------------------------------------- experiment

EXPERIMENT_ID="${STRATEGY}_${WORKLOAD}_run${RUN}"

mkdir -p results/raw results/perf results/meta

WORKLOAD_BIN="$ROOT/workloads/bin/$WORKLOAD"
MONITOR_BIN="$ROOT/monitor"

if [ ! -x "$WORKLOAD_BIN" ]; then
    echo "ERROR: workload executable missing: $WORKLOAD_BIN (run 'make workloads')" >&2
    exit 2
fi

if [ ! -x "$MONITOR_BIN" ]; then
    echo "ERROR: monitor binary missing: $MONITOR_BIN (run 'make')" >&2
    exit 2
fi

CSV_FILE="$ROOT/results/raw/${EXPERIMENT_ID}.csv"
PERF_FILE="$ROOT/results/perf/${EXPERIMENT_ID}_time.txt"
META_FILE="$ROOT/results/meta/${EXPERIMENT_ID}_meta.txt"
WORKLOAD_LOG="$ROOT/results/meta/${EXPERIMENT_ID}_workload.log"

# 1 + 2: start the workload in the background, logging to the workload log.
"$WORKLOAD_BIN" > "$WORKLOAD_LOG" 2>&1 &
WORKLOAD_PID=$!

# 3: the PID is captured immediately; no extra commands between launch and capture above.

# 4: required 1-second delay so the shell is replaced by the workload before sampling.
sleep 1

# 5: run the monitor under GNU time.
"$GNU_TIME" -v -o "$PERF_FILE" \
    "$MONITOR_BIN" --pid "$WORKLOAD_PID" --strategy "$STRATEGY" \
    --duration "$DURATION" --output "$CSV_FILE"
MONITOR_RC=$?

# 6: monitor exit status preserved in MONITOR_RC.

# 7: terminate the workload if it is still running (its natural runtime may
#    exceed the monitor duration), then wait for it.
if kill -0 "$WORKLOAD_PID" 2>/dev/null; then
    kill "$WORKLOAD_PID" 2>/dev/null || true
fi
wait "$WORKLOAD_PID" 2>/dev/null || true

# 8: sustained_growth must provide an exact anomaly-start timestamp.
ANOMALY_START_EPOCH=""
EXPERIMENT_FAILED=0

if [ "$WORKLOAD" = "sustained_growth" ]; then
    ANOMALY_START_EPOCH="$(
        grep -oE '^ANOMALY_START_EPOCH=[0-9]+' "$WORKLOAD_LOG" \
            | head -n 1 | cut -d= -f2
    )"

    if [ -z "$ANOMALY_START_EPOCH" ]; then
        EXPERIMENT_FAILED=1
    fi
fi

# 10: write metadata.
{
    echo "experiment_id=${EXPERIMENT_ID}"
    echo "strategy=${STRATEGY}"
    echo "workload=${WORKLOAD}"
    echo "run=${RUN}"
    echo "ground_truth=${GROUND_TRUTH}"
    echo "monitor_duration_sec=${DURATION}"
    echo "workload_pid=${WORKLOAD_PID}"
    echo "anomaly_start_epoch=${ANOMALY_START_EPOCH}"
    echo "monitor_exit_status=${MONITOR_RC}"
} > "$META_FILE"

# 11: completion message.
if [ "$EXPERIMENT_FAILED" -eq 1 ]; then
    echo "EXPERIMENT FAILED: ${EXPERIMENT_ID} (sustained_growth produced no ANOMALY_START_EPOCH)" >&2
    echo "  experiment_id : ${EXPERIMENT_ID}" >&2
    echo "  csv           : ${CSV_FILE}" >&2
    echo "  time          : ${PERF_FILE}" >&2
    echo "  meta          : ${META_FILE}" >&2
    echo "  workload log  : ${WORKLOAD_LOG}" >&2
    echo "  monitor exit  : ${MONITOR_RC}" >&2
    exit 3
fi

echo "EXPERIMENT COMPLETE: ${EXPERIMENT_ID}"
echo "  experiment_id : ${EXPERIMENT_ID}"
echo "  csv           : ${CSV_FILE}"
echo "  time          : ${PERF_FILE}"
echo "  meta          : ${META_FILE}"
echo "  workload log  : ${WORKLOAD_LOG}"
echo "  monitor exit  : ${MONITOR_RC}"

exit 0