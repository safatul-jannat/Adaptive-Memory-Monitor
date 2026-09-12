#ifndef ANALYZER_H
#define ANALYZER_H

#include "history.h"

/*
 * Calculate memory growth rate between
 * the latest two samples.
 *
 * Returns:
 *   growth rate in KB/second
 *
 * Returns 0 if:
 *   - fewer than 2 samples exist
 *   - timestamps are invalid
 */
double calculate_growth_rate(
    const ProcessHistory *history
);
/*
 * Calculate sustained RSS growth ratio
 * inside a time window.
 *
 * Return:
 *     0.0 - 1.0
 *
 * 0.0 = no sustained growth
 * 1.0 = every interval increased
 */
double calculate_sustained_growth(
    const ProcessHistory *history,
    time_t window_seconds
);
/*
 * Calculate memory retention score.
 *
 * Return:
 *     0.0 - 1.0
 *
 * 0.0 = memory mostly released
 * 1.0 = memory retained
 */
double calculate_retention_score(
    const ProcessHistory *history,
    time_t window_seconds
);
/*
 * Calculate deviation of newest RSS
 * from historical average RSS.
 *
 * Return:
 *     0.0 = no deviation
 *     higher = more unusual
 */
double calculate_historical_deviation(
    const ProcessHistory *history
);
#endif