#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEMORY_MB 10
#define RUN_SECONDS 120

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

    printf("Normal workload started.\n");
    printf("PID: %d\n", getpid());
    printf("Allocated memory: %d MB\n", MEMORY_MB);
    printf("Running for %d seconds...\n", RUN_SECONDS);

    fflush(stdout);

    sleep(RUN_SECONDS);

    free(buffer);

    printf("Normal workload finished.\n");

    return 0;
}
