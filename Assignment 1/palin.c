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

#define MAX_WORD_LENGTH 256
#define ASCII_STARTS    97


/* Dictionary
 * ----------
 * A simple struct to hold the stdin dictionary and it's length.
 */
typedef struct stdin_dictionary {
  int size;
  char** words;
} stdin_dictionary;

/* Trie Node
 * --------------
 * A Trie node.
 */
typedef struct trie_node {
  /* Is this node the end of a word? */
  int words;
  /* The links to child nodes. */  
  struct trie_node *links[27];
} trie_node;

/* Trie request packet
 * -------------------
 * Since we will be using threads heavily, and threads only allow one arg,
 * we'll define this struct to handle passing params around.
 */
typedef struct trie_request {
  struct trie_node* node;
  char* item;
  int position;
} trie_request;

/* Add to a Trie
 * -------------
 * Recursively steps through a string and adds it to the trie.
 * Params: (in a trie_request)
 *   - struct trie_node trie:
 *        The Trie we're adding to. This need not be the root.
 *   - char* item:
 *        What **remains** of the item we're adding.
 *   - int position:
 *        What position we are in the word.
 */
int trie_add(trie_request* request) {
  /* Detect if item == "" */
  if (request->position == strlen(request->item)) {
    /* If yes, we're done. */
    /* Set the value to be a word and return up the stack. */
    request->node->words += 1;
    return 1;
  } else {
    /* Else, there's still a ways to go. */
    /* Find the next value, this is item[0]. */
    int links_index = request->item[request->position] - 97;
		if ((links_index < 0) || (links_index > 97)) {
			/* The value is something special, like a hypen */
			links_index = 26;
		}
    /* See if a trie node at trie[value] exists. */
    if (request->node->links[links_index] != NULL) {
      /* If yes, call add again on that node with item[1..] */
      request->node = request->node->links[links_index];
      request->position += 1;
      return trie_add(request);
    } else {
      /* Else, Create a new trie_node, and call add on that! */
      request->node->links[links_index] = calloc(1, sizeof(trie_node));
      if (request->node->links[links_index] == NULL) {
        fprintf(stderr, "Error allocating a new trie node.\n");
        exit(-1);
      }
      /* Set up vals */
      request->node->links[links_index]->words = 0;
      /* Move */
      request->node = request->node->links[links_index];
      request->position += 1;
      return trie_add(request);
    }
  }
  return 0;
}

/* Find in a Trie
 * -------------
 * Recursively steps through a string and finds it in the trie.
 * Params: (in a trie_request)
 *   - struct trie_node trie:
 *        The Trie we're finding in. This need not be the root.
 *   - char* item:
 *        What we're finding.
 *   - int position:
 *       The position of the word we're in.
 */
int trie_find(trie_request* request) {
  /* Detect if item == "" */
  if (request->position == strlen(request->item)) {
    /* If yes, we're done. */
    if (request->node->words >= 1) {
      /* See if value is a word and return up the stack. */
			return 1;
    } else {
      return 0;
    }
  } else {
    /* Else, there's still a ways to go. */
    /* Find the next value, this is item[0]. */
    int links_index = request->item[request->position] - 97;
		if ((links_index < 0) || (links_index > 97)) {
			/* The value is something special, like a hypen */
			links_index = 26;
		}
    /* See if a trie node at trie[value] exists. */
    if (request->node->links[links_index] != NULL) {
      /* If yes, call find again on that node with item[1..] */
      request->position += 1;
      request->node = request->node->links[links_index];
      return trie_find(request);
    } else {
      /* Else, the word doesn't exist, return false up the stack. */
      return 0;
    }
  }
  return 0;
}

/* Parse Input
 * -----------
 * Parses the `stdin` for a list of words separated by `\n`
 * Grows the `words` global as exponentially then resizes it back down afterwards
 * to avoid a ton of reallocs.
 */
