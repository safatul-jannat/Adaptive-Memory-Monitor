#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEMORY_MB 100
#define HOLD_SECONDS 5
#define CYCLES 3

int main(void)
{
    size_t size = (size_t)MEMORY_MB * 1024 * 1024;

    printf("Bursty workload started.\n");
    printf("PID: %d\n", getpid());
    printf("Each burst: %d MB\n", MEMORY_MB);
    printf("Cycles: %d\n", CYCLES);

    fflush(stdout);

    for (int i = 1; i <= CYCLES; i++)
    {
        char *buffer = malloc(size);

        if (buffer == NULL)
        {
            perror("Memory allocation failed");
            return 1;
        }

        memset(buffer, 1, size);

        printf("Burst %d: allocated %d MB\n", i, MEMORY_MB);
        fflush(stdout);

        sleep(HOLD_SECONDS);

        free(buffer);

        printf("Burst %d: memory released\n", i);
        fflush(stdout);

        sleep(2);
    }

    printf("Bursty workload finished.\n");

    return 0;
}
