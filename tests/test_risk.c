#include <assert.h>
#include <stdio.h>

#include "../src/risk_engine.h"


int main(void)
{
    double score;


    /*
     * Test normalized weighted calculation.
     */
    score = calculate_risk_score(
        100.0,
        100.0,
        100.0,
        100.0
    );

    assert(score == 100.0);
    assert(classify_risk(score) == RISK_CRITICAL);


    /*
     * Test zero risk.
     */
    score = calculate_risk_score(
        0.0,
        0.0,
        0.0,
        0.0
    );

    assert(score == 0.0);
    assert(classify_risk(score) == RISK_NORMAL);


    /*
     * Test raw analyzer-style values.
     *
     * Growth:     512 KB/s -> 50
     * Sustained:  0.80     -> 80
     * Retention:  0.50     -> 50
     * Deviation:  0.25     -> 25
     *
     * Expected risk = 55.25
     */
    score = calculate_risk_from_features(
        512.0,
        0.80,
        0.50,
        0.25
    );

    assert(score > 55.24);
    assert(score < 55.26);

    assert(
        classify_risk(score)
        == RISK_SUSPICIOUS
    );


    /*
     * Full abnormal-growth evidence.
     */
    score = calculate_risk_from_features(
        1024.0,
        1.0,
        1.0,
        1.0
    );

    assert(score == 100.0);
    assert(classify_risk(score) == RISK_CRITICAL);


    /*
     * Negative growth should not
     * contribute abnormal-growth evidence.
     */
    score = calculate_risk_from_features(
        -500.0,
        0.0,
        0.0,
        0.0
    );

    assert(score == 0.0);


    /*
     * Test classification boundaries.
     */
    assert(classify_risk(30.0) == RISK_NORMAL);

    assert(
        classify_risk(31.0)
        == RISK_SUSPICIOUS
    );

    assert(
        classify_risk(60.0)
        == RISK_SUSPICIOUS
    );

    assert(
        classify_risk(61.0)
        == RISK_HIGH
    );

    assert(
        classify_risk(80.0)
        == RISK_HIGH
    );

    assert(
        classify_risk(81.0)
        == RISK_CRITICAL
    );


    printf("All risk engine tests passed.\n");

    return 0;
}