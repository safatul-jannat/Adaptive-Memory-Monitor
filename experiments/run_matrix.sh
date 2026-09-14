#!/usr/bin/env bash
#
# run_matrix.sh — run the full final experiment matrix.
#
# Usage:
#   ./experiments/run_matrix.sh run01             # first 16
#   ./experiments/run_matrix.sh all               # run01..run03 (48)
#   ./experiments/run_matrix.sh run01 run02 run03 # explicit passes
#
# Strategies: fixed1 fixed5 fixed10 adaptive
# Workloads:  normal memory_intensive sustained_growth bursty

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
RUNNER="$ROOT/experiments/run_experiment.sh"

STRATEGIES=(fixed1 fixed5 fixed10 adaptive)
WORKLOADS=(normal memory_intensive sustained_growth bursty)

# Map pass names (run01/run02/run03) to two-digit run labels (01/02/03).
RUN_LABELS=(01 02 03)

PASSES=()
if [ "$#" -eq 0 ] || [ "$1" = "run01" ]; then
    PASSES=(run01)
elif [ "$1" = "all" ]; then
    PASSES=(run01 run02 run03)
else
    PASSES=("$@")
fi

TOTAL=0
for pass in "${PASSES[@]}"; do
    run=""
    for i in "${!RUN_LABELS[@]}"; do
        if [ "$pass" = "run-${RUN_LABELS[$i]}" ] || [ "$pass" = "run${RUN_LABELS[$i]}" ]; then
            run="${RUN_LABELS[$i]}"
        fi
    done

    if [ -z "$run" ]; then
        echo "ERROR: unknown matrix pass '$pass' (expected run01, run02, run03)" >&2
        exit 2
    fi

    for strategy in "${STRATEGIES[@]}"; do
        for workload in "${WORKLOADS[@]}"; do
            TOTAL=$((TOTAL + 1))
            echo "=== RUNNING ${strategy} ${workload} run${run} ($TOTAL) ==="
            if ! "$RUNNER" "$strategy" "$workload" "$run"; then
                echo "!!! FAILED: ${strategy} ${workload} run${run}" >&2
            fi
        done
    done
    echo "=== MATRIX PASS run${run} COMPLETED ==="
done

echo "=== MATRIX TOTAL: $TOTAL RUNS TRIGGERED ==="