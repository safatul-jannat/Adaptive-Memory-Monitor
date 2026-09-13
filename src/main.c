#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "types.h"
#include "process_discovery.h"
#include "proc_reader.h"
#include "logger.h"
#include "history.h"
#include "analyzer.h"
#include "risk_engine.h"
#include "scheduler.h"

#define DEFAULT_INTERVAL 10

typedef enum {
    STRATEGY_FIXED1,
    STRATEGY_FIXED5,
    STRATEGY_FIXED10,
    STRATEGY_ADAPTIVE
} MonitorStrategy;

typedef struct
{
    int pid;
    int active;
    int seen_this_round;

    ProcessHistory history;
    SchedulerState scheduler;

    time_t next_sample_time;

    double growth_rate;
    double sustained_growth;
    double retention_score;
    double historical_deviation;
    double risk_score;
    RiskLevel risk_level;

    int sampling_interval;
} ProcessState;

static const char *strategy_name(MonitorStrategy strategy)
{
    switch (strategy)
    {
        case STRATEGY_FIXED1:
            return "fixed1";
        case STRATEGY_FIXED5:
            return "fixed5";
        case STRATEGY_FIXED10:
            return "fixed10";
        case STRATEGY_ADAPTIVE:
            return "adaptive";
    }

    return "fixed10";
}

static int fixed_interval_for_strategy(MonitorStrategy strategy)
{
    switch (strategy)
    {
        case STRATEGY_FIXED1:
            return 1;
        case STRATEGY_FIXED5:
            return 5;
        case STRATEGY_FIXED10:
            return 10;
        case STRATEGY_ADAPTIVE:
            return -1;
    }

    return 10;
}

static const char *risk_name(RiskLevel level)
{
    switch (level)
    {
        case RISK_NORMAL:
            return "NORMAL";
        case RISK_SUSPICIOUS:
            return "SUSPICIOUS";
        case RISK_HIGH:
            return "HIGH";
        case RISK_CRITICAL:
            return "CRITICAL";
    }

    return "NORMAL";
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  --pid <PID>            monitor a specific process (optional)\n");
    printf("  --strategy <NAME>      fixed1, fixed5, fixed10, or adaptive (required)\n");
    printf("  --duration <SECONDS>   0 or absent = run forever\n");
    printf("  --output <CSV_PATH>    default = memory_log.csv\n");
}

static ProcessState *find_or_create(
    ProcessState *table,
    int pid,
    time_t now
)
{
    for (int e = 0; e < MAX_PROCESSES; e++)
    {
        if (table[e].active && table[e].pid == pid)
        {
            return &table[e];
        }
    }

    for (int e = 0; e < MAX_PROCESSES; e++)
    {
        if (!table[e].active)
        {
            ProcessState *entry = &table[e];

            memset(entry, 0, sizeof(*entry));

            entry->pid = pid;
            entry->active = 1;
            entry->seen_this_round = 1;

            history_init(&entry->history);
            scheduler_init(&entry->scheduler);

            entry->next_sample_time = now;

            entry->risk_level = RISK_NORMAL;
            entry->sampling_interval = DEFAULT_INTERVAL;

            return entry;
        }
    }

    return NULL;
}

static void sample_process(
    ProcessState *state,
    int pid,
    MonitorStrategy strategy,
    time_t now
)
{
    if (now < state->next_sample_time)
    {
        return;
    }

    MemorySample sample;

    if (read_process_memory(pid, &sample) != 0)
    {
        return;
    }

    sample.timestamp = now;

    history_add_sample(&state->history, &sample);

    double growth = 0.0;
    double sustained = 0.0;
    double retention = 0.0;
    double deviation = 0.0;
    double risk = 0.0;
    RiskLevel level = RISK_NORMAL;

    if (history_count(&state->history) >= MIN_ANALYSIS_SAMPLES)
    {
        growth = calculate_growth_rate(&state->history);

        sustained = calculate_sustained_growth(
            &state->history, ANALYSIS_WINDOW_SECONDS);

        retention = calculate_retention_score(
            &state->history, ANALYSIS_WINDOW_SECONDS);

        deviation = calculate_historical_deviation(
            &state->history);

        risk = calculate_risk_from_features(
            growth, sustained, retention, deviation);

        level = classify_risk(risk);
    }

    int interval = fixed_interval_for_strategy(strategy);

    if (strategy == STRATEGY_ADAPTIVE)
    {
        interval = scheduler_update(&state->scheduler, level);
    }

    state->growth_rate = growth;
    state->sustained_growth = sustained;
    state->retention_score = retention;
    state->historical_deviation = deviation;
    state->risk_score = risk;
    state->risk_level = level;
    state->sampling_interval = interval;
    state->next_sample_time = now + interval;

    log_observation(
        &sample,
        growth,
        sustained,
        retention,
        deviation,
        risk,
        level,
        interval,
        strategy_name(strategy)
    );

    printf("PID=%d name=%s RSS=%luKB risk=%.1f level=%s interval=%ds strategy=%s\n",
           sample.pid,
           sample.name,
           sample.rss_kb,
           risk,
           risk_name(level),
           interval,
           strategy_name(strategy));
}

