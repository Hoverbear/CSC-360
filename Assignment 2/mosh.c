/* A2 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 */

#include <stdlib.h>        // Standard Lib.
#include <stdio.h>         // Standard I/O.
#include <sys/types.h>     // Defines data types used in system calls. 
#include <string.h>        // String Functions.
#include <errno.h>         // Error Numbers
#include <unistd.h>        // Fork
#include <sys/wait.h>      // Wait

#define MAX_COMMAND_LENGTH 2048
#define PS1 "mosh $> "

/* print_ps1
 * -----------
 * Simply prints the PS1.
 */
void print_ps1() {
  fprintf(stdout, PS1);
  return;
}

/* evaluate_command
 * -----------
 * Evaluates a command
 * Parameters:
 *   * `char* path`: The path to the configuration file.
 */
int evaluate_command(char* command) {
  fprintf(stderr, "evaluate_command called with: %s\n", command);
  
  // Process command.
  short process;
  if ((process = fork()) == 0) {
    // Child process, runs the command.
    fprintf(stderr, "  (In fork) command is: %s\n", command);
    execl(command, 0);
    exit(-1);
  } else  {
    int returnCode;
    while (process != wait(&returnCode)) { };
    fprintf(stderr, "Process returned with %d\n", returnCode);
  }
  
  return 0;
}

/* main
 * ----
 * Please see `./a2.pdf` for a description of the problem for this program.
 */
int main(int argc, char *argv[]) {
  
  // The REPL
  for (;;) {
    // Print the PS1.
    print_ps1();
    
    // Wait for a command.
    char* command;
    command = calloc(MAX_COMMAND_LENGTH, sizeof(char));
    if (command == NULL) {
      return -1;
    }
    if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
      return -1;
    }
    command[strlen(command)-1] = '\0';
    command = realloc(command, strlen(command) * sizeof(char) +1);
    if (command == NULL) {
      return -1;
    }
    
    // Evalute the command.
    if (evaluate_command(command) == -1) {
      return -1;
    }
    
    // By now, we're done with the command. Start the process over again.
  }
  return 0;
};