
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


#define MAX_PROCESSES 1024
#define SAMPLE_INTERVAL 5

/*
 * Time window used by the analyzer for
 * sustained growth and retention features.
 * Covers the whole history buffer.
 */
#define ANALYSIS_WINDOW_SECONDS \
    (SAMPLE_INTERVAL * HISTORY_CAPACITY)


typedef struct
{
    int pid;
    ProcessHistory history;
    int active;
} ProcessHistoryEntry;


/*
 * Temporary verification helper.
 * Maps the risk level enum to a printable name.
 */
static const char *risk_level_name(RiskLevel level)
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


    printf("Adaptive Memory Monitor\n");
    printf("=======================\n");


    while (1)
    {
        /*
         * Discover currently running processes.
         */
        process_count = get_process_list(
            pids,
            MAX_PROCESSES
        );


        if (process_count < 0)
        {
            printf("Failed to discover processes\n");

            sleep(SAMPLE_INTERVAL);

            continue;
        }


        printf("\n");
        printf("Processes found: %d\n\n", process_count);

        printf(
            "%-8s %-20s %-12s %-12s\n",
            "PID",
            "NAME",
            "RSS(KB)",
            "VSIZE(KB)"
        );

        printf(
            "------------------------------------------------------\n"
        );


        /*
         * Read memory information
         * for every discovered process.
         */
        for (int i = 0; i < process_count; i++)
        {
            MemorySample sample;


            int result = read_process_memory(
                pids[i],
                &sample
            );


            /*
             * Process may have exited between
             * discovery and memory reading.
             */
            if (result != 0)
            {
                continue;
            }


            sample.timestamp = time(NULL);


            /*
             * Locate the per-PID history entry.
             */
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


            /*
             * No active entry yet for this PID,
             * so allocate a new one.
             */
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


            /*
             * Store the real memory sample
             * in per-PID process history.
             */
            if (entry != NULL)
            {
                history_add_sample(
                    &entry->history,
                    &sample
                );
            }


            /*
             * Run the analyzer on this PID's own
             * history once it has enough samples.
             */
            if (entry != NULL &&
                history_count(&entry->history) >= 2)
            {
                double growth = calculate_growth_rate(
                    &entry->history
                );

                double sustained =
                    calculate_sustained_growth(
                        &entry->history,
                        ANALYSIS_WINDOW_SECONDS
                    );

                double retention =
                    calculate_retention_score(
                        &entry->history,
                        ANALYSIS_WINDOW_SECONDS
                    );

                double deviation =
                    calculate_historical_deviation(
                        &entry->history
                    );

                /*
                 * Feed this PID's analyzer features
                 * into the risk engine.
                 */
                double risk_score =
                    calculate_risk_from_features(
                        growth,
                        sustained,
                        retention,
                        deviation
                    );

                RiskLevel risk_level =
                    classify_risk(risk_score);

                printf("PID %d\n", entry->pid);
                printf("Growth: %.4f\n", growth);
                printf("Sustained Growth: %.4f\n", sustained);
                printf("Retention: %.4f\n", retention);
                printf(
                    "Historical Deviation: %.4f\n",
                    deviation
                );
                printf("Risk Score: %.4f\n", risk_score);
                printf(
                    "Risk Level: %s\n",
                    risk_level_name(risk_level)
                );
            }


            printf(
                "%-8d %-20s %-12lu %-12lu\n",
                sample.pid,
                sample.name,
                sample.rss_kb,
                sample.vmsize_kb
            );


            /*
             * Save the sample to CSV.
             */
            log_sample(&sample);
        }

        /*
         * Wait before the next monitoring round.
         */
        sleep(SAMPLE_INTERVAL);
    }


    return 0;
}

