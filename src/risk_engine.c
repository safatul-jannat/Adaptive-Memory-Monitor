#include "risk_engine.h"


/*
 * Initial experimental normalization value.
 *
 * 1024 KB/s = 1 MB/s
 * is currently treated as maximum growth evidence.
 *
 * This value can be tuned later using
 * controlled workload experiments.
 */
#define GROWTH_RATE_FULL_SCALE_KB_PER_SEC 1024.0


static double clamp_0_100(double value)
{
    if (value < 0.0)
    {
        return 0.0;
    }

    if (value > 100.0)
    {
        return 100.0;
    }

    return value;
}


static double normalize_growth_rate(double growth_rate)
{
    if (growth_rate <= 0.0)
    {
        return 0.0;
    }

    double score =
        (growth_rate /
         GROWTH_RATE_FULL_SCALE_KB_PER_SEC)
        * 100.0;

    return clamp_0_100(score);
}


static double normalize_ratio(double value)
{
    if (value <= 0.0)
    {
        return 0.0;
    }

    if (value >= 1.0)
    {
        return 100.0;
    }

    return value * 100.0;
}


static double normalize_deviation(double deviation)
{
    /*
     * deviation = 1.0 means the current RSS
     * differs from historical mean by 100%.
     */
    return normalize_ratio(deviation);
}


double calculate_risk_score(
    double growth_score,
    double sustained_score,
    double retention_score,
    double deviation_score
)
{
    growth_score =
        clamp_0_100(growth_score);

    sustained_score =
        clamp_0_100(sustained_score);

    retention_score =
        clamp_0_100(retention_score);

    deviation_score =
        clamp_0_100(deviation_score);


    double score =
        (0.35 * growth_score) +
        (0.30 * sustained_score) +
        (0.20 * retention_score) +
        (0.15 * deviation_score);


    return clamp_0_100(score);
}


double calculate_risk_from_features(
    double growth_rate,
    double sustained_growth,
    double retention,
    double deviation
)
{
    double growth_score =
        normalize_growth_rate(growth_rate);

    double sustained_score =
        normalize_ratio(sustained_growth);

    double retention_score =
        normalize_ratio(retention);

    double deviation_score =
        normalize_deviation(deviation);


    return calculate_risk_score(
        growth_score,
        sustained_score,
        retention_score,
        deviation_score
    );
}


RiskLevel classify_risk(
    double risk_score
)
{
    if (risk_score <= 30.0)
    {
        return RISK_NORMAL;
    }

    if (risk_score <= 60.0)
    {
        return RISK_SUSPICIOUS;
    }

    if (risk_score <= 80.0)
    {
        return RISK_HIGH;
    }

    return RISK_CRITICAL;
}