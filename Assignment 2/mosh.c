/* A2 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 */

#include <stdlib.h>        // Standard Lib.
#include <stdio.h>         // Standard I/O.
#include <sys/types.h>     // Defines data types used in system calls. 
#include <string.h>        // String Functions.
#include <errno.h>         // Error Numbers

#define MAX_COMMAND_LENGTH 2048

/* print_ps1
 * -----------
 * Simply prints the PS1.
 */
void print_ps1() {
  fprintf(stdout, "mosh $> ");
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