/* A2 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 */

#include <stdlib.h>             // Standard Lib.
#include <stdio.h>              // Standard I/O.
#include <sys/types.h>          // Defines data types used in system calls. 
#include <string.h>             // String Functions.
#include <errno.h>              // Error Numbers
#include <unistd.h>             // Fork
#include <sys/wait.h>           // Wait
#include <readline/readline.h>  // Readline
#include <readline/history.h>   // Readline History

#define MAX_COMMAND_LENGTH 2048
#define MAX_PS1_LENGTH 255
#define OBSCURE_CHARACTER '|'
#define PIPE_OPERATOR "::>"

/* Path variable */
typedef struct word_array {
  int size;
  char** items;
} word_array;

word_array* paths;

/* tokenize_to_array
 * -----------------
 * Parse and tokenize a string into an array.
 */
word_array* tokenize_to_array(char* string, char* token, int breakQuotes) {
  int index = 0;
  int result_size = 1;
  char** result = calloc(index + 1, sizeof(char*));
  if (result == NULL) {
    fprintf(stderr, "Couldn't allocate room for result.\n");
    exit(-1);
  }
  
  // Strtok mangles, make a copy.
  char* copy = calloc(strlen(string), sizeof(char));
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
    result[index] = calloc(strlen(item), sizeof(char));
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
    
  free(copy);
  word_array* the_struct = calloc(1, sizeof(word_array));
  the_struct->size = result_size - 1;
  the_struct->items = result;
  return the_struct;
}

int find_pipes(word_array* tokens) {
  for (int i = 0; i <= tokens->size; i++) {
    if (strncmp(tokens->items[i], PIPE_OPERATOR, 4) == 0) {
      return i;
    }
  }
  return -1;
}

/* evaluate_command
 * -----------
 * Evaluates a command
 */
int evaluate_input(word_array* tokens, FILE* stdin, FILE* stdout) {
  // Process command.
  short process;
  if ((process = fork()) == 0) {
    // Child process, runs the command.
    
    // Add the null at the end.
    tokens->size += 1;
    tokens->items = realloc(tokens->items, tokens->size * sizeof(char*));
    tokens->items[tokens->size] = 0;
    
    char* command_buffer;
    for (int index = paths->size; index > 0; index--) {
      // Need to parse the first command and test for paths.
      command_buffer = calloc(sizeof(paths->items[index]) + sizeof(tokens->items[0]) + 1, sizeof(char));
      if (command_buffer == NULL) {
        fprintf(stderr, "Couldn't allocate a command buffer.\n");
        exit(-1);
      }
      // Get the full path.
      strcat(command_buffer, paths->items[index]);
      strcat(command_buffer, "/");
      strcat(command_buffer, tokens->items[0]);  
        
      // Run
      execv(command_buffer, tokens->items);
      free(command_buffer);
    }
    // There is no command.
    fprintf(stdout,"404: Command not found.\n");
    exit(-1);
  } else  {
    // Back in the parent process.
    int returnCode;
    while (process != wait(&returnCode)) { };
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
  paths = tokenize_to_array(getenv("PATH"), ":", 0);
  
  // The REPL
  for (;;) {
    // Print the PS1.
    snprintf(prompt, sizeof(prompt), "%s %s > ", getenv("USER"), getcwd(NULL, 1024));
    // Read input.
    char* input = calloc(1, sizeof(char*));

    input = readline(prompt);
    if (!input) {
      // No input.
      break;
    }
    // At it to history.
    add_history(input);
    
    // Tokenize the input, quote sensitive.
    word_array* tokens = tokenize_to_array(input, " ", 1); 
    
    // Is there a pipe?
    int pipe_loc = find_pipes(tokens);
    if (pipe_loc != -1) {
      // We have a pipe.
      
      // Break it into multiple inputs.
      word_array** sides = calloc(2, sizeof(word_array*));
      sides[0] = calloc(1, sizeof(word_array));
      sides[1] = calloc(1, sizeof(word_array));

      sides[0]->size = pipe_loc;
      sides[1]->size = tokens->size - pipe_loc;
      
      sides[0]->items = calloc(sides[0]->size, sizeof(char*));
      for (int i=0; i < sides[0]->size; i++) {
        sides[0]->items[i] = tokens->items[i];
      }
      
      sides[1]->items = calloc(sides[1]->size + 1, sizeof(char*));
      for (int i=0; i < sides[1]->size; i++) {
        sides[1]->items[i] = tokens->items[i + pipe_loc + 1];
      }
      
      evaluate_input(sides[0], stdin, stdout);
      evaluate_input(sides[1], stdin, stdout);
      
    } else {
      // No pipes. Just need to evaluate.
      if (evaluate_input(tokens, stdin, stdout) == -1) {
        return -1;
      }
    }
    
    // By now, we're done with the input. Start the process over again.
    free(input);
    free(tokens);
  }
  return 0;
};
