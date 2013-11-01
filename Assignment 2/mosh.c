/* A2 CSC 360 UVIC 2013 FALL
 * =========================
 * Andrew Hobden (V00788452)
 */

#include <stdlib.h>                   // Standard Lib.
#include <signal.h>                   // Makes kill work.
#include <stdio.h>                    // Standard I/O.
#include <sys/types.h>                // Defines data types used in system calls. 
#include <string.h>                   // String Functions.
#include <errno.h>                    // Error Numbers.
#include <unistd.h>                   // Fork.
#include <sys/wait.h>                 // Wait.
#include <readline/readline.h>        // Readline.
#include <readline/history.h>         // Readline History.
#include <sys/stat.h>                 // For Stat

#define MAX_COMMAND_LENGTH  2048      // Maximum command we expect.
#define MAX_PS1_LENGTH      255       // Maximum prompt size we expect.
#define OBSCURE_CHARACTER   '|'       // The char we exploace spaces inside quotes with.
#define PIPE_OPERATOR       "::>"     // The piping string.
#define SEQ_OPERATOR        "++"      // The sequencing string.
#define TOBACK              "toback"  // The string for backgrounding.
#define CMDALL              "cmdall"  // The string to display all processes.
#define CMDKILL             "cmdkill" // The string to kill a process.


/* A basic C array with length. */
typedef struct word_array {
  int size;
  char** items;
} word_array;

/* A process PID / Command struct */
typedef struct process {
  int pid;
  char* command;
} process;

// Some global variables.
process* processes;
int size_processes;

word_array* paths;

/* tokenize_to_array
 * -----------------
 * Parse and tokenize a string into an array.
 */
word_array* tokenize_to_array(char* string, char* token, int breakQuotes) {
  // Setup
  int index = 0;
  int result_size = 0;
  char** result = calloc(result_size, sizeof(char*));
  if (result == NULL) {
    fprintf(stderr, "Couldn't allocate room for result.\n");
    exit(-1);
  }
  // Strtok mangles, make a copy.
  char* copy = calloc(strlen(string) + 1, sizeof(char)); // +1 for 0 index.
  if (copy == NULL) {
    fprintf(stderr, "Couldn't allocate room for copy.\n");
    exit(-1);
  }
  strcpy(copy, string);
  // Search for quotes to mask out
  int found = 0;
  for (int i = strlen(copy); i >= 0; i--) {
    if (copy[i] == '"') {
      // Make a note when we find quotes that we're inside.
      found = !found;
    }
    if (copy[i] == ' ' && found) {
      // Mask spaces with a rarely used ascii character instead.
      copy[i] = OBSCURE_CHARACTER;
    }
  }
  // Copy tokens over to the result.
  char* item = strtok (copy, token);
  while (item) {
    if (index + 1 > result_size) {
      result_size += 1;
      result = realloc(result, result_size * sizeof(char*));
      if (result == NULL) {
        fprintf(stderr, "Couldn't allocate room for result.\n");
        exit(-1);
      }
    }
    result[index] = calloc(strlen(item) + 1, sizeof(char));
    if (result[index] == NULL) {
      fprintf(stderr, "Couldn't allocate room for result[index].\n");
      exit(-1);
    }
    strcpy(result[index], item);
    index += 1;
    item = strtok (NULL, token);
  }
  // Search for quotes to unmask back in
  for (int i = result_size -1; i >= 0; i--) {
    int unfound = 0;
    for (int j = strlen(result[i]); j >= 0; j--) {
      if (result[i][j] == '"') {
        // Make a note if we find quotes that we're inside.
        unfound = !unfound;
      }
      if (result[i][j] == OBSCURE_CHARACTER && unfound) {
        // Unmask the rarely used character and return it to a space.
        result[i][j] = ' ';
      }
    }
  }
  // Clean up.
  free(copy);
  // Prep the struct for return.
  word_array* the_struct = calloc(1, sizeof(struct word_array));
  the_struct->size = result_size - 1;
  the_struct->items = result;
  return the_struct;
}

/* find_pipes
 * ----------
 * Looks for pipes in the command.
 */
int find_pipes(word_array* tokens) {
  int pipe = -1;
  for (int i = 0; i < tokens->size; i++) {
    if (strncmp(tokens->items[i], PIPE_OPERATOR, 4) == 0) {
      pipe = i;
      fprintf(stderr, "Found a pipe at %d\n", i);
      break;
    }
  }
  return pipe;
}

