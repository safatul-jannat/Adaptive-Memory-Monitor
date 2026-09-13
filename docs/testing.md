# Testing and Quality Assurance

## 1. Testing Objective

The objective of testing is to verify that the Adaptive Memory Monitor can distinguish between:

- Normal memory behavior
- Temporary high memory usage
- Sustained memory growth
- Bursty memory usage

Four controlled workload programs were created:

1. normal.c
2. memory_intensive.c
3. sustained_growth.c
4. bursty.c

All workload behavior was verified using Linux /proc/<PID>/status and actual RSS measurements.

---

## 2. Workload Testing

| Workload | Ground Truth | Expected Behavior | Observed Behavior | Status |
|---|---|---|---|---|
| normal | Normal | Stable memory usage | RSS remained stable | PASS |
| memory_intensive | Non-abnormal / Heavy Releasing | High memory followed by release | Approximately 200 MB allocated and released | PASS |
| sustained_growth | Abnormal | Continuous memory growth | RSS increased continuously | PASS |
| bursty | Non-abnormal / Bursty | Repeated memory spikes and releases | RSS increased and decreased repeatedly | PASS |

---

## 3. Normal Workload

The normal.c workload allocates approximately 10 MB and keeps the allocation stable.

Expected behavior:

- Low memory usage
- Stable RSS
- No continuous growth

Result: PASS

---

## 4. Memory Intensive Workload

The memory_intensive.c workload uses three phases:

1. Low-memory pre-phase
2. Approximately 200 MB allocation and hold
3. Memory release followed by a low-memory phase

Expected behavior:

- RSS increases significantly during the allocation phase.
- RSS decreases after memory release.
- The process remains alive after release.

Result: PASS

---

## 5. Sustained Growth Workload

The sustained_growth.c workload first remains stable and then continuously allocates additional memory.

Expected behavior:

- Stable memory usage during the initial phase
- Continuous RSS increase after abnormal growth begins
- The workload should eventually be classified as abnormal/high risk by the monitor

Result: PASS

---

## 6. Bursty Workload

The bursty.c workload contains:

- 15-second baseline phase
- Three burst cycles
- 100 MB allocation during each burst
- 5-second hold period
- Memory release after each burst
- 10-second low-memory gap between bursts

Observed RSS behavior:

Baseline: approximately 2004 kB

Burst 1: approximately 104404 kB
Release: approximately 2004 kB

Burst 2: approximately 104404 kB
Release: approximately 2004 kB

Burst 3: approximately 104404 kB
Release: approximately 2004 kB

Observed pattern:

LOW -> HIGH -> LOW -> HIGH -> LOW -> HIGH -> LOW

Therefore, the bursty workload successfully demonstrated temporary memory spikes rather than sustained growth.

Result: PASS

---

## 7. Compilation Test

All four workload programs were compiled successfully using GCC:

```bash
mkdir -p workloads/bin

gcc -Wall -Wextra -O2 -std=c11 workloads/normal.c -o workloads/bin/normal

gcc -Wall -Wextra -O2 -std=c11 workloads/memory_intensive.c -o workloads/bin/memory_intensive

gcc -Wall -Wextra -O2 -std=c11 workloads/sustained_growth.c -o workloads/bin/sustained_growth

gcc -Wall -Wextra -O2 -std=c11 workloads/bursty.c -o workloads/bin/bursty
