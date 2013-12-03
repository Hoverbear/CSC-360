/*
 * cpusched.c
 *
 * Skeleton code for solution to A#3, CSC 360, Fall 2013
 *
 * Prepared by: Michael Zastre (University of Victoria) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_LENGTH 100

#define FCFS 0
#define PS   1
#define RR 2
#define STRIDE 3

#define PRIORITY_LEVELS 4


/*
 * Stores raw event data from the input,
 * and has spots for per-task statistics.
 * You may want to modify this if you wish
 * to store other per-task statistics in
 * the same spot.
 */

typedef struct Task_t {
  int   arrival_time;
  float length;
  int   priority;

  float finish_time;
  int   schedulings;
  float cpu_cycles;
  // For STRIDE
  float meter_rate;
  float metered_time;
  // For RR
  int position;
} task_t; 


/*
 * Some function prototypes.
 */

void read_task_data(void);
void init_simulation_data(int);
void first_come_first_serve(void);
void stride_scheduling(int);
void priority_scheduling(void);
void mlfq_scheduling(int);
void run_simulation(int, int);
void compute_and_print_stats(void);


/*
 * Some global vars.
 */
int     num_tasks = 0;
task_t *tasks = NULL;


void read_task_data()
{
  int max_tasks = 2;
  int  in_task_num, in_task_arrival, in_task_priority;
  float in_task_length;


  assert( tasks == NULL );

  tasks = (task_t *)malloc(sizeof(task_t) * max_tasks);
  if (tasks == NULL) {
    fprintf(stderr, "error: malloc failure in read_task_data()\n");
    exit(1);
  }

  num_tasks = 0;

  /* Given the format of the input is strictly formatted,
   * we can used fscanf .
   */
  while (!feof(stdin)) {
    fscanf(stdin, "%d %d %f %d\n", &in_task_num,
        &in_task_arrival, &in_task_length, &in_task_priority);
    assert(num_tasks == in_task_num);
    tasks[num_tasks].arrival_time = in_task_arrival;
    tasks[num_tasks].length       = in_task_length;
    tasks[num_tasks].priority     = in_task_priority;

    num_tasks++;
    if (num_tasks >= max_tasks) {
      max_tasks *= 2;
      tasks = (task_t *)realloc(tasks, sizeof(task_t) * max_tasks);
      if (tasks == NULL) {
        fprintf(stderr, "error: malloc failure in read_task_data()\n");
        exit(1);
      } 
    }
  }
}


void init_simulation_data(int algorithm)
{
  int i;

  for (i = 0; i < num_tasks; i++) {
    tasks[i].finish_time = 0.0;
    tasks[i].schedulings = 0;
    tasks[i].cpu_cycles = 0.0;
  }
}


void first_come_first_serve() 
{
  int current_task = 0;
  int current_tick = 0;

  for (;;) {
    current_tick++;

    if (current_task >= num_tasks) {
      break;
    }

    /*
     * Is there even a job here???
     */
    if (tasks[current_task].arrival_time > current_tick-1) {
      continue;
    }

    tasks[current_task].cpu_cycles += 1.0;

    if (tasks[current_task].cpu_cycles >= tasks[current_task].length) {
      float quantum_fragment = tasks[current_task].cpu_cycles -
        tasks[current_task].length;
      tasks[current_task].cpu_cycles = tasks[current_task].length;
      tasks[current_task].finish_time = current_tick - quantum_fragment;
      tasks[current_task].schedulings = 1;
      current_task++;
      if (current_task > num_tasks) {
        break;
      }
      tasks[current_task].cpu_cycles += quantum_fragment;
    }
  }
}


