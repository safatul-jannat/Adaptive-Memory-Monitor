#ifndef TYPES_H
#define TYPES_H

#include <time.h>

#define PROCESS_NAME_LEN 256


typedef struct {

    int pid;

    char name[PROCESS_NAME_LEN];

    unsigned long rss_kb;

    unsigned long vmsize_kb;

    time_t timestamp;

} MemorySample;


#endif