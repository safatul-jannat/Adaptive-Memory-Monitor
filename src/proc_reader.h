#ifndef PROC_READER_H
#define PROC_READER_H

#include "types.h"


int read_process_memory(
        int pid,
        MemorySample *sample
);


#endif