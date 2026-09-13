#include <stdio.h>
#include <assert.h>

#include "../src/scheduler.h"


int main(void)
{
    SchedulerState stable;
    SchedulerState growing;


    scheduler_init(&stable);
    scheduler_init(&growing);


    printf(
        "Stable interval: %d\n",
        stable.current_interval
    );

    printf(
        "Growing interval: %d\n",
        growing.current_interval
    );


    assert(stable.current_interval == 10);
    assert(growing.current_interval == 10);


    /*
     * Escalate ONLY the growing state
     * to RISK_HIGH.
     */
    scheduler_update(
        &growing,
        RISK_HIGH
    );


    printf(
        "\nAfter HIGH escalation:\n"
    );

    printf(
        "Stable interval: %d\n",
        stable.current_interval
    );

    printf(
        "Growing interval: %d\n",
        growing.current_interval
    );


    assert(stable.current_interval == 10);
    assert(growing.current_interval == 2);


    /*
     * Escalate ONLY the growing state
     * to RISK_CRITICAL.
     */
    scheduler_update(
        &growing,
        RISK_CRITICAL
    );


    printf(
        "\nAfter CRITICAL escalation:\n"
    );

    printf(
        "Stable interval: %d\n",
        stable.current_interval
    );

    printf(
        "Growing interval: %d\n",
        growing.current_interval
    );


    assert(stable.current_interval == 10);
    assert(growing.current_interval == 1);


    /*
     * Verify stable risk state unchanged.
     */
    assert(
        stable.current_level == RISK_NORMAL
    );


    printf(
        "\nPER-PROCESS SCHEDULER TEST PASS\n"
    );

    return 0;
}
