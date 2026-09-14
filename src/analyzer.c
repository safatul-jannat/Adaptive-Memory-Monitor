#include "analyzer.h"


/*
 * Experimental evidence constants:
 *
 * A single positive RSS increase must not
 * be treated as sustained abnormal growth.
 *
 * At least MIN_POSITIVE_GROWTH_INTERVALS
 * positive intervals inside the window are
 * required, and the positive-to-total ratio
 * must reach MIN_SUSTAINED_GROWTH_RATIO.
 */
#define MIN_POSITIVE_GROWTH_INTERVALS 3
#define MIN_SUSTAINED_GROWTH_RATIO 0.75


double calculate_growth_rate(
    const ProcessHistory *history
)
{
    size_t count = history_count(history);

    /*
     * Need at least two samples:
     *
     * old sample
     * new sample
     */
    if (count < 2)
    {
        return 0.0;
    }


    MemorySample previous;
    MemorySample current;


    /*
     * Get the last two samples
     */
    if (history_get_sample(
            history,
            count - 2,
            &previous) != 0)
    {
        return 0.0;
    }


    if (history_get_sample(
            history,
            count - 1,
            &current) != 0)
    {
        return 0.0;
    }


    /*
     * Avoid division by zero
     */
    if (current.timestamp <= previous.timestamp)
    {
        return 0.0;
    }


    double time_difference =
        (double)(current.timestamp - previous.timestamp);


    double rss_difference =
        (double)current.rss_kb -
        (double)previous.rss_kb;


    return rss_difference / time_difference;
}
double calculate_sustained_growth(
    const ProcessHistory *history,
    time_t window_seconds
)
{
    size_t count = history_count(history);

    if (count < 2)
    {
        return 0.0;
    }


    MemorySample newest;

    if (history_get_sample(
            history,
            count - 1,
            &newest) != 0)
    {
        return 0.0;
    }


    int total_intervals = 0;
    int positive_growth = 0;


    /*
     * Walk backwards through history
     * until window limit is reached
     */
    for (size_t i = count - 1; i > 0; i--)
    {
        MemorySample older;
        MemorySample newer;


        if (history_get_sample(
                history,
                i - 1,
                &older) != 0)
        {
            continue;
        }


        if (history_get_sample(
                history,
                i,
                &newer) != 0)
        {
            continue;
        }


        /*
         * Stop when outside time window
         */
        if ((newest.timestamp - older.timestamp)
            > window_seconds)
        {
            break;
        }


        total_intervals++;


        if (newer.rss_kb > older.rss_kb)
        {
            positive_growth++;
        }
    }


    if (total_intervals == 0)
    {
        return 0.0;
    }


    if (positive_growth < MIN_POSITIVE_GROWTH_INTERVALS)
    {
        return 0.0;
    }


    double ratio =
        (double)positive_growth /
        (double)total_intervals;


    if (ratio < MIN_SUSTAINED_GROWTH_RATIO)
    {
        return 0.0;
    }


    return ratio;
}
double calculate_retention_score(
    const ProcessHistory *history,
    time_t window_seconds
)
{
    size_t count = history_count(history);

    if (count < 2)
    {
        return 0.0;
    }


    MemorySample newest;

    if (history_get_sample(
            history,
            count - 1,
            &newest) != 0)
    {
        return 0.0;
    }


    int total_intervals = 0;
    int positive_growth = 0;

    unsigned long total_positive_growth = 0;

    unsigned long oldest_rss = 0;
    int found_first_interval = 0;


    /*
     * Walk forward through history
     * keeping only chronological intervals
     * inside the requested time window.
     */
    for (size_t i = 1; i < count; i++)
    {
        MemorySample older;
        MemorySample newer;


        if (history_get_sample(
                history,
                i - 1,
                &older) != 0)
        {
            continue;
        }


        if (history_get_sample(
                history,
                i,
                &newer) != 0)
        {
            continue;
        }


        /*
         * Stop when outside time window.
         */
        if ((newest.timestamp - older.timestamp)
            > window_seconds)
        {
            continue;
        }


        if (!found_first_interval)
        {
            oldest_rss = older.rss_kb;
            found_first_interval = 1;
        }


        total_intervals++;


        if (newer.rss_kb > older.rss_kb)
        {
            positive_growth++;

            total_positive_growth +=
                newer.rss_kb - older.rss_kb;
        }
    }


    /*
     * Use the same evidence rules as
     * sustained growth.
     */
    if (total_intervals == 0)
    {
        return 0.0;
    }


    if (positive_growth < MIN_POSITIVE_GROWTH_INTERVALS)
    {
        return 0.0;
    }


    double growth_ratio =
        (double)positive_growth /
        (double)total_intervals;


    if (growth_ratio < MIN_SUSTAINED_GROWTH_RATIO)
    {
        return 0.0;
    }


    if (total_positive_growth == 0)
    {
        return 0.0;
    }


    /*
     * Avoid unsigned subtraction underflow.
     */
    if (newest.rss_kb <= oldest_rss)
    {
        return 0.0;
    }


    double net_retained_growth =
        (double)newest.rss_kb -
        (double)oldest_rss;


    double retention =
        net_retained_growth /
        (double)total_positive_growth;


    if (retention < 0.0)
    {
        retention = 0.0;
    }


    if (retention > 1.0)
    {
        retention = 1.0;
    }


    return retention;
}
double calculate_historical_deviation(
    const ProcessHistory *history
)
{
    size_t count = history_count(history);


    if (count < 2)
    {
        return 0.0;
    }


    unsigned long total_rss = 0;


    for (size_t i = 0; i < count; i++)
    {
        MemorySample sample;


        if (history_get_sample(
                history,
                i,
                &sample) != 0)
        {
            continue;
        }


        total_rss += sample.rss_kb;
    }


    double mean_rss =
        (double)total_rss / count;


    MemorySample current;


    if (history_get_sample(
            history,
            count - 1,
            &current) != 0)
    {
        return 0.0;
    }


    if (mean_rss == 0)
    {
        return 0.0;
    }


    if (current.rss_kb <= mean_rss)
    {
        return 0.0;
    }


    return
        ((double)current.rss_kb - mean_rss) /
        mean_rss;
}