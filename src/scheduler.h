#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "risk_engine.h"


typedef struct
{
    RiskLevel current_level;

    int current_interval;

    int lower_risk_count;

} SchedulerState;



/*
 * Initialize scheduler state.
 */
void scheduler_init(
    SchedulerState *state
);



/*
 * Decide next sampling interval
 * based on risk level.
 *
 * No hysteresis.
 */
int scheduler_get_interval(
    RiskLevel level
);



/*
 * Update scheduler using hysteresis.
 *
 * Returns:
 *      new sampling interval
 */
int scheduler_update(
    SchedulerState *state,
    RiskLevel new_level
);



#endif