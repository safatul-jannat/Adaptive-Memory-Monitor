# Demo Guide

## 1. Project Overview

The Adaptive Memory Monitor is designed to monitor Linux application memory behavior and identify abnormal memory growth.

The testing phase uses four workload programs with different memory behavior patterns.

## 2. Workload Programs

### 2.1 Normal Workload

File:

```text
workloads/normal.c
```

Purpose:

* Uses a small and stable amount of memory.
* Represents normal application behavior.
* Expected classification: Normal.

Run:

```bash
cd ~/Projects/Adaptive-Memory-Monitor/workloads
./normal
```

---

### 2.2 Memory Intensive Workload

File:

```text
workloads/memory_intensive.c
```

Purpose:

* Allocates 200 MB of memory.
* Holds the memory temporarily.
* Releases the memory after 20 seconds.
* Represents temporary high memory usage.
* Expected classification: Normal / Non-abnormal.

Run:

```bash
cd ~/Projects/Adaptive-Memory-Monitor/workloads
./memory_intensive
```

Expected output:

```text
Memory intensive workload started.
Allocated memory: 200 MB
Holding memory for 20 seconds...
Memory released.
Memory intensive workload finished.
```

---

### 2.3 Sustained Growth Workload

File:

```text
workloads/sustained_growth.c
```

Purpose:

* Continuously allocates additional memory.
* Retains the allocated memory.
* Demonstrates sustained memory growth.
* Expected classification: Abnormal / High Risk.

Run:

```bash
cd ~/Projects/Adaptive-Memory-Monitor/workloads
./sustained_growth
```

Expected behavior:

```text
Allocated additional 10 MB
Allocated additional 20 MB
Allocated additional 30 MB
...
```

The memory usage should continue increasing over time.

---

### 2.4 Bursty Workload

File:

```text
workloads/bursty.c
```

Purpose:

* Creates temporary memory spikes.
* Releases memory after each spike.
* Repeats the process three times.
* Expected classification: Normal / Temporary Spike.

Run:

```bash
cd ~/Projects/Adaptive-Memory-Monitor/workloads
./bursty
```

Expected output:

```text
Burst 1: allocated 100 MB
Burst 1: memory released
Burst 2: allocated 100 MB
Burst 2: memory released
Burst 3: allocated 100 MB
Burst 3: memory released
```

## 3. RSS Monitoring Demo

While a workload is running, another terminal can be used to inspect its memory usage.

In Terminal 2:

```bash
grep -E "VmSize|VmRSS" /proc/PID/status
```

Replace `PID` with the actual process ID.

Example:

```bash
grep -E "VmSize|VmRSS" /proc/4969/status
```

Example result:

```text
VmSize: 238380 kB
VmRSS:  237612 kB
```

For short-running workloads, `/proc/PID/status` may disappear after the process finishes. This is normal Linux behavior.

## 4. Demonstration Sequence

For the project demonstration, use the following order:

1. Explain the project objective.
2. Show the four workload programs.
3. Run `normal`.
4. Run `memory_intensive`.
5. Run `sustained_growth`.
6. Show increasing memory behavior.
7. Run `bursty`.
8. Show the memory allocation and release cycles.
9. Explain the expected ground-truth classification.
10. Show the testing results in `docs/testing.md`.

## 5. Expected Demonstration Result

| Workload         | Expected Classification  |
| ---------------- | ------------------------ |
| normal           | Normal                   |
| memory_intensive | Normal / Non-abnormal    |
| sustained_growth | Abnormal / High Risk     |
| bursty           | Normal / Temporary Spike |

## 6. Conclusion

The four workloads provide different memory behavior patterns for testing the Adaptive Memory Monitor.

Together, they allow the project team to evaluate whether the monitor can distinguish stable memory usage, temporary high usage, sustained abnormal growth, and temporary bursty behavior.
