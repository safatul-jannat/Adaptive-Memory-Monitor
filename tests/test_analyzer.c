#include <stdio.h>
#include <assert.h>

#include "../src/analyzer.h"


int main(void)
{
    ProcessHistory history;

    history_init(&history);


    MemorySample sample1 = {
        .rss_kb = 100000,
        .vmsize_kb = 200000,
        .timestamp = 100
    };


    MemorySample sample2 = {
        .rss_kb = 110000,
        .vmsize_kb = 210000,
        .timestamp = 110
    };


    history_add_sample(
        &history,
        &sample1
    );


    history_add_sample(
        &history,
        &sample2
    );


    double growth =
        calculate_growth_rate(&history);


    printf(
        "Growth rate: %.2f KB/s\n",
        growth
    );


    /*
     * Expected:
     *
     * (110000 - 100000) / (110 - 100)
     *
     * = 1000 KB/s
     */
    assert(growth == 1000.0);


    printf("ANALYZER TEST PASS\n");


    return 0;
}