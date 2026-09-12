#include <stddef.h>
#include "scheduler.h"



#define NORMAL_INTERVAL 10
#define SUSPICIOUS_INTERVAL 5
#define HIGH_INTERVAL 2
#define CRITICAL_INTERVAL 1


#define LOWER_RISK_LIMIT 3



void scheduler_init(
    SchedulerState *state
)
{
    if (state == NULL)
    {
        return;
    }


    state->current_level = RISK_NORMAL;

    state->current_interval =
        NORMAL_INTERVAL;

    state->lower_risk_count = 0;
}



int scheduler_get_interval(
    RiskLevel level
)
{
    switch(level)
    {
        case RISK_NORMAL:
            return NORMAL_INTERVAL;


        case RISK_SUSPICIOUS:
            return SUSPICIOUS_INTERVAL;


        case RISK_HIGH:
            return HIGH_INTERVAL;


        case RISK_CRITICAL:
            return CRITICAL_INTERVAL;


        default:
            return NORMAL_INTERVAL;
    }
}



int scheduler_update(
    SchedulerState *state,
    RiskLevel new_level
)
{
    if (state == NULL)
    {
        return NORMAL_INTERVAL;
    }



    /*
     * Risk increased:
     * immediately increase monitoring.
     */
    if (new_level > state->current_level)
    {
        state->current_level = new_level;

        state->current_interval =
            scheduler_get_interval(new_level);

        state->lower_risk_count = 0;

        return state->current_interval;
    }



    /*
     * Risk decreased:
     * require stable lower risk.
     */
    if (new_level < state->current_level)
    {
        state->lower_risk_count++;


        if (state->lower_risk_count >= LOWER_RISK_LIMIT)
        {
            state->current_level = new_level;

            state->current_interval =
                scheduler_get_interval(new_level);

            state->lower_risk_count = 0;
        }


        return state->current_interval;
    }



    /*
     * Same risk level.
     */
    state->lower_risk_count = 0;


    return state->current_interval;
}