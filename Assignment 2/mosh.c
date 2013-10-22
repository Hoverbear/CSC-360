/* A2 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 */

#include <stdlib.h>        // Standard Lib.
#include <stdio.h>         // Standard I/O.
#include <sys/types.h>     // Defines data types used in system calls. 
#include <string.h>        // String Functions.
#include <errno.h>         // Error Numbers

#define CONFIG_FILE "~/.moshrc"
#define DEFAULT_CONFIG_FILE "./example_moshrc"

/* read_config
 * -----------
 * Reads from the specified configuration file. This sets various options used in the shell. Options are listed alongside relevant code.
 * Parameters:
 *   * `char* path`: The path to the configuration file.
 */
int read_config(char* path) {
  fprintf(stderr, "read_config called.\n");
  return -1;
};

/* event_loop
 * ----------
 * The main worker loop of the program.
 */
int event_loop() {
  fprintf(stderr, "event_loop called.\n");
  return -1;
};

/* main
 * ----
 * Please see `./a2.pdf` for a description of the problem for this program.
 */
int main(int argc, char *argv[]) {
  // Read from `CONFIG_FILE`, or use defaults.
  if (read_config(CONFIG_FILE) == -1) {
    read_config(DEFAULT_CONFIG_FILE);
  }
  
  // Start main event loop.
  if (event_loop()) {
    return 0; 
  }
  return -1;
};