#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define STEP_MB 5
#define MAX_MB 450
#define INTERVAL_SECONDS 1
#define STABLE_SECONDS 30
#define RUN_SECONDS 120

int main(void)
{
    int blocks = MAX_MB / STEP_MB;
    char **memory = malloc(blocks * sizeof(char *));

    if (memory == NULL)
    {
        perror("Memory allocation failed");
        return 1;
    }

    for (int i = 0; i < blocks; i++)
    {
        memory[i] = NULL;
    }

    printf("Sustained growth workload started.\n");
    printf("PID: %d\n", getpid());
    printf("Stable phase: %d seconds\n", STABLE_SECONDS);
    printf("Growth: %d MB every %d second\n",
           STEP_MB, INTERVAL_SECONDS);
    printf("Maximum growth: %d MB\n", MAX_MB);
    printf("Total runtime: %d seconds\n", RUN_SECONDS);

    fflush(stdout);

    sleep(STABLE_SECONDS);

    time_t anomaly_start = time(NULL);

    printf("ANOMALY_START_EPOCH=%ld\n", (long)anomaly_start);
    fflush(stdout);

    int growth_elapsed = 0;

    while (growth_elapsed < (RUN_SECONDS - STABLE_SECONDS))
    {
        int index = growth_elapsed / INTERVAL_SECONDS;

        if (index < blocks && memory[index] == NULL)
        {
            memory[index] = malloc(STEP_MB * 1024 * 1024);

            if (memory[index] == NULL)
            {
                perror("Memory allocation failed");
                break;
            }

            memset(memory[index], 1, STEP_MB * 1024 * 1024);

            printf("Allocated additional %d MB\n",
                   (index + 1) * STEP_MB);
            fflush(stdout);
        }

        sleep(INTERVAL_SECONDS);
        growth_elapsed += INTERVAL_SECONDS;
    }

    for (int i = 0; i < blocks; i++)
    {
        free(memory[i]);
    }

    free(memory);

    printf("Sustained growth workload finished.\n");
    fflush(stdout);

    return 0;
}
