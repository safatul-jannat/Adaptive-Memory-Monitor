
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "types.h"
#include "process_discovery.h"
#include "proc_reader.h"
#include "logger.h"
#include "history.h"


#define MAX_PROCESSES 1024
#define SAMPLE_INTERVAL 5


typedef struct
{
    int pid;
    ProcessHistory history;
    int active;
} ProcessHistoryEntry;


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

