#include <stdio.h>

#include "logger.h"


#define LOG_FILE "memory_log.csv"


void log_sample(MemorySample *sample)
{
    FILE *file;


    file = fopen(LOG_FILE, "a");


    if(file == NULL)
    {
        return;
    }


    /*
     * Add header if the file is empty.
     */

    fseek(file, 0, SEEK_END);

    if(ftell(file) == 0)
    {
        fprintf(
            file,
            "timestamp,pid,name,rss_kb,vmsize_kb\n"
        );
    }


    /*
     * Write one memory observation.
     */

    fprintf(
        file,
        "%ld,%d,%s,%lu,%lu\n",
        sample->timestamp,
        sample->pid,
        sample->name,
        sample->rss_kb,
        sample->vmsize_kb
    );


    fclose(file);
}