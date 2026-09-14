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

    
    volatile unsigned char *low_buffer = malloc(low_size);
    
    if (low_buffer == NULL)
    {
        perror("Low memory allocation failed");
        return 1;
    }

    /* Touch every page so the allocation is reflected in VmRSS. */
    for (size_t i = 0; i < low_size; i += 4096)
    {
        low_buffer[i] = 1;
    }

    printf("Memory intensive workload started.\n");
    printf("PID: %d\n", getpid());
    printf("LOW phase: %d MB for %d seconds\n",
           LOW_MEMORY_MB, LOW_HOLD_SECONDS);

    fflush(stdout);

    sleep(LOW_HOLD_SECONDS);

    volatile unsigned char *high_buffer = malloc(high_size);
    if (high_buffer == NULL)
    {
        perror("High memory allocation failed");
        free((void *)low_buffer);
        return 1;
    }

    /* Touch every page so the allocation is reflected in VmRSS. */
    for (size_t i = 0; i < high_size; i += 4096)
    {
        high_buffer[i] = 1;
    }

    printf("HIGH phase: %d MB for %d seconds\n",
           HIGH_MEMORY_MB, HIGH_HOLD_SECONDS);

    fflush(stdout);

    sleep(HIGH_HOLD_SECONDS);

    free((void *)high_buffer);
    free((void *)low_buffer);

    printf("Memory released.\n");
    printf("RELEASE phase: %d seconds\n", RELEASE_HOLD_SECONDS);

    fflush(stdout);

    sleep(RELEASE_HOLD_SECONDS);

    printf("Memory intensive workload finished.\n");

    return 0;
}
