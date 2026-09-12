#include <stdio.h>
#include <assert.h>

#include "../src/analyzer.h"


int main(void)
{
    ProcessHistory history;

    history_init(&history);


    /*
     * Normal history:
     *
     * 100000
     * 110000
     * 120000
     */

    MemorySample a = {
        .rss_kb = 100000,
        .timestamp = 0
    };

    MemorySample b = {
        .rss_kb = 110000,
        .timestamp = 10
    };

    MemorySample c = {
        .rss_kb = 120000,
        .timestamp = 20
    };


    history_add_sample(&history, &a);
    history_add_sample(&history, &b);
    history_add_sample(&history, &c);


    double deviation =
        calculate_historical_deviation(
            &history
        );


    printf(
        "Historical deviation: %.3f\n",
        deviation
    );


    /*
     * Expected:
     *
     * Mean = 110000
     * Current = 120000
     *
     * deviation ≈ 0.09
     */

    assert(deviation < 0.2);


    printf("DEVIATION TEST PASS\n");


    return 0;
}