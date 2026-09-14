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


    /*
 * Long-history retention test.
 *
 * The complete history is longer than the 60-second analysis
 * window. Persistent recent growth must still produce a high
 * retention score.
 */
ProcessHistory long_history;
history_init(&long_history);

MemorySample long_samples[] = {
    {.rss_kb = 100000, .timestamp = 0},
    {.rss_kb = 110000, .timestamp = 10},
    {.rss_kb = 120000, .timestamp = 20},
    {.rss_kb = 130000, .timestamp = 30},
    {.rss_kb = 140000, .timestamp = 40},
    {.rss_kb = 150000, .timestamp = 50},
    {.rss_kb = 160000, .timestamp = 60},
    {.rss_kb = 170000, .timestamp = 70},
    {.rss_kb = 180000, .timestamp = 80}
};

size_t long_count =
    sizeof(long_samples) / sizeof(long_samples[0]);

for (size_t i = 0; i < long_count; i++)
{
    history_add_sample(
        &long_history,
        &long_samples[i]
    );
}

double long_retention =
    calculate_retention_score(
        &long_history,
        60
    );

printf(
    "Long-history retention score: %.2f\n",
    long_retention
);

assert(long_retention > 0.8);

printf("LONG HISTORY RETENTION TEST PASS\n");
    return 0;
}