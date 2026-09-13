# Adaptive Memory Monitor

## Project purpose

A Linux user-space monitor that reads per-process memory from `/proc/<pid>/status`
and produces a weighted, explainable abnormal memory-growth risk score
(Normal/Suspicious/High/Critical) per process. It supports four sampling
strategies (three fixed-rate plus one adaptive) from a single binary, writes a
full experiment CSV, and can monitor either one target PID or every discovered
Linux process.

## Linux requirement

Linux kernel with a mounted `procfs`. The monitor reads
`/proc/<pid>/status` (`VmRSS`, `VmSize`, `Name`); process discovery reads the
numeric directories of `/proc`. Build: `gcc` with C11 support
(`-std=c11`). The workloads allocate and `memset()` memory and are for
controlled-risk experiments.

## Build commands

```bash
make clean
make            # builds ./monitor
make workloads  # builds workloads/bin/{normal,memory_intensive,sustained_growth,bursty}
make test       # builds and runs every test in tests/
make monitor    # alias for building ./monitor
make tests      # builds test binaries without running them
```

## Four strategy names

```text
fixed1    -> sample every 1 second
fixed5    -> sample every 5 seconds
fixed10   -> sample every 10 seconds
adaptive  -> scheduler decides 10/5/2/1 seconds from per-process risk level
```

All four strategies run the identical collector, history, analyzer, and risk
engine; only the sampling policy differs.

## CLI examples

```bash
./monitor --pid <PID> --strategy fixed1  --duration <SEC> --output <CSV>
./monitor --pid <PID> --strategy fixed5  --duration <SEC> --output <CSV>
./monitor --pid <PID> --strategy fixed10 --duration <SEC> --output <CSV>
./monitor --pid <PID> --strategy adaptive --duration <SEC> --output <CSV>
```

```text
--pid <PID>           optional; if absent, monitor discovered processes
--strategy <NAME>     required; one of the four strategy names above
--duration <SECONDS>  0 or absent = run forever
--output <CSV_PATH>   default = memory_log.csv
```

## CSV schema

```text
timestamp,pid,process_name,rss_kb,vmsize_kb,growth_rate,sustained_growth,retention_score,historical_deviation,risk_score,risk_level,sampling_interval,strategy
```

`timestamp` is epoch seconds. `risk_level` is one of
`NORMAL,SUSPICIOUS,HIGH,CRITICAL`. One row is written only when a process is
actually sampled.

## Workload commands

```bash
./workloads/bin/normal
./workloads/bin/memory_intensive
./workloads/bin/sustained_growth
./workloads/bin/bursty
```

Start a workload in the background, capture its PID, then run `./monitor`
against that PID with any strategy.

## Experiment command pointer

Run the controlled experiment matrix with:

```bash
./experiments/run_experiment.sh <strategy> <workload> <run_number>
```

and inspect generated tables/plots under `results/` (see the current analysis
scripts for details).

## OS concepts actually used

- `/proc` virtual filesystem and per-process `status` fields (`VmRSS`, `VmSize`, `Name`)
- `procfs` directory enumeration for process discovery
- Epoch timestamping for sample timing and duration-based termination
- Per-process scheduler state with hysteresis and independent per-PID
  next-sample deadlines (adaptive mode)
- User-space memory-growth risk monitoring (not kernel changes)

## Known limitations

- Sampling accuracy depends on the `MAIN_LOOP_TICK_SECONDS` tick.
- A process that exits between discovery and read is skipped gracefully; its
  history stops accumulating.
- Risk is about abnormal memory-growth behavior, not memory-leak detection.
- Analysis is limited to the stored history window per process.