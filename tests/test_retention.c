#include <stdio.h>
#include <assert.h>

#include "../src/analyzer.h"


int main(void)
{
    /*
     * Test 1:
     * Memory keeps increasing.
     *
     * Expected:
     * retention = 1.0
     */

    ProcessHistory history1;

    history_init(&history1);


    MemorySample a = {
        .rss_kb = 100000,
        .timestamp = 0
    };

    MemorySample b = {
        .rss_kb = 200000,
        .timestamp = 10
    };

    MemorySample c = {
        .rss_kb = 300000,
        .timestamp = 20
    };

    MemorySample d = {
        .rss_kb = 400000,
        .timestamp = 30
    };


    history_add_sample(&history1, &a);
    history_add_sample(&history1, &b);
    history_add_sample(&history1, &c);
    history_add_sample(&history1, &d);


    double retained =
        calculate_retention_score(
            &history1,
            60
        );


    printf(
        "Retained growth score: %.2f\n",
        retained
    );


    assert(retained == 1.0);



    /*
     * Test 2:
     * Memory spikes then releases.
     *
     * Expected:
     * score should be low.
     */

    ProcessHistory history2;

    history_init(&history2);


    MemorySample e = {
        .rss_kb = 100000,
        .timestamp = 0
    };

    MemorySample f = {
        .rss_kb = 400000,
        .timestamp = 10
    };

    MemorySample g = {
        .rss_kb = 150000,
        .timestamp = 20
    };


    history_add_sample(&history2, &e);
    history_add_sample(&history2, &f);
    history_add_sample(&history2, &g);


    double released =
        calculate_retention_score(
            &history2,
            60
        );


    printf(
        "Released memory score: %.2f\n",
        released
    );


    assert(released < 0.5);



    printf("RETENTION TEST PASS\n");


    return 0;
}