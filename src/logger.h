#ifndef LOGGER_H
#define LOGGER_H

#include "types.h"
#include "risk_engine.h"

/*
 * Configure the CSV output file.
 *
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int logger_init(const char *path);

/*
 * Write one full observation row to the configured CSV file.
 */
void log_observation(
    const MemorySample *sample,
    double growth_rate,
    double sustained_growth,
    double retention_score,
    double historical_deviation,
    double risk_score,
    RiskLevel risk_level,
    int sampling_interval,
    const char *strategy
);

#endif