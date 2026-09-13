# Demo Guide

## 1. Project Overview

The Adaptive Memory Monitor monitors Linux application memory behavior and identifies abnormal memory growth.

The final demonstration focuses on the complete pipeline:

Workload → Linux /proc → Adaptive Memory Monitor → CSV Results

Four controlled workloads are available:

1. Normal
2. Memory Intensive
3. Sustained Growth
4. Bursty

---

## 2. Build the Project

From the project root:

```bash
cd ~/Projects/Adaptive-Memory-Monitor
```

Build the workload programs:

```bash
mkdir -p workloads/bin

gcc -Wall -Wextra -O2 -std=c11 workloads/normal.c -o workloads/bin/normal
gcc -Wall -Wextra -O2 -std=c11 workloads/memory_intensive.c -o workloads/bin/memory_intensive
gcc -Wall -Wextra -O2 -std=c11 workloads/sustained_growth.c -o workloads/bin/sustained_growth
gcc -Wall -Wextra -O2 -std=c11 workloads/bursty.c -o workloads/bin/bursty
```

---

## 3. Show Linux Process Memory

Start a workload:

```bash
./workloads/bin/normal > /tmp/normal.log 2>&1 &
```

Get the PID:

```bash
PID=$!
echo $PID
```

Show memory information:

```bash
grep -E 'VmRSS|VmSize' /proc/$PID/status
```

This demonstrates that the project uses real Linux process memory information.

---

## 4. Normal Workload

The normal workload keeps approximately 10 MB of memory allocated.

Expected behavior:

* RSS remains relatively stable.
* No continuous memory growth occurs.
* Risk should remain low.

Run:

```bash
./workloads/bin/normal
```

---

## 5. Sustained Growth Workload

The sustained growth workload first remains stable and then continuously allocates additional memory.

Run:

```bash
./workloads/bin/sustained_growth > /tmp/sustained.log 2>&1 &
```

Get the PID:

```bash
PID=$!
echo $PID
```

Expected behavior:

* Initial memory remains stable.
* RSS starts increasing after the stable phase.
* Growth continues over time.
* The monitor should detect sustained growth.

---

## 6. Run the Adaptive Monitor

Use the workload PID:

```bash
./monitor --pid $PID --strategy adaptive --duration 90 --output /tmp/sustained.csv
```

The monitor records memory and risk information in CSV format.

Important fields include:

```text
timestamp
pid
process_name
rss_kb
vmsize_kb
growth_rate
sustained_growth
retention_score
historical_deviation
risk_score
risk_level
sampling_interval
strategy
```

---

## 7. Demonstrate Adaptive Sampling

For a stable process:

```text
Risk is low
↓
Longer sampling interval
```

For a continuously growing process:

```text
Memory growth detected
↓
Risk increases
↓
Sampling interval becomes shorter
```

Expected adaptive behavior:

```text
10 seconds → 5 seconds → 2 seconds → 1 second
```

---

## 8. Show the Generated CSV

After monitoring:

```bash
head /tmp/sustained.csv
```

Or:

```bash
cat /tmp/sustained.csv
```

The CSV contains the monitor's actual observations and risk information.

---

## 9. Other Workloads

### Memory Intensive

Run:

```bash
./workloads/bin/memory_intensive
```

Expected behavior:

```text
Low memory
↓
Approximately 200 MB allocation
↓
Memory held
↓
Memory released
↓
Low memory
```

Expected classification:

```text
Non-abnormal / Heavy Releasing
```

### Bursty

Run:

```bash
./workloads/bin/bursty
```

Expected behavior:

```text
LOW → HIGH → LOW → HIGH → LOW → HIGH → LOW
```

Expected classification:

```text
Non-abnormal / Bursty
```

---

## 10. Final Demonstration Sequence

Use this order during the presentation:

1. Explain the project objective.
2. Show a running workload PID.
3. Show `/proc/<PID>/status`.
4. Start the normal workload.
5. Start the sustained-growth workload.
6. Run the adaptive monitor.
7. Show the generated CSV.
8. Explain adaptive sampling.
9. Show that stable memory uses a longer interval.
10. Show that sustained growth causes shorter sampling intervals.
11. Briefly explain memory-intensive and bursty workloads.
12. Show the final testing results.

---

## 11. Expected Final Results

| Workload         | Expected Result                |
| ---------------- | ------------------------------ |
| Normal           | Normal / Low Risk              |
| Memory Intensive | Non-abnormal / Heavy Releasing |
| Sustained Growth | Abnormal / High Risk           |
| Bursty           | Non-abnormal / Bursty          |

---

## 12. Conclusion

The demonstration shows the complete workflow of the Adaptive Memory Monitor.

Real Linux workload processes generate memory behavior, Linux `/proc` provides the process memory information, and the monitor analyzes the behavior to estimate risk.

The adaptive strategy increases sampling frequency when memory growth becomes suspicious, allowing abnormal behavior to be monitored more closely.