/* find_seq
 * --------
 * Looks for sequences in the command.
 */
int find_seq(word_array* tokens) {
  int seq = -1;
  for (int i = 0; i < tokens->size; i++) {
    if (strncmp(tokens->items[i], SEQ_OPERATOR, 4) == 0) {
      seq = i;
      fprintf(stderr, "Found a seq at %d\n", i);
      break;
    }
  }
  return seq;
}

/* list_processes
 * --------------
 * Looks for pipes in the command.
 */
void list_processes() {
  for (int i = 0; i < size_processes; i++) {
    if (processes[i].pid != -1) {
      fprintf(stdout, "pid: %d, command: '%s'\n", processes[i].pid, processes[i].command);
    }
  }
}

/* kill_process
 * ------------
 * Kill a process by PID.
 */
void kill_process(int pid) {
  int killed_something = 0;
  for (int i = 0; i < size_processes; i++) {
    if (processes[i].pid == pid) {
      killed_something = 1;
      kill(pid, SIGKILL);
      fprintf(stderr, "Killed process with pid %d\n", pid);
      processes[i].pid = -1;
      free(processes[i].command);
    }
  }
  if (!killed_something) {
    fprintf(stdout, "Nothing to kill with pid %d\n", pid);
  }
}

// Implicit Declarations
void eval_pipes(word_array* tokens, int pipe_loc);
void eval_seq(word_array* tokens, int seq_loc);

/* evaluate_command
 * ----------------
 * Evaluates a command
 */
int evaluate_input(word_array* tokens) {
  fprintf(stderr, "Evalutating an input %s\n", tokens->items[0]);
  //
  // Is there a pipe?
  int pipe_loc = find_pipes(tokens);
  int seq_loc = find_seq(tokens);
  if (pipe_loc != -1) {
    // We have a pipe.
    eval_pipes(tokens, pipe_loc);  
  } else if (seq_loc != -1){
    // We have a Seq.
    eval_seq(tokens, seq_loc);
  } else {
    if (strncmp(tokens->items[0], "cd", 3) == 0) {
      // Detect cd
      // Make sure the directory exists.
      struct stat s;
      int err = stat(tokens->items[1], &s);
      if (err == -1) {
        fprintf(stdout, "That directory does not exist.\n");
      } else {
        if(S_ISDIR(s.st_mode)) {
          // It's a directory.
          fprintf(stdout, "Changing directory.\n");
          chdir(tokens->items[1]);
        } else {
          // It's not a directory.
          fprintf(stdout, "That is not a directory.\n");
        }
      }
    } else if (strncmp(tokens->items[0], CMDALL, 6) == 0) {
      // Detect `cmdall`
      list_processes();
    } else if (strncmp(tokens->items[0], CMDKILL, 6) == 0) {
      // Detect `cmdkill`
      int pid = atoi(tokens->items[1]);
      kill_process(pid);
    } else {
      // Just a normal command.
      int should_wait = 1; // We flag that we should wait by default.
      if (strncmp(tokens->items[0], TOBACK, 6) == 0) {
        // Detect `toback`
        // Most of the work is done in the parent process, later.
        should_wait = 0;
      } 
      // Process command.
      short process;
      if ((process = fork()) == 0) {
        // Child process, runs the command.
        // Add the null at the end.
        tokens->size += 1;
        tokens->items = realloc(tokens->items, (tokens->size+1) * sizeof(char*)); // For 0 index.
        if (tokens->items == NULL) {
          fprintf(stderr, "Couldn't reallocate tokens->items\n");
          exit(-1);
        }
        tokens->items[tokens->size] = 0;
        // Decide if we should wait or not (based on toback)
        if (!should_wait) {
          tokens->items++;
          tokens->size--;
        }
        // Build the command path.
        // Do this by repeatedly testing all of the valid PATH variable entries.
        char* command_buffer;
        for (int index = paths->size; index > 0; index--) {
          // Need to parse the first command and test for paths.
          command_buffer = calloc(strlen(paths->items[index]) + strlen(tokens->items[0]) + 2, sizeof(char)); // Zero index and null
          if (command_buffer == NULL) {
            fprintf(stderr, "Couldn't allocate a command buffer.\n");
            exit(-1);
          }
          // Get the full path.
          strcat(command_buffer, paths->items[index]);
          strcat(command_buffer, "/");
          strcat(command_buffer, tokens->items[0]);
          // Run if (if it doesn't work, that's ok, loop and try the next!)
          execv(command_buffer, tokens->items);
          free(command_buffer);
        }
        // There is no command.
        fprintf(stdout,"404: Command not found.\n");
        exit(-1);
      } else  {
        // Back in the parent process.
        int returnCode;
        // If it's a toback call, we bypass this and do the else.
        if (should_wait) {
          while (process != wait(&returnCode)) { };
        } else {
          // Need to add it to our process list.
          // Is there room?
          int had_room = 0;
          for (int i=0; i < size_processes; i++) {
            // Is the entry populated already?
            if (processes[i].pid == -1) {
              // It isn't! Add the process here.
              had_room = 1; // Mark that we had room here.
              processes[i].pid = process;
              processes[i].command = calloc(MAX_COMMAND_LENGTH, sizeof(char));
              for (int j=0; j <= tokens->size; j++) {
                strcat(processes[i].command, tokens->items[j]);
                if (j != tokens->size) {
                  strcat(processes[i].command, " ");
                }
              }
              processes[i].command = realloc(processes[i].command, (strlen(processes[i].command) + 1) * sizeof(char));
              break;
            }
          }
          // If we didn't find a empty spot, create some room.
          if (!had_room) {
            // Allocate a new spot
            size_processes++;
            processes = realloc(processes, size_processes * sizeof(struct process));
            // Set the PID
            processes[size_processes-1].pid = process;
            // Set the command
            processes[size_processes-1].command = calloc(MAX_COMMAND_LENGTH, sizeof(char));
            for (int j=0; j <= tokens->size; j++) {
              strcat(processes[size_processes-1].command, tokens->items[j]);
              if (j != tokens->size) {
                strcat(processes[size_processes-1].command, " ");
              }
            }
            // Resize the memory to fit.
            processes[size_processes-1].command = realloc(processes[size_processes-1].command, strlen(processes[size_processes-1].command) * sizeof(char));
          }
        }
      }
    }
  }
  return 0;
}

