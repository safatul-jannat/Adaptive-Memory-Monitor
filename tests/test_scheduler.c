#include <assert.h>
#include <stdio.h>

#include "../src/scheduler.h"



int main(void)
{
    SchedulerState state;


    scheduler_init(&state);


    assert(
        scheduler_get_interval(RISK_NORMAL)
        == 10
    );


    assert(
        scheduler_get_interval(RISK_SUSPICIOUS)
        == 5
    );


    assert(
        scheduler_get_interval(RISK_HIGH)
        == 2
    );


    assert(
        scheduler_get_interval(RISK_CRITICAL)
        == 1
    );



    /*
     * Test immediate escalation.
     */

    scheduler_update(
        &state,
        RISK_HIGH
    );


    assert(
        state.current_interval == 2
    );



    /*
     * Test hysteresis.
     *
     * High → Suspicious
     *
     * First two times:
     * stay High
     *
     * Third time:
     * change
     */

    scheduler_update(
        &state,
        RISK_SUSPICIOUS
    );

    assert(
        state.current_interval == 2
    );


    scheduler_update(
        &state,
        RISK_SUSPICIOUS
    );

    assert(
        state.current_interval == 2
    );


    scheduler_update(
        &state,
        RISK_SUSPICIOUS
    );

    assert(
        state.current_interval == 5
    );



    printf("All scheduler tests passed.\n");


    return 0;
}