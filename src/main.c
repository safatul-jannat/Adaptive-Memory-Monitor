#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "process_discovery.h"
#include "proc_reader.h"


#define MAX_PROCESSES 1024
#define SAMPLE_INTERVAL 5


int main()
{
    int pids[MAX_PROCESSES];

    int process_count;


    printf("Adaptive Memory Monitor\n");
    printf("=======================\n");


    while(1)
    {
        /*
         * Discover currently running processes
         */

        process_count = get_process_list(
            pids,
            MAX_PROCESSES
        );


        if(process_count < 0)
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
         * for every discovered process
         */

        for(int i = 0; i < process_count; i++)
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

            if(result != 0)
            {
                continue;
            }


            printf(
                "%-8d %-20s %-12lu %-12lu\n",
                sample.pid,
                sample.name,
                sample.rss_kb,
                sample.vmsize_kb
            );
        }


        /*
         * Wait before next monitoring round
         */

        sleep(SAMPLE_INTERVAL);
    }


    return 0;
}