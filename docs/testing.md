# Testing and Quality Assurance

## 1. Testing Objective

The objective of testing is to verify that the Adaptive Memory Monitor can distinguish between normal memory behavior, temporary high memory usage, sustained memory growth, and bursty memory usage.

Four workload programs were created for testing:

1. `normal.c`
2. `memory_intensive.c`
3. `sustained_growth.c`
4. `bursty.c`

## 2. Workload Testing

| Workload         | Ground Truth          | Expected Behavior                     | Observed Behavior                           | Status |
| ---------------- | --------------------- | ------------------------------------- | ------------------------------------------- | ------ |
| normal           | Normal                | Stable low memory usage               | Memory remained stable                      | PASS   |
| memory_intensive | Normal / Non-abnormal | High memory usage followed by release | 200 MB allocated and later released         | PASS   |
| sustained_growth | Abnormal              | Memory continuously increases         | Memory increased gradually and was retained | PASS   |
| bursty           | Normal / Temporary    | Memory spikes followed by release     | 100 MB allocated and released in 3 cycles   | PASS   |

## 3. Test Results

### Normal Workload

The `normal.c` workload allocated 10 MB and maintained the allocation for a fixed period.

Observed behavior:

* Low memory usage
* Stable RSS
* No continuous memory growth

Result: **PASS**

### Memory Intensive Workload

The `memory_intensive.c` workload allocated 200 MB and held the memory for 20 seconds.

Observed output:

```text
Memory intensive workload started.
Allocated memory: 200 MB
Holding memory for 20 seconds...
Memory released.
Memory intensive workload finished.
```

The memory was released after the holding period.

Result: **PASS**

### Sustained Growth Workload

The `sustained_growth.c` workload continuously allocated additional memory.

Observed behavior:

```text
Allocated additional 10 MB
Allocated additional 20 MB
Allocated additional 30 MB
...
Allocated additional 450 MB
```

The retained memory increased over time, representing abnormal sustained growth.

Result: **PASS**

### Bursty Workload

The `bursty.c` workload repeatedly allocated and released memory.

Observed output:

```text
Burst 1: allocated 100 MB
Burst 1: memory released
Burst 2: allocated 100 MB
Burst 2: memory released
Burst 3: allocated 100 MB
Burst 3: memory released
```

This represents temporary memory spikes rather than continuous growth.

Result: **PASS**

## 4. RSS Validation

Linux process memory behavior was also checked using:

```bash
grep -E "VmSize|VmRSS" /proc/PID/status
```

For the sustained growth workload, RSS increased significantly while memory was retained.

Example observed result:

```text
VmSize: 238380 kB
VmRSS:  237612 kB
```

This confirms that the workload can produce significant retained memory growth.

For short-duration workloads, `/proc/PID/status` may disappear after the process terminates. This is expected Linux behavior and does not indicate a workload failure.

## 5. Ground Truth Classification

The expected classification of the workloads is:

* `normal.c` → Normal
* `memory_intensive.c` → Normal / Non-abnormal
* `sustained_growth.c` → Abnormal / High Risk
* `bursty.c` → Normal / Temporary Spike

## 6. QA Conclusion

All four workload programs compiled and executed successfully.

The workloads produced the intended memory behavior:

* Stable memory usage
* Temporary high memory usage
* Sustained memory growth
* Repeated temporary memory spikes

Therefore, the workload test suite is ready to be used for validating the Adaptive Memory Monitor.
