#ifndef RISK_ENGINE_H
#define RISK_ENGINE_H


typedef enum
{
    RISK_NORMAL = 0,
    RISK_SUSPICIOUS,
    RISK_HIGH,
    RISK_CRITICAL

} RiskLevel;


/*
 * Calculate weighted risk score from
 * already normalized feature values.
 *
 * All inputs should be in range 0-100.
 */
double calculate_risk_score(
    double growth_score,
    double sustained_score,
    double retention_score,
    double deviation_score
);


/*
 * Calculate risk directly from the raw
 * analyzer feature values.
 *
 * growth_rate:
 *      KB/second
 *
 * sustained_growth:
 *      0.0 - 1.0
 *
 * retention:
 *      0.0 - 1.0
 *
 * deviation:
 *      ratio relative to historical mean
 */
double calculate_risk_from_features(
    double growth_rate,
    double sustained_growth,
    double retention,
    double deviation
);


/*
 * Convert risk score into risk level.
 */
RiskLevel classify_risk(
    double risk_score
);


#endif