#include <stdio.h>
#include <assert.h>

#include "../src/history.h"
#include "../src/analyzer.h"
#include "../src/risk_engine.h"
#include "../src/scheduler.h"


int main(void)
{
    ProcessHistory history;
    SchedulerState scheduler;

    history_init(&history);
    scheduler_init(&scheduler);


    /*
     * Steady persistent growth:
     *
     * t=0   RSS=100000
     * t=10  RSS=110000
     * t=20  RSS=120000
     * t=30  RSS=130000
     * t=40  RSS=140000
     *
     * Every interval increases by 10000 KB.
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

    MemorySample s5 = {
        .rss_kb = 140000,
        .timestamp = 40
    };


    history_add_sample(&history, &s1);
    history_add_sample(&history, &s2);
    history_add_sample(&history, &s3);
    history_add_sample(&history, &s4);
    history_add_sample(&history, &s5);


    double growth_rate =
        calculate_growth_rate(&history);

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

    double risk_score =
        calculate_risk_from_features(
            growth_rate,
            sustained_growth,
            retention,
            deviation
        );

    RiskLevel level =
        classify_risk(risk_score);

    int interval =
        scheduler_update(
            &scheduler,
            level
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
        "Retention: %.4f\n",
        retention
    );

    printf(
        "Historical deviation: %.4f\n",
        deviation
    );

    printf(
        "Risk score: %.2f\n",
        risk_score
    );

    printf(
        "Risk level: %d\n",
        level
    );

    printf(
        "Sampling interval: %d seconds\n",
        interval
    );


    /*
     * Persistent repeated growth
     * should pass all evidence thresholds.
     */
    assert(sustained_growth >= 0.75);
    assert(retention > 0.8);
    assert(risk_score >= 61.0);

    assert(
        level == RISK_HIGH ||
        level == RISK_CRITICAL
    );


    if (level == RISK_HIGH)
    {
        assert(interval == 2);
    }

    if (level == RISK_CRITICAL)
    {
        assert(interval == 1);
    }


    printf(
        "\nPERSISTENT GROWTH TEST PASS\n"
    );

    return 0;
}