stdin_dictionary* parse_input() {
  stdin_dictionary *dict = calloc(1, sizeof(stdin_dictionary));
  if (dict == NULL) {
    fprintf(stderr, "Error allocating memory for words.\n");
    exit(-1);
  }
  dict->size = 1;
  dict->words = malloc(dict->size * sizeof(char*));
  int stdin_position = 0;
  if (dict->words == NULL) {
    fprintf(stderr, "Error allocating memory for words.\n");
    exit(-1);
  }

  /* Take in dictionary from STDIN */
  /* Threading Potential: 0/10 */
  for (;;) {
    /* Do we need to enlarge the words array? */
    if (stdin_position >= dict->size - 1) {
      dict->size *= 2;
      dict->words = realloc (dict->words, dict->size * sizeof(char*));
      if (dict->words == NULL) {
        fprintf(stderr, "Error reallocating memory for words.\n");
        exit(-1);
      }
    }
    
    /* Add the item */
    dict->words[stdin_position] = calloc(256, sizeof(char*));
    if (dict->words[stdin_position] == NULL) {
      fprintf(stderr, "Error allocating memory for fgets.\n");
    }

    void* status = fgets(dict->words[stdin_position], 256, stdin);
    if (status == NULL) {
      /* Done STDIN */
      break;
    } else {
      /* Keep going */
      if (dict->words[stdin_position][strlen(dict->words[stdin_position])-1] == '\n') {
        dict->words[stdin_position][strlen(dict->words[stdin_position])-1] = 0;   /* Strip the \n */
      }
    }
    /* Move! */
    stdin_position++;
  }
  /* Resize words to fit. */
  dict->size = stdin_position;
  dict->words = realloc(dict->words, stdin_position * sizeof(char*));
  if (dict->words == NULL) {
    fprintf(stderr, "Error reallocating memory for words after the loop.\n");
    exit(-1);
  }
  return dict;
}

/* Reverse a string
 * ----------------
 * Just reverses a string and returns a copy it.
 */
char* reverse(char* item) {
  int size = strlen(item) - 1;
  char* reverse = calloc(size, sizeof(char*));
  if (reverse == NULL) {
    fprintf(stderr, "Couldn't allocate reverse memory.\n");
    exit(-1);
  }
  int step;
  for (step = 0; step <= size; step++) {
    reverse[step] = item[size - step];
  }
  return reverse;
}

/* Main
 * ----
 * Please see `./a1.pdf` for a description of the problem for this program.
 */
int main(int argc, char *argv[]) {
  /* Variable Init */
  stdin_dictionary *input = parse_input();
  trie_node *root = calloc(1, sizeof(trie_node));
  if (root == NULL) {
    fprintf(stderr, "Error allocating root trie node.\n");
    exit(-1);
  }

  /* Process Dictionary into a Trie */
  int trie_processor;
  for (trie_processor = 0; trie_processor < input->size; trie_processor++) {
    /* DO NOT: Mutate the array as we work. */
    trie_request *request = calloc(1, sizeof(trie_request));
    request->node = root;
		request->item = input->words[trie_processor];
    request->position = 0;
    /* Later, this will be a pthread call. */
    trie_add(request);
    free(request);
  }
  int output_size = input->size;
  char** output = calloc(output_size, sizeof(char*));
  if (output == NULL) {
    fprintf(stderr, "Error allocating the output array.\n");
    exit(-1);
  }

  /* Find the reverse of each item in the array in the dictionary */
  int trie_finder;
  for (trie_finder = 0; trie_finder < input->size; trie_finder++) {
    trie_request *request = calloc(1, sizeof(trie_request));
    request->node = root;
    request->item = reverse(input->words[trie_finder]);
    request->position = 0;
    /* Later, this will be a pthread call. */
    int status = trie_find(request);
    if (status == 1) {
      /* If it exists, place it in an output array (no sorting needed) */ 
			output[trie_finder] = input->words[trie_finder];
    } else {
      /* Else, nothing. */
      continue;
    }
  }

  /* Output the array as a list, broken by \n */
  int stdout_position;
  for (stdout_position = 0; stdout_position < output_size; stdout_position++) { 
    if (output[stdout_position] != NULL) {
			fprintf(stdout, "%s\n", output[stdout_position]);    	
    }
  }

  /* Done! */
  exit(0);
  return 0;
}
