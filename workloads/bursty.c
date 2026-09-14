#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEMORY_MB 100
#define PRE_SECONDS 15
#define HOLD_SECONDS 12
#define GAP_SECONDS 10
#define CYCLES 3

int main(void)
{
    size_t size = (size_t)MEMORY_MB * 1024 * 1024;

    printf("Bursty workload started.\n");
    printf("PID: %d\n", getpid());
    printf("Each burst: %d MB\n", MEMORY_MB);
    printf("Cycles: %d\n", CYCLES);
    printf("Baseline: %d seconds\n", PRE_SECONDS);
    fflush(stdout);

    /* Initial low-memory baseline */
    printf("Baseline phase started.\n");
    fflush(stdout);

    sleep(PRE_SECONDS);

    for (int i = 1; i <= CYCLES; i++)
    {
        volatile unsigned char *buffer = malloc(size);

        if (buffer == NULL)
        {
            perror("Memory allocation failed");
            return 1;
        }

        /*
         * Touch one byte from every memory page so the
         * allocated memory is actually reflected in RSS.
         */
        for (size_t j = 0; j < size; j += 4096)
        {
            buffer[j] = 1;
        }

        printf("Burst %d: allocated %d MB\n", i, MEMORY_MB);
        fflush(stdout);

        /* Keep the high-memory state long enough for fixed10 sampling. */
        sleep(HOLD_SECONDS);

        free((void *)buffer);

        printf("Burst %d: memory released\n", i);
        printf(
            "Burst %d: low-memory phase started (%d seconds)\n",
            i,
            GAP_SECONDS
        );
        fflush(stdout);

        /*
         * Always remain alive in the low-memory state
         * after every release, including the final burst.
         */
        sleep(GAP_SECONDS);
    }

    printf("Bursty workload finished.\n");
    fflush(stdout);

    return 0;
}