void stride_scheduling(int quantum)
{
  int done_tasks = 0;
  int current_tick = 1;
  float total_bribes = 0;
  // Set up meter times.
  for (int i=0; i < num_tasks; i++) {
    total_bribes += 11 - tasks[i].priority; // 10 + 1 for Divide by zero.
    tasks[i].meter_rate = 1 / (4 - tasks[i].priority); // For divide by zero.
    tasks[i].metered_time = 0;
  }
  float total_rate = 1 / total_bribes;
  fprintf(stderr, "Bribes: %4.2f, Rate: %4.2f\n", total_bribes, total_rate);
  float total_meter = total_rate * quantum;
  int last_task = -1;
  for (;;) {
    fprintf(stderr, "Processing tick %d\n", current_tick);
    float tick_left = quantum;
    // While tick_left > 0.0
    while (tick_left > 0.0) {
      int target_task = -1;
      // Set the target task.
      for (int i = 0; i < num_tasks; i++) {
        // Is is ready?
        int arrived = (tasks[i].arrival_time <= current_tick + quantum - tick_left);
        int done = tasks[i].cpu_cycles >= tasks[i].length;
        if (!arrived || done) {
          continue; // If not, drop out.
        }
        // If the task just arrived, need to set it's metered time.
        if (arrived && tasks[i].metered_time == 0 && tasks[i].arrival_time == current_tick + quantum - tick_left) {
          tasks[i].metered_time = total_meter + tasks[i].meter_rate;
          fprintf(stderr, "Task[%d] arrived and got a meter of %4.2f\n", i, tasks[i].metered_time);
        }
        // If this task has the lowest metered time, set it as active.
        if (target_task == -1 || tasks[target_task].metered_time > tasks[i].metered_time) {
          fprintf(stderr, "Setting target to %d\n", i);
          target_task = i;
        }
      }
      if (target_task == -1) { tick_left -= 0.1; continue; } else if (target_task != last_task) {
        tasks[target_task].schedulings++;
        last_task = target_task;
        fprintf(stderr, "     This is a new scheduling\n!");
      }
      fprintf(stderr, " Task is %d, priority %d\n", target_task, tasks[target_task].priority);
      // Determine how much time it can has. (Up to quantum)
      float desired_time = tasks[target_task].length - tasks[target_task].cpu_cycles;
      if (desired_time > tick_left) {
        desired_time = tick_left;
      }
      fprintf(stderr, "   Consuming %4.2f, has %4.2f\n", desired_time, tasks[target_task].cpu_cycles);
      // Increment it's cycles by that amount.
      tasks[target_task].cpu_cycles += desired_time;
      // Decrement the tick_left by the amount of time taken.
      tick_left -= desired_time;
      // Up it's metered_time by meter_rate.
      tasks[target_task].metered_time += tasks[target_task].meter_rate;
      // Is it done?
      if (tasks[target_task].cpu_cycles >= tasks[target_task].length) {
        done_tasks++;
        tasks[target_task].finish_time = current_tick + quantum - tick_left;
      }
    }
    // End while
    
    //
    if (done_tasks >= num_tasks) {
      break;
    }
    current_tick += quantum;
    total_meter += (total_rate * quantum);
  }
}

void priority_scheduling()
{
  int current_tick = 0;
  int last_task = -1;
  int done_tasks = 0;
  for (;;) {
    current_tick++;
    fprintf(stderr, "Processing tick %d\n", current_tick);
    if (done_tasks >= num_tasks) {
      break;
    }
    // --- //
    float tick_left = 1.0;
    // While we haven't consumed the entire tick.
    while (tick_left != 0.0) {
    //   Get the highest priority task that's arrived and not done.
      int target_task = 0;
      int found = 0;
      for (int i=0; i < num_tasks; i++) {
        int arrived = (tasks[i].arrival_time <= current_tick + 1 - tick_left);
        int done = (tasks[i].cpu_cycles >= tasks[i].length);
        int higher_priority = (tasks[i].priority <= tasks[target_task].priority);
        if (arrived && !done && (higher_priority || !found)) {
          target_task = i;
          found = 1;
          fprintf(stderr, " Setting %d to active task\n", target_task);
        }
      }
      if (!found) { fprintf(stderr, " Found nothing...\n"); break; }
      fprintf(stderr, " %d is consuming tick %4.2f is left.\n", target_task, tick_left);
    //   If it's not the same one as before, increment the number of times it scheduled.
      if (target_task != last_task) {
        tasks[target_task].schedulings++;
        fprintf(stderr, "   Increasing the number of scheddulings for %d\n", target_task);
      }
      last_task = target_task;
      float desired_time = (tasks[target_task].length - tasks[target_task].cpu_cycles);
    //   Consume up to the rest of the tick. Leave any leftovers.
      if (desired_time >= 1.0) {
        tasks[target_task].cpu_cycles += 1.0;
        tick_left = 0.0;
      } else {
        tasks[target_task].cpu_cycles += desired_time;
        tick_left -= desired_time;
      }
    //   If done
      if (tasks[target_task].cpu_cycles >= tasks[target_task].length) {
        fprintf(stderr, "   !!Task %d is done at %4.2f\n", target_task, current_tick + 1 - tick_left);
    //    Mark your finish time.
        tasks[target_task].finish_time = current_tick + 1.0 - tick_left;
        done_tasks++;

      }
    }
    //    If all done
    //      Get out.
    if (done_tasks == num_tasks) {
      break;
    }
    // --- //
  }
}