void eval_pipes(word_array* tokens, int pipe_loc) {
  fprintf(stderr, "Evaluating a pipe %s\n", tokens->items[0]);
  // Break it into multiple inputs. Each entry is one side of the pipe.
  word_array** sides = calloc(2, sizeof(word_array*));
  sides[0] = calloc(1, sizeof(struct word_array));
  if (sides[0] == NULL) {
    fprintf(stderr, "Couldn't allocate sides[0]\n");
    exit(-1);
  }
  sides[1] = calloc(1, sizeof(struct word_array));
  if (sides[0] == NULL) {
    fprintf(stderr, "Couldn't allocate sides[1]\n");
    exit(-1);
  }
  // Set the sizes as necessary.
  sides[0]->size = pipe_loc - 1;
  sides[1]->size = tokens->size - pipe_loc - 1;
  // Populate the first side.
  sides[0]->items = calloc(sides[0]->size + 1, sizeof(char*));
  if (sides[0]->items == NULL) {
    fprintf(stderr, "Couldn't allocate sides[0]->items\n");
    exit(-1);
  }
  for (int i=0; i <= sides[0]->size; i++) {
    sides[0]->items[i] = tokens->items[i];
  }
  // Populate the second side.
  sides[1]->items = calloc(sides[1]->size + 1, sizeof(char*));
  if (sides[1]->items == NULL) {
    fprintf(stderr, "Couldn't allocate sides[1]->items\n");
    exit(-1);
  }
  for (int i=0; i <= sides[1]->size; i++) {
    sides[1]->items[i] = tokens->items[i + pipe_loc + 1];
  }
  // Plumb the pipes
  int the_pipe[2];
  pipe(the_pipe);
  // Make a copy of our current pipes.
  int the_stdout = dup(1);
  int the_stdin = dup(0);
  // Set up pipes and run.
  dup2(the_pipe[1], 1);       // The pipe's write channel gets set as STDOUT.
  evaluate_input(sides[0]);   // Evaluate the first command, stash the STDOUT into the pipe
  close(the_pipe[1]);         // Close the pipe, signalling EOF
  dup2(the_stdout, 1);        // Replace STDOUT with what it should be.
  // -----------------------------------------------------------------
  dup2(the_pipe[0], 0);       // Set the pipe's read channel to the STDIN
  evaluate_input(sides[1]);   // Evaluate the second command, pulling STDIN from the pipe.
  close(the_pipe[0]);         // Close the pipe, signalling EOF.
  dup2(the_stdin, 0);         // Reset the pipes to normal.
}

