/* A1 CSC 360 UVIC 2013 FALL
 * Andrew Hobden (V00788452)
 * -------------------------
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
#define THREADS_TO_USE  4


/* Dictionary
 * ----------
 * A simple struct to hold the stdin dictionary and it's length.
 */
typedef struct stdin_dictionary {
  int size;
  char** words;
} stdin_dictionary;


/* Trie Node
 * ---------
 * A Trie node.
 */
typedef struct trie_node {
  /* Is this node the end of a word? */
  int words;
  /* The links to child nodes. */  
  struct trie_node *links[27];
  pthread_mutex_t lock;
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

/* Job
 * ---
 * A job packet for threads. This is passed to `add_worker` and `find_worker`.
 * Avoid mutating the `dictionary` or `root` yourself, let the trie calls do that.
 */
typedef struct job {
  int start;                    /* The location to start in the inputted array. */
  int stop;                     /* The location to stop in the inputted array. */
  trie_node** root;             /* The root of our trie. */
  stdin_dictionary* input;     /* The input dictionary. */
  char** output;                /* The output array. (Why not a dict? We don't need a size.) */
} job;

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
    pthread_mutex_lock(&request->node->lock);
    request->node->words += 1;
    pthread_mutex_unlock(&request->node->lock);
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
      pthread_mutex_lock(&request->node->lock);
      /* Else, Create a new trie_node, and call add on that! */
      request->node->links[links_index] = calloc(1, sizeof(trie_node));
      if (request->node->links[links_index] == NULL) {
        fprintf(stderr, "Error allocating a new trie node.\n");
        exit(-1);
      }
      /* Set up vals */
      request->node->links[links_index]->words = 0;
      pthread_mutex_unlock(&request->node->lock);
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

/* Trie Adder Worker
 * ------------------
 * Basically our "Bob the Builder" this worker accepts a job struct.
 * Params: (in a job struct)
 *   - int start:
 *        The location to start in the inputted array.
 *   - int stop:
 *        The location to stop in the inputted array.
 *   - trie_node** root:
 *        The root of our trie.
 *        You shouldn't mutate this, but the trie_add function call **should**.
 *   - stdin_dictionary** input:
 *        The input dictionary.
 *   - char** output:
 *        The output array. (Why not a dict? We don't need a size.)
 */
void *add_worker(void *job_req) {
  job *job = job_req;
  /* Loop */
  int pos;
  for (pos = job->start; pos <= job->stop; pos++) {
    /* Setup */
    trie_request *request = calloc(1, sizeof(trie_request));
    if (request == NULL) {
      fprintf(stderr, "Error allocating thread request.\n");
      exit(-1);
    }
    request->node = *job->root;
    request->item = job->input->words[pos];
    request->position = 0;
    trie_add(request);
    free(request);
  }
  pthread_exit(NULL);
  return((void *) 0);
}

/* Trie Finder Worker
 * ------------------
 * Basically our "Sherlock" this worker accepts a job struct.
 * Params: (in a job struct)
 *   - int start:
 *        The location to start in the inputted array.
 *   - int stop:
 *        The location to stop in the inputted array.
 *   - trie_node** root:
 *        The root of our trie.
 *        You shouldn't mutate this, at all! trie_find won't.
 *   - stdin_dictionary** input:
 *        The input dictionary.
 *   - char** output:
 *        The output array. (Why not a dict? We don't need a size.)
 */
void *find_worker(void *job_req) {
  job *job = job_req;
  /* Loop */
  int pos;
  for (pos = job->start; pos <= job->stop; pos++) {
    /* Setup */
    trie_request *request = calloc(1, sizeof(trie_request));
    if (request == NULL) {
      fprintf(stderr, "Error allocating thread request.\n");
      exit(-1);
    }
    request->node = *job->root;
    request->item = reverse(job->input->words[pos]);
    request->position = 0;
    int success = trie_find(request);
    if (success == 1) {
      job->output[pos] = job->input->words[pos];
    }
    free(request);
  }
  pthread_exit(NULL);
  return((void *) 0);

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
  
  /* How big should thread chunks be?
   * Take note that this may not guarantee even division. That's OK,
   * The last thread will pick up the straggling remainer. 
   * TODO: More elegant?
   */ 
  int job_chunk = input->size / THREADS_TO_USE;
  job **jobs = calloc(THREADS_TO_USE, sizeof(job*));

  pthread_t *add_threads = calloc(THREADS_TO_USE, sizeof(pthread_t));
  /* Process Dictionary into a Trie */
  int thread_num;
  for (thread_num = 0; thread_num < THREADS_TO_USE; thread_num++) {
    /* Divvy up the jobs appropriately. This is a purposely simple divvy. */
    jobs[thread_num] = calloc(1, sizeof(job));
    jobs[thread_num]->start = thread_num * job_chunk;
    /* On the last thread? */
    if (thread_num == THREADS_TO_USE - 1) {
      jobs[thread_num]->stop = input->size-1;
    }
    else {
      jobs[thread_num]->stop = (thread_num + 1) * job_chunk;
    }
    jobs[thread_num]->input = input;
    jobs[thread_num]->output = NULL; /* We don't write to this. */
    jobs[thread_num]->root = &root;
    pthread_create(&add_threads[thread_num], 0, add_worker, jobs[thread_num]);
  }

  /* Threads join here */
  int add_join_num;
  for (add_join_num = 0; add_join_num < THREADS_TO_USE; add_join_num++) {
    pthread_join(add_threads[add_join_num], NULL);
  }
  
  /* Output */
  char** output = calloc(input->size, sizeof(char*));
  pthread_t *find_threads = calloc(THREADS_TO_USE, sizeof(pthread_t));
  /* Process Dictionary into a Trie */
  int find_thread_num;
  for (find_thread_num = 0; find_thread_num < THREADS_TO_USE; find_thread_num++) {
    /* Divvy up the jobs appropriately. This is a purposely simple divvy. */
    jobs[find_thread_num] = calloc(1, sizeof(job));
    jobs[find_thread_num]->start = find_thread_num * job_chunk;
    /* On the last thread? */
    if (find_thread_num == THREADS_TO_USE - 1) {
      jobs[find_thread_num]->stop = input->size-1;
    }
    else {
      jobs[find_thread_num]->stop = (find_thread_num + 1) * job_chunk;
    }
    jobs[find_thread_num]->input = input;
    jobs[find_thread_num]->output = output;
    jobs[find_thread_num]->root = &root;
    pthread_create(&find_threads[find_thread_num], 0, find_worker, jobs[find_thread_num]);
  }
  int find_join_num;
  for (find_join_num = 0; find_join_num < THREADS_TO_USE; find_join_num++) {
    pthread_join(find_threads[find_join_num], NULL);
  }

  fprintf(stderr, "Getting ready to output!\n");
  /* Output the array as a list, broken by \n */
  int stdout_position;
  for (stdout_position = 0; stdout_position < input->size; stdout_position++) { 
    if (output[stdout_position] != NULL) {
      fprintf(stdout, "%s\n", output[stdout_position]);     
    }
  }

  /* Done! */
  exit(0);
  return 0;
}
