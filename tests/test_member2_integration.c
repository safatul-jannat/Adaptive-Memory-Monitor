#include <assert.h>
#include <stdio.h>

#include "../src/history.h"
#include "../src/analyzer.h"
#include "../src/risk_engine.h"
#include "../src/scheduler.h"


int main(void)
{
    ProcessHistory history;
    SchedulerState scheduler;


    /*
     * Initialize history and scheduler.
     */
    history_init(&history);
    scheduler_init(&scheduler);


    /*
     * Synthetic sustained memory growth.
     *
     * RSS increases by 10240 KB every
     * 10 seconds.
     *
     * Growth rate:
     *
     * 10240 KB / 10 sec
     * = 1024 KB/sec
     *
     * Designated initializers are used
     * so we do not depend on the exact
     * field order inside MemorySample.
     */
    MemorySample samples[] =
    {
        {
            .timestamp = 0,
            .rss_kb = 10000
        },

        {
            .timestamp = 10,
            .rss_kb = 20240
        },

        {
            .timestamp = 20,
            .rss_kb = 30480
        },

        {
            .timestamp = 30,
            .rss_kb = 40720
        },

        {
            .timestamp = 40,
            .rss_kb = 50960
        }
    };


    size_t sample_count =
        sizeof(samples) / sizeof(samples[0]);


    /*
     * Add all synthetic samples
     * to process history.
     */
    for (size_t i = 0; i < sample_count; i++)
    {
        int result =
            history_add_sample(
                &history,
                &samples[i]
            );

        assert(result == 0);
    }


    /*
     * --------------------------------
     * STEP 1:
     * Analyze memory behavior
     * --------------------------------
     */

    double growth_rate =
        calculate_growth_rate(
            &history
        );


    double sustained_growth =
        calculate_sustained_growth(
            &history,
            60
        );


    double retention =
        calculate_retention_score(
            &history,
            60
        );


    double deviation =
        calculate_historical_deviation(
            &history
        );


    printf(
        "Growth rate: %.2f KB/s\n",
        growth_rate
    );

    printf(
        "Sustained growth: %.2f\n",
        sustained_growth
    );

    printf(
        "Retention: %.2f\n",
        retention
    );

    printf(
        "Historical deviation: %.2f\n",
        deviation
    );


    /*
     * Basic analyzer checks.
     */
    assert(growth_rate > 0.0);

    assert(sustained_growth > 0.0);

    assert(retention > 0.0);


    /*
     * --------------------------------
     * STEP 2:
     * Calculate risk
     * --------------------------------
     */

    double risk_score =
        calculate_risk_from_features(
            growth_rate,
            sustained_growth,
            retention,
            deviation
        );


    RiskLevel level =
        classify_risk(
            risk_score
        );


    printf(
        "Risk score: %.2f\n",
        risk_score
    );

    printf(
        "Risk level: %d\n",
        level
    );


    /*
     * Sustained strong growth should
     * result in High or Critical risk.
     */
    assert(
        level == RISK_HIGH ||
        level == RISK_CRITICAL
    );


    /*
     * --------------------------------
     * STEP 3:
     * Adaptive scheduler
     * --------------------------------
     */

    int interval =
        scheduler_update(
            &scheduler,
            level
        );


    printf(
        "Sampling interval: %d seconds\n",
        interval
    );


    /*
     * Risk escalation should happen
     * immediately.
     */
    if (level == RISK_HIGH)
    {
        assert(interval == 2);
    }


    if (level == RISK_CRITICAL)
    {
        assert(interval == 1);
    }


    printf(
        "Member 2 integration test passed.\n"
    );


    return 0;
}