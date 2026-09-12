#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "types.h"
#include "process_discovery.h"
#include "proc_reader.h"
#include "logger.h"
#include "history.h"
#include "analyzer.h"
#include "risk_engine.h"
#include "scheduler.h"

#define MAX_PROCESSES 1024

/*
 * The analyzer time window must cover the whole stored
 * history even when the scheduler is sampling slowly
 * (10 seconds per sample).
 */
#define ANALYSIS_WINDOW_SECONDS (HISTORY_CAPACITY * 10)

typedef struct
{
    int pid;
    ProcessHistory history;
    int active;
} ProcessHistoryEntry;

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

    return "UNKNOWN";
}

int main()
{
    int pids[MAX_PROCESSES];
    int process_count;

    static ProcessHistoryEntry history_table[MAX_PROCESSES] = {0};

    SchedulerState sched;

    scheduler_init(&sched);

    printf("Adaptive Memory Monitor\n");
    printf("=======================\n");

    while (1)
    {
        process_count = get_process_list(pids, MAX_PROCESSES);

        if (process_count < 0)
        {
            printf("Failed to discover processes\n");

            sleep(sched.current_interval);

            continue;
        }

        printf("\nProcesses found: %d\n\n", process_count);

        printf("%-8s %-20s %-12s %-12s\n",
               "PID", "NAME", "RSS(KB)", "VSIZE(KB)");

        printf("------------------------------------------------------\n");

        /*
         * Track the worst risk level seen this round
         * and the process that caused it.
         */
        RiskLevel worst = RISK_NORMAL;
        int worst_pid = -1;

        for (int i = 0; i < process_count; i++)
        {
            MemorySample sample;

            if (read_process_memory(pids[i], &sample) != 0)
            {
                continue;
            }

            sample.timestamp = time(NULL);

            /* look up the history entry for this pid */
            ProcessHistoryEntry *entry = NULL;

            for (int e = 0; e < MAX_PROCESSES; e++)
            {
                if (history_table[e].active &&
                    history_table[e].pid == sample.pid)
                {
                    entry = &history_table[e];
                    break;
                }
            }

            /* no entry yet, so create one */
            if (entry == NULL)
            {
                for (int e = 0; e < MAX_PROCESSES; e++)
                {
                    if (!history_table[e].active)
                    {
                        entry = &history_table[e];
                        entry->pid = sample.pid;
                        entry->active = 1;
                        history_init(&entry->history);
                        break;
                    }
                }
            }

            if (entry != NULL)
            {
                history_add_sample(&entry->history, &sample);
            }

            printf("%-8d %-20s %-12lu %-12lu\n",
                   sample.pid, sample.name,
                   sample.rss_kb, sample.vmsize_kb);

            log_sample(&sample);

            /* analyse once enough samples have been collected */
            if (entry == NULL ||
                history_count(&entry->history) < 2)
            {
                continue;
            }

            double growth = calculate_growth_rate(&entry->history);

            double sustained = calculate_sustained_growth(
                &entry->history, ANALYSIS_WINDOW_SECONDS);

            double retention = calculate_retention_score(
                &entry->history, ANALYSIS_WINDOW_SECONDS);

            double deviation =
                calculate_historical_deviation(&entry->history);

            double risk = calculate_risk_from_features(
                growth, sustained, retention, deviation);

            RiskLevel level = classify_risk(risk);

            if (level > worst)
            {
                worst = level;
                worst_pid = sample.pid;
            }
        }

        /* feed the worst level to the global scheduler */
        int interval = scheduler_update(&sched, worst);

        if (worst_pid > 0)
        {
            printf("\nHighest risk this round: PID %d (%s)\n",
                   worst_pid, risk_name(worst));
        }

        printf("Next sampling interval: %d sec\n", interval);

        sleep(interval);
    }

    return 0;
}