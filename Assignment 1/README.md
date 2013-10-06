# Assignment 1 #
Author: Andrew Hobden (V00788452)

## Algorithms ##
Since this is essentially a glorified dictionary lookup, I wanted to use a [Trie](http://en.wikipedia.org/wiki/Trie) which is basically just a 26-tree (I ended up making it a 27-tree for special characters like '-'). The algorithms were, accordingly, very recursive.

The general structure of the program is:
  * Read from STDIN into an array of words.
  * Spawn 'n' threads that pick a fair slice of the array and add each item to the trie.
  * Wait for all of them to finish (Not with a barrier, just a loop of joins)
  * Span 'n' threads that pick a fair slice of the array and find the reverse of each item in the trie. If they find one, the copy it from the input array into an output array of the same size.
  * Print out the output array.
  
## Race Conditions ##
There were only two possible race conditions, and of those only one really actually mattered (although both were covered as per assignment spec).

```c
    /* Detect if item == "" */
    if (request->position == strlen(request->item)) {
      /* If yes, we're done. */
      /* Set the value to be a word and return up the stack. */
      sem_wait(&request->node->lock);
      request->node->words += 1;
      sem_post(&request->node->lock);
      return 1;
    } else {
```
The first race condition could happen when we increment the "words" value of a node, however since we only care that this is `!=0` in the `trie_find()` call it doesn't really matter.

```c
    sem_wait(&request->node->lock);
    /* See if a trie node at trie[value] exists. */
    if (request->node->links[links_index] != NULL) {
      /* If yes, call add again on that node with item[1..] */
      sem_post(&request->node->lock);
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
      sem_init(&request->node->links[links_index]->lock,1,1);
      sem_post(&request->node->lock);
```
The second race condition is actually quite important. If two threads wanted to create a `trie_node` child they might both accidently make disjoint subtrees due to a race condition. **However** this clobbering would be unlikely given how we slice arrays, but there might be edge cases.

## Timings ##
Multi Threaded: `cat words | ./threaded_palin`

  * Lovelace (1.8ghz dual Macbook Air5,2) with 2 threads: 1.25s user 0.96s system 173% cpu 1.273 total
  * Lovelace (1.8ghz dual Macbook Air5,2) with 4 threads: 1.49s user 1.23s system 287% cpu 0.946 total
  * n-nu (ECS 360) with 4 threads: real	0m1.360s user	0m0.840s sys	0m1.076s
  * Stack (3.5ghz quad w/ hyperthreading) with 8 threads: 0.27s user 0.80s system 170% cpu 0.397 total
  * Stack (3.5ghz quad w/ hyperthreading) with 4 threads: 0.28s user 0.40s system 184% cpu 0.371 total

Single threaded: `cat words | ./palin`

  * Lovelace (1.8ghz dual Macbook Air5,2): 0.37s user 0.26s system 98% cpu 0.636 total
  * Lovelace (1.8ghz dual Macbook Air5,2): 0.36s user 0.24s system 99% cpu 0.603 total
  * Stack (3.5ghz quad): 0.13s user 0.18s system 98% cpu 0.320 total
  * Stack (3.5ghz quad): 0.15s user 0.15s system 98% cpu 0.302 total
  * n-nu (ECS 360): real	0m0.114s user	0m0.068s sys	0m0.040s

(For fun) JS, Single threaded: `cat words | node palindrome.js`

  * Lovelace (1.8 ghz dual Macbook Air5,2): 2.06s user 0.11s system 100% cpu 2.164 total
  * Stack (3.5ghz quad w/ hyperthreading): 1.16s user 0.05s system 100% cpu 1.207 total