#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "process_discovery.h"



int get_process_list(int *pids, int max_processes)
{

    DIR *directory;

    struct dirent *entry;


    int count = 0;



    // Open /proc directory

    directory = opendir("/proc");


    if(directory == NULL)
    {
        return -1;
    }



    // Read every entry inside /proc

    while((entry = readdir(directory)) != NULL)
    {


        // Check if directory name is a PID

        int is_pid = 1;



        for(int i = 0; entry->d_name[i] != '\0'; i++)
        {

            if(!isdigit(entry->d_name[i]))
            {
                is_pid = 0;
                break;
            }

        }



        if(is_pid)
        {

            if(count < max_processes)
            {

                pids[count] = atoi(entry->d_name);

                count++;

            }

        }


    }



    closedir(directory);



    return count;
}