# Testing and Quality Assurance

## 1. Testing Objective

The objective of testing is to verify that the Adaptive Memory Monitor can distinguish between normal memory behavior, temporary high memory usage, sustained memory growth, and bursty memory usage.

Four workload programs were created for testing:

1. `normal.c`
2. `memory_intensive.c`
3. `sustained_growth.c`
4. `bursty.c`

## 2. Workload Testing

| Workload | Ground Truth | Expected Behavior | Observed Behavior | Status |
|---|---|---|---|---|
| normal | Normal | Stable low memory usage | Memory remained stable | PASS |
| memory_intensive | Normal / Non-abnormal | High memory usage followed by release | 200 MB allocated and later released | PASS |
| sustained_growth | Abnormal | Memory continuously increases | RSS increased continuously over time | PASS |
| bursty | Normal / Temporary | Memory spikes followed by release | RSS increased and decreased repeatedly | PASS |

## 3. Test Results

### Normal Workload

The `normal.c` workload allocated 10 MB and maintained the allocation for a fixed period.

Observed behavior:

- Low memory usage
- Stable RSS
- No continuous memory growth

Result: **PASS**

### Memory Intensive Workload

The `memory_intensive.c` workload allocated 200 MB and held the memory for 20 seconds.

Observed output:

```text
Memory intensive workload started.
PID: 4842
Allocated memory: 200 MB
Holding memory for 20 seconds...
VmRSS: 206808 kB
Memory released.
Memory intensive workload finished.
