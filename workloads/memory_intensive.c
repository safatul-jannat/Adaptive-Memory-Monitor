#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEMORY_MB 200
#define HOLD_SECONDS 20

int main(void)
{
    size_t size = (size_t)MEMORY_MB * 1024 * 1024;

    char *buffer = malloc(size);

    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        return 1;
    }

    memset(buffer, 1, size);

    printf("Memory intensive workload started.\n");
    printf("PID: %d\n", getpid());
    printf("Allocated memory: %d MB\n", MEMORY_MB);
    printf("Holding memory for %d seconds...\n", HOLD_SECONDS);

    fflush(stdout);

    sleep(HOLD_SECONDS);

    free(buffer);

    printf("Memory released.\n");
    printf("Memory intensive workload finished.\n");

    return 0;
}
