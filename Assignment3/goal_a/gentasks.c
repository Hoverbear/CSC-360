/*
 * gentasks.c
 *
 * Driver code for A#3, CSC 360 Fall 2013
 *
 * Prepared by: Michael Zastre (University of Victoria) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define SHORT_TASK_MIN 3
#define SHORT_TASK_MAX 20
#define LONG_TASK_MIN  100
#define LONG_TASK_MAX  250
#define SHORT_LONG_THRESHOLD 0.8

#define SHORT_LOW_PRIORITY 0
#define SHORT_HIGH_PRIORITY 1
#define SHORT_LOW_HIGH_THRESHOLD 0.3

#define LONG_LOW_PRIORITY 3
#define LONG_HIGH_PRIORITY 2
#define LONG_LOW_HIGH_THRESHOLD 0.9

#define ARRIVAL_TIME_RANGE_LARGE 15
#define ARRIVAL_TIME_RANGE_SMALL 5
#define LARGE_SMALL_THRESHOLD 0.5



float generate_task_length()
{
    float short_long_choice = rand();
    float length = rand();

    if ((short_long_choice / RAND_MAX) > SHORT_LONG_THRESHOLD) {
        return (length / RAND_MAX * (LONG_TASK_MAX - LONG_TASK_MIN) + LONG_TASK_MIN);
    } else {
        return (length / RAND_MAX * (SHORT_TASK_MAX - SHORT_TASK_MIN) + SHORT_TASK_MIN);
    }
}


int generate_priority(float length)
{
    float high_low_choice = rand();
    
    high_low_choice /= RAND_MAX;

    if (length <= SHORT_TASK_MAX) {
        if  (high_low_choice < SHORT_LOW_HIGH_THRESHOLD) {
            return SHORT_LOW_PRIORITY;
        } else {
            return SHORT_HIGH_PRIORITY;
        }
    }

    if (length >= LONG_TASK_MIN) {
        if (high_low_choice < LONG_LOW_HIGH_THRESHOLD) {
            return LONG_LOW_PRIORITY;
        } else {
            return LONG_HIGH_PRIORITY;
        }
    }

    /*
     * Why the dickens are we here???
     */
    assert(0);
}


int generate_arrival_interval()
{
    float large_small_choice = rand();
    large_small_choice /= RAND_MAX;

    float length = rand();

    if (large_small_choice < LARGE_SMALL_THRESHOLD) {
        return (int)(length / RAND_MAX * (ARRIVAL_TIME_RANGE_SMALL - 1) + 1);
    } else {
        return (int)(length / RAND_MAX * (ARRIVAL_TIME_RANGE_LARGE - 1) + 1);
    }
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 3) {
        fprintf(stderr, "usage: %s <number of tasks> <random seed>\n", argv[0]);
        exit(1); 
    }
   
    int num_tasks = atoi(argv[1]);
    int random_seed = atoi(argv[2]);
    srand(random_seed);

    int   task_arrival_time = 0;
    float task_length = 0.0;
    int   task_priority = 0;
    for (i = 0; i < num_tasks; i++) {
        task_arrival_time += generate_arrival_interval();
        task_length = generate_task_length();
        task_priority = generate_priority(task_length);

        printf("%d %d %.1f %d\n", i, task_arrival_time, task_length, task_priority);
    }

    exit(0);
}
