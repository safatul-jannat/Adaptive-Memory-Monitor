#include "analyzer.h"


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


    return (double)positive_growth /
           (double)total_intervals;
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


    unsigned long min_rss = newest.rss_kb;
    unsigned long max_rss = newest.rss_kb;


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


        if ((newest.timestamp - sample.timestamp)
            <= window_seconds)
        {
            if (sample.rss_kb < min_rss)
            {
                min_rss = sample.rss_kb;
            }


            if (sample.rss_kb > max_rss)
            {
                max_rss = sample.rss_kb;
            }
        }
    }


    if (max_rss == min_rss)
    {
        return 0.0;
    }


    return
        (double)(newest.rss_kb - min_rss) /
        (double)(max_rss - min_rss);
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


    double difference =
        (double)current.rss_kb -
        mean_rss;


    if (difference < 0)
    {
        difference = -difference;
    }


    return difference / mean_rss;
}