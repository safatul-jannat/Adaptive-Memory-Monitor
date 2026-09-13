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


    printf("DEVIATION TEST 1 PASS\n");


    /*
     * Test 2:
     * Memory release below history mean.
     *
     * Downward deviation is NOT abnormal
     * growth evidence.
     *
     * 300000
     * 300000
     * 100000
     */

    ProcessHistory history2;

    history_init(&history2);


    MemorySample d = {
        .rss_kb = 300000,
        .timestamp = 0
    };

    MemorySample e = {
        .rss_kb = 300000,
        .timestamp = 10
    };

    MemorySample f = {
        .rss_kb = 100000,
        .timestamp = 20
    };


    history_add_sample(&history2, &d);
    history_add_sample(&history2, &e);
    history_add_sample(&history2, &f);


    double downward_deviation =
        calculate_historical_deviation(
            &history2
        );


    printf(
        "Downward deviation: %.4f\n",
        downward_deviation
    );


    assert(downward_deviation == 0.0);


    printf("DEVIATION TEST 2 PASS\n");


    printf("ALL DEVIATION TESTS PASS\n");


    return 0;
}