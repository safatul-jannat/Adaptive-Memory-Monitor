#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>
#include "types.h"

/*
 * Maximum number of memory samples kept for one history.
 * This is an initial implementation value and can be tuned later.
 */
#define HISTORY_CAPACITY 128

typedef struct {
    MemorySample samples[HISTORY_CAPACITY];

    /*
     * Number of valid samples currently stored.
     * This never becomes greater than HISTORY_CAPACITY.
     */
    size_t count;

    /*
     * Position where the next sample will be written.
     */
    size_t next_index;

} ProcessHistory;


/* Initialize an empty history. */
void history_init(ProcessHistory *history);


/* Add a new memory sample to the history. */
int history_add_sample(
    ProcessHistory *history,
    const MemorySample *sample
);


/* Return the number of samples currently stored. */
size_t history_count(
    const ProcessHistory *history
);


/*
 * Get a sample by chronological position.
 *
 * index 0 = oldest sample
 * index count-1 = newest sample
 */
int history_get_sample(
    const ProcessHistory *history,
    size_t index,
    MemorySample *out
);


#endif