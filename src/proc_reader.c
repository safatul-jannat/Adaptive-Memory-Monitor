#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "proc_reader.h"



int read_process_memory(int pid, MemorySample *sample)
{
    char path[256];
    char line[256];

    FILE *file;


    // Create path:
    // /proc/<PID>/status

    snprintf(
        path,
        sizeof(path),
        "/proc/%d/status",
        pid
    );


    // Open status file

    file = fopen(path, "r");


    if(file == NULL)
    {
        // Process may have exited
        return -1;
    }



    // Store PID

    sample->pid = pid;



    // Read status file line by line

    while(fgets(line, sizeof(line), file))
    {


        /*
            Example line:

            Name:   firefox

        */

        if(strncmp(line, "Name:", 5) == 0)
        {
            sscanf(
                line,
                "Name:\t%s",
                sample->name
            );
        }



        /*
            Example:

            VmRSS:  120000 kB

        */

        else if(strncmp(line, "VmRSS:", 6) == 0)
        {
            sscanf(
                line,
                "VmRSS:\t%lu",
                &sample->rss_kb
            );
        }



        /*
            Example:

            VmSize: 300000 kB

        */

        else if(strncmp(line, "VmSize:", 7) == 0)
        {
            sscanf(
                line,
                "VmSize:\t%lu",
                &sample->vmsize_kb
            );
        }

    }



    fclose(file);



    return 0;
}