#include <stdio.h>
#include <string.h>

#include "logger.h"


static char g_output_path[512] = {0};


int logger_init(const char *path)
{
    if (path == NULL)
    {
        return -1;
    }

    snprintf(g_output_path, sizeof(g_output_path), "%s", path);

    FILE *file = fopen(g_output_path, "w");

    if (file == NULL)
    {
        return -1;
    }

    fprintf(
        file,
        "timestamp,pid,process_name,rss_kb,vmsize_kb,"
        "growth_rate,sustained_growth,retention_score,"
        "historical_deviation,risk_score,risk_level,"
        "sampling_interval,strategy\n"
    );

    fclose(file);

    return 0;
}


static const char *risk_string(RiskLevel level)
{
    switch (level)
    {
        case RISK_NORMAL:
            return "NORMAL";
        case RISK_SUSPICIOUS:
            return "SUSPICIOUS";
        case RISK_HIGH:
            return "HIGH";
        case RISK_CRITICAL:
            return "CRITICAL";
    }

    return "NORMAL";
}


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
)
{
    if (sample == NULL || g_output_path[0] == '\0')
    {
        return;
    }

    FILE *file;

    file = fopen(g_output_path, "a");

    if (file == NULL)
    {
        return;
    }

    fprintf(
        file,
        "%ld,%d,%s,%lu,%lu,%.6f,%.6f,%.6f,%.6f,%.6f,%s,%d,%s\n",
        sample->timestamp,
        sample->pid,
        sample->name,
        sample->rss_kb,
        sample->vmsize_kb,
        growth_rate,
        sustained_growth,
        retention_score,
        historical_deviation,
        risk_score,
        risk_string(risk_level),
        sampling_interval,
        strategy
    );

    fclose(file);
}