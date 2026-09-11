#include "history.h"


void history_init(ProcessHistory *history)
{
    if (history == NULL) {
        return;
    }

    history->count = 0;
    history->next_index = 0;
}


int history_add_sample(
    ProcessHistory *history,
    const MemorySample *sample
)
{
    if (history == NULL || sample == NULL) {
        return -1;
    }

    history->samples[history->next_index] = *sample;

    history->next_index =
        (history->next_index + 1) % HISTORY_CAPACITY;

    if (history->count < HISTORY_CAPACITY) {
        history->count++;
    }

    return 0;
}


size_t history_count(
    const ProcessHistory *history
)
{
    if (history == NULL) {
        return 0;
    }

    return history->count;
}


int history_get_sample(
    const ProcessHistory *history,
    size_t index,
    MemorySample *out
)
{
    if (history == NULL || out == NULL) {
        return -1;
    }

    if (index >= history->count) {
        return -1;
    }

    size_t oldest_index;

    if (history->count < HISTORY_CAPACITY) {

        oldest_index = 0;

    } else {

        oldest_index = history->next_index;
    }

    size_t actual_index =
        (oldest_index + index) % HISTORY_CAPACITY;

    *out = history->samples[actual_index];

    return 0;
}