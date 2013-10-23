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
#include <readline/readline.h>  // Readline
#include <readline/history.h>   // Readline History

#define MAX_COMMAND_LENGTH 2048
#define MAX_PS1_LENGTH 255

/* Path variable */
char** paths;

/* tokenize_to_array
 * -----------------
 * Parse and tokenize a string into an array.
 */
char** tokenize_to_array(char* string, char token) {
  char** result = NULL;
  // Strtok mangles, make a copy.
  char* copy = calloc(strlen(string), sizeof(char*));
  fprintf(stderr, "%s\n", string);
  strcpy(copy, string);
  
  fprintf(stderr, "%s\n", copy);
  
  char* item = strtok (copy, ":");
  fprintf(stderr, "%s\n", item);
  
  return result;
}

/* evaluate_command
 * -----------
 * Evaluates a command
 * Parameters:
 *   * `char* path`: The path to the configuration file.
 */
int evaluate_input(char* input) {
  fprintf(stderr, "evaluate_command called with: %s\n", input);
  
  // Process command.
  short process;
  if ((process = fork()) == 0) {
    // Child process, runs the command.
    fprintf(stderr, "  (In fork) command is: %s\n", input);
    // TODO: Split args
    // TODO: Handle Path
    execl(input, "", NULL);
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
  
  char prompt[MAX_PS1_LENGTH] = { 0 };
  
  rl_bind_key('\t', rl_complete);
  
  paths = tokenize_to_array(getenv("PATH"), ':');
  
  // The REPL
  for (;;) {
    // Print the PS1.
    snprintf(prompt, sizeof(prompt), "%s :: %s :: > ", getenv("USER"), getcwd(NULL, 1024));
    
    // Read input.
    char* input;
    input = readline(prompt);
    if (!input) {
      // No input.
      break;
    }
    // At it to history.
    add_history(input);
    
    // Evalute the command.
    if (evaluate_input(input) == -1) {
      return -1;
    }
    
    // By now, we're done with the input. Start the process over again.
    free(input);
  }
  return 0;
};