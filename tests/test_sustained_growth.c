#include <stdio.h>
#include <assert.h>

#include "../src/analyzer.h"


int main(void)
{
    ProcessHistory history;

    history_init(&history);


    /*
     * Increasing RSS:
     *
     * 100000
     * 110000
     * 120000
     * 130000
     *
     * Every interval increases.
     */
    MemorySample s1 = {
        .rss_kb = 100000,
        .timestamp = 0
    };

    MemorySample s2 = {
        .rss_kb = 110000,
        .timestamp = 10
    };

    MemorySample s3 = {
        .rss_kb = 120000,
        .timestamp = 20
    };

    MemorySample s4 = {
        .rss_kb = 130000,
        .timestamp = 30
    };


    history_add_sample(&history, &s1);
    history_add_sample(&history, &s2);
    history_add_sample(&history, &s3);
    history_add_sample(&history, &s4);


    double result =
        calculate_sustained_growth(
            &history,
            60
        );


    printf(
        "Sustained growth ratio: %.2f\n",
        result
    );


    /*
     * All intervals increased.
     */
    assert(result == 1.0);


    printf("SUSTAINED GROWTH TEST PASS\n");


    return 0;
}