int main(int argc, char **argv)
{
    int target_pid = -1;
    MonitorStrategy strategy = STRATEGY_ADAPTIVE;
    int strategy_set = 0;
    long duration_seconds = 0;
    const char *output_path = "memory_log.csv";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--pid") == 0 && i + 1 < argc)
        {
            target_pid = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--strategy") == 0 && i + 1 < argc)
        {
            const char *name = argv[++i];

            if (strcmp(name, "fixed1") == 0)
            {
                strategy = STRATEGY_FIXED1;
            }
            else if (strcmp(name, "fixed5") == 0)
            {
                strategy = STRATEGY_FIXED5;
            }
            else if (strcmp(name, "fixed10") == 0)
            {
                strategy = STRATEGY_FIXED10;
            }
            else if (strcmp(name, "adaptive") == 0)
            {
                strategy = STRATEGY_ADAPTIVE;
            }
            else
            {
                print_usage(argv[0]);
                return 1;
            }

            strategy_set = 1;
        }
        else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc)
        {
            duration_seconds = atol(argv[++i]);

            if (duration_seconds < 0)
            {
                duration_seconds = 0;
            }
        }
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
        {
            output_path = argv[++i];
        }
        else
        {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!strategy_set)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (logger_init(output_path) != 0)
    {
        fprintf(stderr, "Failed to open output file: %s\n", output_path);
        return 1;
    }

    printf("Adaptive Memory Monitor\n");
    printf("=======================\n");

    if (target_pid > 0)
    {
        printf("Monitoring PID %d (strategy: %s)\n",
               target_pid, strategy_name(strategy));
    }
    else
    {
        printf("Monitoring discovered processes (strategy: %s)\n",
               strategy_name(strategy));
    }

    if (duration_seconds > 0)
    {
        printf("Duration: %ld seconds\n", duration_seconds);
    }

    time_t monitor_start = time(NULL);

    static ProcessState state_table[MAX_PROCESSES] = {0};
    int pids[MAX_PROCESSES];

    while (1)
    {
        if (duration_seconds > 0 &&
            time(NULL) - monitor_start >= duration_seconds)
        {
            break;
        }

        time_t now = time(NULL);

        int process_count = 0;

        if (target_pid > 0)
        {
            pids[0] = target_pid;
            process_count = 1;
        }
        else
        {
            for (int e = 0; e < MAX_PROCESSES; e++)
            {
                if (state_table[e].active)
                {
                    state_table[e].seen_this_round = 0;
                }
            }

            process_count = get_process_list(pids, MAX_PROCESSES);

            if (process_count < 0)
            {
                printf("Failed to discover processes\n");

                sleep(MAIN_LOOP_TICK_SECONDS);

                continue;
            }
        }

        for (int i = 0; i < process_count; i++)
        {
            ProcessState *entry =
                find_or_create(state_table, pids[i], now);

            if (entry == NULL)
            {
                continue;
            }

            entry->seen_this_round = 1;

            sample_process(entry, pids[i], strategy, now);
        }

        if (target_pid <= 0)
        {
            for (int e = 0; e < MAX_PROCESSES; e++)
            {
                if (state_table[e].active &&
                    !state_table[e].seen_this_round)
                {
                    state_table[e].active = 0;
                }
            }
        }

        sleep(MAIN_LOOP_TICK_SECONDS);
    }

    printf("\nMonitoring finished after %ld seconds.\n",
           (long)(time(NULL) - monitor_start));

    return 0;
}