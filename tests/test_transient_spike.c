#include <stdio.h>
#include <assert.h>

#include "../src/history.h"
#include "../src/analyzer.h"
#include "../src/risk_engine.h"


int main(void)
{
    ProcessHistory history;

    history_init(&history);


    /*
     * A single heavy allocation:
     *
     * t=0   RSS=100000
     * t=10  RSS=100000  (no change)
     * t=20  RSS=400000  (spike)
     * t=30  RSS=400000  (held)
     *
     * This is a one-off transient spike,
     * not sustained repeated growth.
     */
    MemorySample s1 = {
        .rss_kb = 100000,
        .timestamp = 0
    };

    MemorySample s2 = {
        .rss_kb = 100000,
        .timestamp = 10
    };

    MemorySample s3 = {
        .rss_kb = 400000,
        .timestamp = 20
    };

    MemorySample s4 = {
        .rss_kb = 400000,
        .timestamp = 30
    };


    history_add_sample(&history, &s1);
    history_add_sample(&history, &s2);
    history_add_sample(&history, &s3);
    history_add_sample(&history, &s4);


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


    /*
     * A single spike must NOT be treated
     * as sustained repeated growth.
     */
    assert(sustained_growth == 0.0);
    assert(retention == 0.0);
    assert(risk_score < 61.0);
    assert(level != RISK_HIGH);
    assert(level != RISK_CRITICAL);


    printf(
        "\nTRANSIENT SPIKE TEST PASS\n"
    );

    return 0;
}
