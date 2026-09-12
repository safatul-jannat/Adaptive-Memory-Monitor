#include <stdio.h>
#include <stdlib.h>

#include "src/history.h"
void create_sample(MemorySample *sample, unsigned long value)
{
    sample->pid = 1000;
    sample->rss_kb = value;
    sample->vmsize_kb = value * 2;
    sample->timestamp = value;
}
int test_empty_history()
{
    ProcessHistory history;

    history_init(&history);


    if (history_count(&history) != 0)
    {
        printf("TEST 1 FAIL\n");
        return 0;
    }


    printf("TEST 1 PASS - Empty history\n");
    return 1;
}
int test_one_sample()
{
    ProcessHistory history;
    MemorySample input;
    MemorySample output;


    history_init(&history);


    create_sample(&input, 100);


    if (history_add_sample(&history, &input) != 0)
    {
        printf("TEST 2 FAIL - add sample\n");
        return 0;
    }


    if (history_count(&history) != 1)
    {
        printf("TEST 2 FAIL - count\n");
        return 0;
    }


    history_get_sample(&history, 0, &output);


    if (output.rss_kb != 100)
    {
        printf("TEST 2 FAIL - wrong value\n");
        return 0;
    }


    printf("TEST 2 PASS - One sample\n");
    return 1;
}
int test_multiple_samples()
{
    ProcessHistory history;
    MemorySample sample;
    MemorySample output;


    history_init(&history);


    unsigned long values[] = {100,200,300};


    for(int i=0;i<3;i++)
    {
        create_sample(&sample, values[i]);
        history_add_sample(&history,&sample);
    }


    for(size_t i=0;i<3;i++)
    {
        history_get_sample(&history,i,&output);

        if(output.rss_kb != values[i])
        {
            printf("TEST 3 FAIL\n");
            return 0;
        }
    }


    printf("TEST 3 PASS - Multiple order\n");
    return 1;
}
int test_overflow()
{
    ProcessHistory history;
    MemorySample sample;
    MemorySample output;


    history_init(&history);


    for(unsigned long i=1;i<=130;i++)
    {
        create_sample(&sample,i);
        history_add_sample(&history,&sample);
    }


    if(history_count(&history)!=128)
    {
        printf("TEST 4 FAIL - wrong count\n");
        return 0;
    }


    for(size_t i=0;i<128;i++)
    {
        history_get_sample(&history,i,&output);


        unsigned long expected=i+3;


        if(output.rss_kb != expected)
        {
            printf(
            "TEST 4 FAIL at index %zu expected %lu got %lu\n",
            i,
            expected,
            output.rss_kb);

            return 0;
        }
    }


    printf("TEST 4 PASS - Circular buffer\n");
    return 1;
}
int test_invalid_inputs()
{
    ProcessHistory history;
    MemorySample sample;


    create_sample(&sample,100);


    history_init(NULL);


    if(history_add_sample(NULL,&sample)!=-1)
    {
        printf("TEST 5 FAIL - NULL history\n");
        return 0;
    }


    history_init(&history);


    if(history_add_sample(&history,NULL)!=-1)
    {
        printf("TEST 5 FAIL - NULL sample\n");
        return 0;
    }


    if(history_get_sample(&history,0,&sample)!=-1)
    {
        printf("TEST 5 FAIL - invalid index\n");
        return 0;
    }


    printf("TEST 5 PASS - Invalid inputs\n");

    return 1;
}
int main()
{
    int result = 1;


    result &= test_empty_history();
    result &= test_one_sample();
    result &= test_multiple_samples();
    result &= test_overflow();
    result &= test_invalid_inputs();


    if(result)
    {
        printf("\nALL HISTORY TESTS PASS\n");
        return 0;
    }


    printf("\nSOME TESTS FAILED\n");
    return 1;
}