void eval_seq(word_array* tokens, int seq_loc) {
  // Break it into multiple inputs.
  word_array** sides = calloc(2, sizeof(word_array*));
  sides[0] = calloc(1, sizeof(struct word_array));
  if (sides[0] == NULL) {
    fprintf(stderr, "Couldn't allocate sides[0]\n");
    exit(-1);
  }
  sides[1] = calloc(1, sizeof(struct word_array));
  if (sides[1] == NULL) {
    fprintf(stderr, "Couldn't allocate sides[1]\n");
    exit(-1);
  }
  // Setup the sizes as needed.
  sides[0]->size = seq_loc - 1;
  sides[1]->size = tokens->size - seq_loc - 1;
  // Populate first side.
  sides[0]->items = calloc(sides[0]->size + 1, sizeof(char*));
  if (sides[0]->items == NULL) {
    fprintf(stderr, "Couldn't allocate sides[0]->items\n");
    exit(-1);
  }
  for (int i=0; i <= sides[0]->size; i++) {
    sides[0]->items[i] = tokens->items[i];
  }
  // Populate second side.
  sides[1]->items = calloc(sides[1]->size + 1, sizeof(char*));
  if (sides[1]->items == NULL) {
    fprintf(stderr, "Couldn't allocate sides[1]->items\n");
    exit(-1);
  }
  for (int i=0; i <= sides[1]->size; i++) {
    sides[1]->items[i] = tokens->items[i + seq_loc + 1];
  }
  // Evaluate both sequentially.
  evaluate_input(sides[0]);
  evaluate_input(sides[1]);
}

/* check_processes
 * ---------------
 * Prints output if any process in the list has finished (finally!)
 */
void check_processes(void) {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  timeinfo = localtime(&rawtime);

  for (int i=0; i< size_processes; i++) {
    if (processes[i].pid != -1) {
      if (waitpid(processes[i].pid, NULL, WNOHANG) != 0) { // Doesn't block.
        fprintf(stdout, "%s :: Command (pid %d) finished: %s\n", asctime(timeinfo), processes[i].pid, processes[i].command);
        processes[i].pid = -1;
        free(processes[i].command);
      }
    }
  }
}

/* main
 * ----
 * Please see `./a2.pdf` for a description of the problem for this program.
 */
int main(int argc, char *argv[]) {
  char prompt[MAX_PS1_LENGTH] = { 0 };
  // Setup paths and tab completion
  rl_bind_key('\t', rl_complete);
  paths = tokenize_to_array(getenv("PATH"), ":", 0);
  // Setup the process list.
  size_processes = 0;                     // No array yet.
  processes = calloc(size_processes, sizeof(struct process)); // Init at zero.
  // The REPL
  for (;;) {
    check_processes();
    // Print the PS1.
    char* hostname = calloc(MAX_COMMAND_LENGTH, sizeof(char));
    gethostname(hostname, MAX_COMMAND_LENGTH);
    snprintf(prompt, sizeof(prompt), "%s %s > ", hostname, getcwd(NULL, 1024));
    // Read input.
    char* input = calloc(1, sizeof(char*));
    input = readline(prompt);
    if (!input) {
      // No input.
      break;
    } else if (strncmp(input, "", 2) == 0) {
      continue;
    }
    // At it to history.
    add_history(input);
    // Tokenize the input, quote sensitive.
    word_array* tokens = tokenize_to_array(input, " ", 1);
    for (int i=0; i <= tokens->size; i++) {
      fprintf(stderr, "  Tokenized: %s\n", tokens->items[i]);
    }
    // No pipes. Just need to evaluate.
    if (evaluate_input(tokens) == -1) {
      return -1;
    }
    // By now, we're done with the input. Start the process over again.
    free(input);
    free(tokens);
  }
  return 0;
}
