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


/* Main
 * ----
 * Please see `./a1.pdf` for a description of the problem for this program.
 */
 int main(int argc, char *argv[]) {
  /* Variable Init */
  int dict_size = 1;
	int dict_position = 0;
  char** words = malloc(dict_size * sizeof(char*)); /* Start with 10 words in size. Grow later */
  size_t word_size;

	for (;;) {
		/* Do we need to enlarge the words array? */
		if (dict_position >= dict_size - 1) {
			
		}
		
		/* Add the item */
	  words[dict_position] = malloc(0); /* Workaround for getline */
		int status = getline(&words[dict_position], &word_size, stdin);
	  if (status == -1) {
			/* Done STDIN */
			break;
		} else {
			/* Keep going */
			words[dict_position][strlen(words[0])-1] = 0; 	/* C is annoying! */
			fprintf(stdout, "'%s'\n", words[dict_position]);
		}
	}

  /* Take in dictionary from STDIN */
  /* IDEA: Use this as a producer? */

  /* Process Dictionary into a Trie */
    /* DO NOT: Mutate the array as we work. */
    /* IDEA: Use this as a consumer? */

  /* Find the reverse of each item in the array in the dictionary */
    /* If it exists, place it in an output array (We'll need to do some light sort). */
    /* Else, nothing. */
  /* Output the array as a list, broken by \n */


  /* Done! */
  exit(0);
  return 0;
}
