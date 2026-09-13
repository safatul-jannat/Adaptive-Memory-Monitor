# Member 2 Implementation Progress

## Project

Adaptive Memory Monitor

## Branch

feature/analyzer

## Member 2 Scope

Owned source files:

* src/history.c
* src/history.h
* src/analyzer.c
* src/analyzer.h
* src/risk_engine.c
* src/risk_engine.h
* src/scheduler.c
* src/scheduler.h
* tests/

Additional local tracking file:

* MEMBER2_UPDATE.md

## Files Member 2 Must NOT Modify

* src/main.c
* src/config.h
* src/proc_reader.c
* src/proc_reader.h
* src/process_discovery.c
* src/process_discovery.h
* src/logger.c
* src/logger.h
* Makefile
* workloads/*
* experiments/*
* analysis/*
* dashboard/*
* Member 3 result files

## Locked Design

Risk formula:

Risk = 0.35G + 0.30S + 0.20R + 0.15H

Risk levels:

* NORMAL: 0-30
* SUSPICIOUS: 31-60
* HIGH: 61-80
* CRITICAL: 81-100

Scheduler:

* NORMAL -> 10 seconds
* SUSPICIOUS -> 5 seconds
* HIGH -> 2 seconds
* CRITICAL -> 1 second

Scheduler behavior:

* Escalation is immediate.
* De-escalation requires 3 consecutive lower-risk evaluations.

Growth rate formula (locked, unchanged):

* (current RSS - previous RSS) / elapsed seconds

## Final Requirements Checklist

* [x] History ring buffer previously implemented
* [x] Growth rate previously implemented
* [x] Harden sustained growth against transient spikes
* [x] Rewrite retention to measure retained repeated growth
* [x] Make historical deviation upward-only
* [x] Verify risk engine remains unchanged
* [x] Verify scheduler remains unchanged
* [x] Fix test_history.c include
* [x] Add transient spike test
* [x] Add persistent growth test
* [x] Add independent per-process scheduler test
* [x] Test downward historical deviation
* [x] Run all Member 2 tests
* [x] Review final diff
* [x] Final Git status checked

## Completed Work

### Task 1: Harden sustained growth against transient spikes

Files changed:
* src/analyzer.c

Implementation detail:
* Added `MIN_POSITIVE_GROWTH_INTERVALS 3` and `MIN_SUSTAINED_GROWTH_RATIO 0.75`.
* `calculate_sustained_growth()` keeps the existing time-window counting loop.
* After counting: `total_intervals == 0` -> 0.0; `positive_growth < 3` -> 0.0;
  `ratio < 0.75` -> 0.0; otherwise return ratio.
* One or two positive intervals alone are no longer sufficient evidence.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_sustained_growth.c src/history.c src/analyzer.c -o /tmp/test_sustained_growth`

Test command:
* `/tmp/test_sustained_growth`

Result:
* PASS (ratio 1.00)

### Task 2: Rewrite retention to measure retained repeated growth

Files changed:
* src/analyzer.c

Implementation detail:
* `calculate_retention_score()` no longer uses `(newest - min) / (max - min)`.
* Walks forward over chronological intervals inside the requested window.
* Tracks `total_intervals`, `positive_growth`, `total_positive_growth`,
  `oldest_rss` (RSS at the start of the oldest interval inside the window), and `newest`.
* Applies the same evidence rules as sustained growth
  (>= 3 positive intervals, ratio >= 0.75).
* `total_positive_growth == 0` -> 0.0; `newest <= oldest_rss` -> 0.0 (avoids unsigned underflow).
* `retention = (newest - oldest_rss) / total_positive_growth`, clamped to [0.0, 1.0].
* 100->200->300->400 gives 1.0; spike 100->400->100 gives 0.0; single allocation gives 0.0.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_retention.c src/history.c src/analyzer.c -o /tmp/test_retention`

Test command:
* `/tmp/test_retention`

Result:
* PASS (retained 1.00, released 0.00)

### Task 3: Make historical deviation upward-only

Files changed:
* src/analyzer.c

Implementation detail:
* `calculate_historical_deviation()` keeps the existing historical mean calculation.
* `mean_rss == 0` -> 0.0; `current.rss_kb <= mean_rss` -> 0.0.
* Otherwise returns `(current.rss_kb - mean_rss) / mean_rss`.
* Absolute value removed: downward memory release is no longer abnormal-growth evidence.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_deviation.c src/history.c src/analyzer.c -o /tmp/test_deviation`

Test command:
* `/tmp/test_deviation`

Result:
* PASS (upward 0.091, downward 0.0000)

### Task 4: Fix test_history.c include

Files changed:
* tests/test_history.c

Implementation detail:
* Changed `#include "src/history.h"` to `#include "../src/history.h"`.
* No history implementation changes were needed.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_history.c src/history.c -o /tmp/test_history`

Test command:
* `/tmp/test_history`

Result:
* PASS (all 5 history tests)

### Task 5: Add transient spike test

Files changed (new):
* tests/test_transient_spike.c

Implementation detail:
* History: 100000@0, 100000@10, 400000@20, 400000@30 (60-second windows).
* Asserts: sustained_growth == 0.0, retention == 0.0, risk < 61.0,
  level not RISK_HIGH and not RISK_CRITICAL.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_transient_spike.c src/history.c src/analyzer.c src/risk_engine.c -o /tmp/test_transient_spike`

Test command:
* `/tmp/test_transient_spike`

Result:
* PASS (risk score 9.00, level NORMAL)

### Task 6: Add persistent growth test

Files changed (new):
* tests/test_persistent_growth.c

Implementation detail:
* History: 100000@0, 110000@10, 120000@20, 130000@30, 140000@40 (60-second windows).
* Asserts: sustained_growth >= 0.75, retention > 0.8, risk >= 61.0,
  level HIGH or CRITICAL, scheduler interval 2s or 1s accordingly.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_persistent_growth.c src/history.c src/analyzer.c src/risk_engine.c src/scheduler.c -o /tmp/test_persistent_growth`

Test command:
* `/tmp/test_persistent_growth`

Result:
* PASS (risk 86.68, level CRITICAL, interval 1s)

### Task 7: Add independent per-process scheduler test

Files changed (new):
* tests/test_per_process_scheduler.c

Implementation detail:
* Two `SchedulerState`s (stable, growing) both initialized at 10s.
* Escalating ONLY `growing` to RISK_HIGH -> 2s and RISK_CRITICAL -> 1s.
* `stable` remains at 10s with current_level RISK_NORMAL.

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_per_process_scheduler.c src/scheduler.c src/risk_engine.c -o /tmp/test_per_process_scheduler`

Test command:
* `/tmp/test_per_process_scheduler`

Result:
* PASS

### Task 8: Extend deviation test with downward release

Files changed:
* tests/test_deviation.c

Implementation detail:
* Kept the existing upward-deviation test as DEVIATION TEST 1.
* Added a second independent history 300000@0, 300000@10, 100000@20.
* Asserts downward_deviation == 0.0 (DEVIATION TEST 2).

Compile command:
* `gcc -Wall -Wextra -Wpedantic -std=c11 tests/test_deviation.c src/history.c src/analyzer.c -o /tmp/test_deviation`

Test command:
* `/tmp/test_deviation`

Result:
* PASS

### Task 9: Verify risk engine (no changes)

Files inspected:
* src/risk_engine.c
* src/risk_engine.h
* tests/test_risk.c

Result:
* UNCHANGED - already correct.
* Formula: 0.35 growth + 0.30 sustained + 0.20 retention + 0.15 deviation.
* Classification: <=30 NORMAL, <=60 SUSPICIOUS, <=80 HIGH, otherwise CRITICAL.
* Existing risk tests pass.

### Task 10: Verify scheduler (no changes)

Files inspected:
* src/scheduler.c
* src/scheduler.h
* tests/test_scheduler.c

Result:
* UNCHANGED - already correct.
* NORMAL 10s, SUSPICIOUS 5s, HIGH 2s, CRITICAL 1s.
* Immediate escalation; de-escalation requires 3 consecutive lower-risk evaluations.
* Existing scheduler hysteresis tests pass.

## Current Task

All Member 2 final requirements completed. Only final Git-state review and
MEMBER2_UPDATE.md update remain (done here).

## Problems / Errors

* No compiler warnings, errors, or assertion failures.
* Makefile is empty (0 bytes) and has no `test` target, so all tests were
  compiled and run manually into `/tmp` binaries as permitted.

## Final Test Results

| Test | Result |
| --- | --- |
| tests/test_history.c | PASS |
| tests/test_analyzer.c | PASS |
| tests/test_sustained_growth.c | PASS |
| tests/test_retention.c | PASS |
| tests/test_deviation.c | PASS |
| tests/test_risk.c | PASS |
| tests/test_scheduler.c | PASS |
| tests/test_member2_integration.c | PASS |
| tests/test_transient_spike.c | PASS |
| tests/test_persistent_growth.c | PASS |
| tests/test_per_process_scheduler.c | PASS |

All 11 Member 2 tests PASS. Compiler flags used for every build:
`-Wall -Wextra -Wpedantic -std=c11`. No warnings produced.

## Git State

* Current branch: feature/analyzer
* Branch is ahead of origin/feature/analyzer by 22 commits (fast-forwarded
  feature/analyzer to main's tip 38c880d before starting implementation).
* Modified files:
  * src/analyzer.c
  * tests/test_deviation.c
  * tests/test_history.c
* New files (untracked):
  * tests/test_transient_spike.c
  * tests/test_persistent_growth.c
  * tests/test_per_process_scheduler.c
  * MEMBER2_UPDATE.md
* Untracked pre-existing compiled test binaries left in place
  (test_analyzer, test_deviation, test_history, test_member2_integration,
  test_retention, test_risk, test_scheduler, test_sustained_growth) plus
  FINAL_36H_MEMBER_INSTRUCTIONS.md.
* Nothing committed during this task.
* Nothing pushed during this task.

## Next Step

Review Member 2 changes with the team/user, then stage only approved Member 2 files, commit with:

Harden abnormal-growth features against transient spikes

and push feature/analyzer.