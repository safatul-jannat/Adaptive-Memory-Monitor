#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOW_MEMORY_MB 20
#define HIGH_MEMORY_MB 200

#define LOW_HOLD_SECONDS 20
#define HIGH_HOLD_SECONDS 20
#define RELEASE_HOLD_SECONDS 20

int main(void)
{
    size_t low_size = (size_t)LOW_MEMORY_MB * 1024 * 1024;
    size_t high_size = (size_t)HIGH_MEMORY_MB * 1024 * 1024;

    char *low_buffer = malloc(low_size);

    if (low_buffer == NULL)
    {
        perror("Low memory allocation failed");
        return 1;
    }

    memset(low_buffer, 1, low_size);

    printf("Memory intensive workload started.\n");
    printf("PID: %d\n", getpid());
    printf("LOW phase: %d MB for %d seconds\n",
           LOW_MEMORY_MB, LOW_HOLD_SECONDS);

    fflush(stdout);

    sleep(LOW_HOLD_SECONDS);

    char *high_buffer = malloc(high_size);

    if (high_buffer == NULL)
    {
        perror("High memory allocation failed");
        free(low_buffer);
        return 1;
    }

    memset(high_buffer, 1, high_size);

    printf("HIGH phase: %d MB for %d seconds\n",
           HIGH_MEMORY_MB, HIGH_HOLD_SECONDS);

    fflush(stdout);

    sleep(HIGH_HOLD_SECONDS);

    free(high_buffer);
    free(low_buffer);

    printf("Memory released.\n");
    printf("RELEASE phase: %d seconds\n", RELEASE_HOLD_SECONDS);

    fflush(stdout);

    sleep(RELEASE_HOLD_SECONDS);

    printf("Memory intensive workload finished.\n");

    return 0;
}