void rr_scheduling(int quantum)
{
  int done_tasks = 0;
  int current_tick = 1;
  int last_task = -1;
  // Positions by priority.
  int positions[4] = {0, 0, 0, 0};
  
  for (;;) {
    fprintf(stderr, "Processing tick %d\n", current_tick);
    float tick_left = quantum;
    int target_task = -1;
    // While tick_left > 0.0
    while (tick_left > 0.0) {
      // START
      int i;
      // Start at the highest priority, travel down.
      for (i=0; i<4; i++) {
        // See if any tasks exist at this priority level which are eligible.
        int step = 0;
        while (step <= num_tasks) {
          int this_spot = (positions[i] + step) % num_tasks;
          // Is is ready?
          int arrived = (tasks[this_spot].arrival_time <= current_tick - (quantum - tick_left));
          int done = tasks[this_spot].cpu_cycles >= tasks[this_spot].length;
          int is_this_priority = tasks[this_spot].priority == i;
          if (arrived && !done && is_this_priority) {
            fprintf(stderr, "Found a task at priority %d, task is %d. Selecting...\n", i, this_spot);
            last_task = target_task;
            target_task = this_spot;
            positions[i] = target_task;
            break;
          }
          if (target_task != -1) {
            break;
          }
          step++;
        }
      }
      if (target_task != last_task) {
        tasks[target_task].schedulings++;
      }
      // END
      

      fprintf(stderr, " Task is %d\n", target_task);
      if (target_task == -1) { break; }
      // Determine how much time it can has. (Up to quantum)
      float desired_time = tasks[target_task].length - tasks[target_task].cpu_cycles;
      if (desired_time > tick_left) {
        desired_time = tick_left;
      }
      fprintf(stderr, "   Consuming %4.2f, has %4.2f\n", desired_time, tasks[target_task].cpu_cycles);
      // Increment it's cycles by that amount.
      tasks[target_task].cpu_cycles += desired_time;
      // Decrement the tick_left by the amount of time taken.
      tick_left -= desired_time;
      // Is it done?
      if (tasks[target_task].cpu_cycles >= tasks[target_task].length) {
        done_tasks++;
        tasks[target_task].finish_time = current_tick + quantum - tick_left;
      }
    }
    // End while
    
    //
    if (done_tasks >= num_tasks) {
      break;
    }
    current_tick += quantum;
  }
}

void run_simulation(int algorithm, int quantum)
{
  switch(algorithm) {
    case STRIDE:
      stride_scheduling(quantum);
      break;
    case PS:
      priority_scheduling();
      break;
    case RR:
      rr_scheduling(quantum);
      break;
    case FCFS:
    default:
      first_come_first_serve();
      break;
  }
}


void compute_and_print_stats()
{
  int tasks_at_level[PRIORITY_LEVELS] = {0,};
  float response_at_level[PRIORITY_LEVELS] = {0.0, };
  int scheduling_events = 0;
  int i;

  for (i = 0; i < num_tasks; i++) {
    tasks_at_level[tasks[i].priority]++;
    response_at_level[tasks[i].priority] += 
      tasks[i].finish_time - (tasks[i].arrival_time * 1.0);
    scheduling_events += tasks[i].schedulings;

    printf("Task %3d: cpu time (%4.1f), response time (%4.1f), waiting (%4.1f), schedulings (%5d)\n",
        i, tasks[i].length,
        tasks[i].finish_time - tasks[i].arrival_time,
        tasks[i].finish_time - tasks[i].arrival_time - tasks[i].cpu_cycles,
        tasks[i].schedulings);

  }

  printf("\n");

  if (num_tasks > 0) {
    for (i = 0; i < PRIORITY_LEVELS; i++) {
      if (tasks_at_level[i] == 0) {
        response_at_level[i] = 0.0;
      } else {
        response_at_level[i] /= tasks_at_level[i];
      }
      printf("Priority level %d: average response time (%4.1f)\n",
          i, response_at_level[i]);
    }
  }

  printf ("Total number of scheduling events: %d\n", scheduling_events);
}


int main(int argc, char *argv[])
{
  int i = 0;
  int algorithm = FCFS;
  int quantum = 1;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-q") == 0) {
      i++;
      quantum = atoi(argv[i]);
    } else if (strcmp(argv[i], "-a") == 0) {
      i++;
      if (strcmp(argv[i], "FCFS") == 0) {
        algorithm = FCFS;
      } else if (strcmp(argv[i], "PS") == 0) {
        algorithm = PS;
      } else if (strcmp(argv[i], "RR") == 0) {
        algorithm = RR;
      } else if (strcmp(argv[i], "STRIDE") == 0) {
        algorithm = STRIDE;
      }
    }
  }

  read_task_data();

  if (num_tasks == 0) {
    fprintf(stderr,"%s: no tasks for the simulation\n", argv[0]);
    exit(1);
  }

  init_simulation_data(algorithm);
  run_simulation(algorithm, quantum);
  compute_and_print_stats();

  exit(0);
}
