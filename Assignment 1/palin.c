/* A1 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 * USAGE: make run
 * OR: make && cat words | palin
 */

#include <sys/types.h>    /* Defines data types used in system calls. */
#include <stdio.h>        /* Standard IO. */
#include <string.h>       /* String functions */
#include <stdlib.h>       /* Builtin functions */
#include <unistd.h>       /* Keypress testing. */
#include <pthread.h>      /* Threads! =D! */
#include <errno.h>

/* Alphabet Enum
 * --------------
 * Used for links[] lookups?
 */
enum alphabet {
  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z
};

/* Trie Node
 * --------------
 * A Trie node.
 */
struct trie_node {
  /* Is this node the end of a word? */
  int word;
  /* The value of the node. Not strictly necessary. */
  char value;
  /* The links to child nodes. */  
  struct trie_node* links[26];
};

/* Add to a Trie
 * -------------
 * Recursively steps through a string and adds it to the trie.
 * Params:
 *   - struct trie_node trie:
 *        The Trie we're adding to. This need not be the root.
 *   - char* item:
 *        What **remains** of the item we're adding.
 */
int trie_add(struct trie_node trie, char* item) {
  /* Detect if item == "" */
    /* If yes, we're done. */
      /* Set the value to be a word and return up the stack. */
    /* Else, there's still a ways to go. */
      /* Find the next value, this is item[0]. */
      /* See if a trie node at trie[value] exists. */
        /* If yes, call add again on that node with item[1..] */
        /* Else, Create a new trie_node, and call add on that! */
  return 0;
}

/* Find in a Trie
 * -------------
 * Recursively steps through a string and finds it in the trie.
 * Params:
 *   - struct trie_node trie:
 *        The Trie we're finding in. This need not be the root.
 *   - char* item:
 *        What **remains** of the item we're finding..
 */
int trie_find(struct trie_node trie, char* item) {
  /* Detect if item == "" */
    /* If yes, we're done. */
      /* See if value is a word and return up the stack. */
    /* Else, there's still a ways to go. */
      /* Find the next value, this is item[0]. */
      /* See if a trie node at trie[value] exists. */
        /* If yes, call find again on that node with item[1..] */
        /* Else, the word doesn't exist, return false up the stack. */
  return 0;
}

/* Parse Input
 * -----------
 * Parses the `stdin` for a list of words separated by `\n`
 * Grows the `words` global as exponentially then resizes it back down afterwards
 * to avoid a ton of reallocs.
 */
void parse_input(int* dict_size, char*** words) {
  

}

/* Main
 * ----
 * Please see `./a1.pdf` for a description of the problem for this program.
 */
int main(int argc, char *argv[]) {
  /* Variable Init */
  int dict_size = 1;
  int stdin_position = 0;
  char** words = malloc(dict_size * sizeof(char*));
  if (words == NULL) {
    fprintf(stderr, "Error allocating memory for words.\n");
    exit(-1);
  }

  /* Take in dictionary from STDIN */
  /* Threading Potential: 0/10 */
  for (;;) {
    fprintf(stderr, "STDIN Position: %d\n", stdin_position);
    /* Do we need to enlarge the words array? */
    if (stdin_position >= dict_size - 1) {
      dict_size *= 2;
      words = realloc (words, dict_size * sizeof(char*));
      if (words == NULL) {
        fprintf(stderr, "Error reallocating memory for words.\n");
        exit(-1);
      }
    }
    
    /* Add the item */
    words[stdin_position] = malloc(256 * sizeof(char*));
    if (words[stdin_position] == NULL) {
      fprintf(stderr, "Error allocating memory for fgets.\n");
    }

    char* status = fgets(words[stdin_position], 256, stdin);
    if (status == NULL) {
      /* Done STDIN */
      break;
    } else {
      /* Keep going */
      words[stdin_position][strlen(words[stdin_position])-1] = 0;   /* Strip the \n */
    }
    /* Move! */
    stdin_position++;
  }
  /* Resize words to fit. */
  dict_size = stdin_position;
  words = realloc(words, stdin_position * sizeof(char*));
  if (words == NULL) {
    fprintf(stderr, "Error reallocating memory for words after the loop.\n");
    exit(-1);
  }



  /* Process Dictionary into a Trie */
    /* DO NOT: Mutate the array as we work. */

  /* Find the reverse of each item in the array in the dictionary */
    /* If it exists, place it in an output array (We'll need to do some light sort). */
    /* Else, nothing. */

  /* Output the array as a list, broken by \n */
  int stdout_position;
  fprintf(stderr, "dict_size: %d\n", dict_size);
  for (stdout_position = 0; stdout_position < dict_size; stdout_position++) { 
    fprintf(stderr, "STDOUT Position: %d\n", stdout_position);
    fprintf(stdout, "STDOUT Word: %s\n", words[stdout_position]); /* DEBUGING */
  }

  /* Done! */
  exit(0);
  return 0;
}
