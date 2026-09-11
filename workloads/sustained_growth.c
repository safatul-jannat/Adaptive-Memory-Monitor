#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STEP_MB 10
#define MAX_MB 512
#define INTERVAL_SECONDS 2
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
    printf("Growth: %d MB every %d seconds\n",
           STEP_MB, INTERVAL_SECONDS);
    printf("Maximum memory: %d MB\n", MAX_MB);
    printf("Running for %d seconds...\n", RUN_SECONDS);

    fflush(stdout);

    int elapsed = 0;

    while (elapsed < RUN_SECONDS)
    {
        if (elapsed >= 30)
        {
            int index = (elapsed - 30) / INTERVAL_SECONDS;

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
        }

        sleep(1);
        elapsed++;
    }

    for (int i = 0; i < blocks; i++)
    {
        free(memory[i]);
    }

    free(memory);

    printf("Sustained growth workload finished.\n");

    return 0;
}
