/*
 * Skeleton code for CSC 360, Fall 2013, Assignment #3.
 *
 * Prepared by: Michael Zastre (University of Victoria) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define SCHEME_NONE 0
#define SCHEME_FIFO 1
#define SCHEME_LRU   2

int page_replacement_scheme = SCHEME_NONE;

#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 30
#define MAX_LINE_LEN 100

int size_of_frame = 0;   /* power of 2 */
int size_of_memory = 0; /* number of frames */

int initialize(void);
int finalize(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);

int page_faults  = 0;
int mem_refs     = 0;
int swap_outs    = 0;
int swap_ins     = 0;

struct page_table_entry *page_table = NULL;

struct page_table_entry {
 long page_num;
 int dirty;
 int free;
  // ---------------Begin Edit---------------- //
 int ticks_since_use;        // Have we used it yet this tick?
  // ---------------End Edit---------------- //
};


// ---------------Begin Edit---------------- //
int FIFO_pos = 0;
int select_by_FIFO(long page) {
  // Sweet! We need to drop the first placed slot.
  // This should be FIFO_pos which will be 0!
  // Replace the page at FIFO_pos.
  page_table[FIFO_pos].page_num = page;
  // TODO: Insert functionality to write to disk.
  // Increment FIFO pos.
  int temp_FIFO = FIFO_pos;
  FIFO_pos = (FIFO_pos + 1) % size_of_memory;
  // Return that page.
  return temp_FIFO;
}

// The number of slots we've marked down as used.
int LRU_tick_current = 0;
// The maximum number of slots we can do that for without having a recently used slot.
int select_by_LRU(long page) {
  // Hot dang! We need to drop the least recently used slot.
  // We can't really be **exactly** precise here, since it would mean bloating up the page table and doing a bunch of time junk.
  // Instead, we're going to use the size_of_memory (Say, 12) as a sort of flag. 
  // When we've used 'size_of_memory' different slots we'll walk through the slots and increment their 'ticks_since_use'
  
  // So, to select the least recently used, just find the number with the highest ticks_since_use. Picking the first if not.
  int i = 0;
  int highest_ticks_since_use = 0;
  int highest_index = 0;
  for ( i=0; i < size_of_memory; i++ ) {
    if (page_table[i].ticks_since_use > highest_ticks_since_use) {
      highest_ticks_since_use = page_table[i].ticks_since_use;
      highest_index = i;
    }
  }
  page_table[highest_index].page_num = page;
  page_table[highest_index].ticks_since_use = 0;
  
  return highest_index;
}
// ---------------End Edit---------------- //


long resolve_address(long logical, int memwrite)
{
 int i;
 long page, frame;
 long offset;
 long mask = 0;
 long effective;

 /* Get the page and offset */
 page = (logical >> size_of_frame);

 for (i=0; i<size_of_frame; i++) {
   mask = mask << 1;
   mask |= 1;
 }
 offset = logical & mask;

 /* Find page in the inverted page table. */
 frame = -1;
 for ( i = 0; i < size_of_memory; i++ ) {
   if (!page_table[i].free && page_table[i].page_num == page) {
     frame = i;
     // ---------------Begin Edit---------------- //
     LRU_tick_current = (LRU_tick_current + 1) % size_of_memory;
     // If we need to roll over, go over each memory frame and increment it's ticks_since_use.
     if (LRU_tick_current == 0) {
       int j;
       for (j=0; j<size_of_memory; j++) { page_table[j].ticks_since_use++; }
     }
     // Mark it's LRU_tick.
     page_table[i].ticks_since_use = 0;
     // ---------------End Edit---------------- //
     break;
   }
 }

 /* If frame is not -1, then we can successfully resolve the
   * address and return the result. */
 if (frame != -1) {
   effective = (frame << size_of_frame) | offset;
   return effective;
 }


 /* If we reach this point, there was a page fault. Find
   * a free frame. */
 page_faults++;

 for ( i = 0; i < size_of_memory; i++) {
   if (page_table[i].free) {
     frame = i;
     break;
   }
 }

 /* If we found a free frame, then patch up the
   * page table entry and compute the effective
   * address. Otherwise return -1.
   */
 if (frame != -1) {
   page_table[frame].page_num = page;
   page_table[i].free = FALSE;
   // ---------------Begin Edit---------------- //
   LRU_tick_current = (LRU_tick_current + 1) % size_of_memory;
   // If we need to roll over, go over each memory frame and increment it's ticks_since_use.
   if (LRU_tick_current == 0 && page_replacement_scheme == SCHEME_LRU) {
     int j;
     for (j=0; j<size_of_memory; j++) { page_table[j].ticks_since_use++; }
   }
   // Mark it's ticks_since_use to 0.
   page_table[i].ticks_since_use = 0;
   // ---------------End Edit---------------- //
   swap_ins++;
   effective = (frame << size_of_frame) | offset;
   return effective;
 } else {
    // ---------------Begin Edit---------------- //
   swap_outs++;   // Add a swap-out.
   swap_ins++;    // Also add a swap in, we're doing both after all.
   LRU_tick_current = (LRU_tick_current + 1) % size_of_memory;
   // If we need to roll over, go over each memory frame and increment it's ticks_since_use.
   if (LRU_tick_current == 0 && page_replacement_scheme == SCHEME_LRU) {
     int j;
     for (j=0; j<size_of_memory; j++) { page_table[j].ticks_since_use++; }
   }
    /* There are no free frames. Need to decide what to do based on
     * our replacement scheme.
     */
   switch (page_replacement_scheme) {
     case SCHEME_FIFO:
        /* Replace the first allocated memory spot. */
        frame = select_by_FIFO(page);
        break;
      case SCHEME_LRU:
        /* Replace the least recently used memory spot. */
        frame = select_by_LRU(page);
        break;
      default:
        return -1;
   }
  effective = (frame << size_of_frame) | offset;
  return effective;
    // ---------------End Edit---------------- //
 }
}


void display_progress(int percent)
{
 int to_date = PROGRESS_BAR_WIDTH * percent / 100;
 static int last_to_date = 0;
 int i;

 if (last_to_date < to_date) {
   last_to_date = to_date;
 } else {
   return;
 }

 printf("Progress [");
 for (i=0; i<to_date; i++) {
   printf(".");
 }
 for (; i<PROGRESS_BAR_WIDTH; i++) {
   printf(" ");
 }
 printf("] %3d%%", percent);
 printf("\r");
 fflush(stdout);
}


int initialize()
{
 int i;

 page_table = (struct page_table_entry *)malloc(sizeof(struct page_table_entry) * 
   size_of_memory);

 if (page_table == NULL) {
   fprintf(stderr, "Simulator error: cannot allocate memory for page table.\n");
   exit(1);
 }

 for (i=0; i<size_of_memory; i++) {
   page_table[i].free = TRUE;
 }

       return -1;
}


int finalize()
{

       return -1;
}


void error_resolve_address(long a, int l)
{
 fprintf(stderr, "\n");
 fprintf(stderr, "Simulator error: cannot resolve address 0x%lx at line %d\n",
   a, l);
 exit(1);
}


int output_report()
{
 printf("\n");
 printf("Memory references: %d\n", mem_refs);
 printf("Page faults: %d\n", page_faults);
 printf("Swap ins: %d\n", swap_ins);
 printf("Swap outs: %d\n", swap_outs);

       return -1;
}


int main(int argc, char **argv)
{
 int i;
 char *infile_name = NULL;
 struct stat infile_stat;
 FILE *infile = NULL;
 int infile_size = 0;
 char *s;
 int show_progress = FALSE;
 char buffer[MAX_LINE_LEN], is_write_c;
 long addr_inst, addr_operand;
 int   is_write;
 int line_num = 0;

 for (i=1; i < argc; i++) {
   if (strncmp(argv[i], "--scheme=", 9) == 0) {
     s = strstr(argv[i], "=") + 1;
     if (strcmp(s, "fifo") == 0) {
       page_replacement_scheme = SCHEME_FIFO;
     } else if (strcmp(s, "lru") == 0) {
       page_replacement_scheme = SCHEME_LRU;
     }   
   } else if (strncmp(argv[i], "--file=", 7) == 0) {
     infile_name = strstr(argv[i], "=") + 1;
   } else if (strncmp(argv[i], "--framesize=", 12) == 0) {
     s = strstr(argv[i], "=") + 1;
     size_of_frame = atoi(s);
   } else if (strncmp(argv[i], "--numframes=", 12) == 0) {
     s = strstr(argv[i], "=") + 1;
     size_of_memory = atoi(s);
   } else if (strcmp(argv[i], "--progress") == 0) {
     show_progress = TRUE;
   }
 }

 if (infile_name == NULL) {
   infile = stdin;
 } else if (stat(infile_name, &infile_stat) == 0) {
   infile_size = (int)(infile_stat.st_size);
   infile = fopen(infile_name, "r");   /* If this fails, infile will be null */
 }


 if (page_replacement_scheme == SCHEME_NONE ||
         size_of_frame <= 0 ||
         size_of_memory <= 0 ||
         infile == NULL)
 {
   fprintf(stderr, "usage: %s --framesize=<m> --numframes=<n> ", argv[0]);
   fprintf(stderr, "--scheme={fifo|lru} [--file=<filename>]\n");
   exit(1);
 }


 initialize();

 while (fgets(buffer, MAX_LINE_LEN-1, infile)) {
   line_num++;
   if (strstr(buffer, ":")) {
     sscanf(buffer, "%lx: %c %lx", &addr_inst, &is_write_c, &addr_operand);

       if (is_write_c == 'R') {
       is_write = FALSE;
     } else if (is_write_c == 'W') {
       is_write = TRUE;
     }   else {
       fprintf(stderr, "Simulator error: unknown memory operation at line %d\n",
         line_num);
       exit(1);
     }

     if (resolve_address(addr_inst, FALSE) == -1) {
       fprintf(stderr, "l346\n");
       error_resolve_address(addr_inst, line_num);
     }
     if (resolve_address(addr_operand, is_write) == -1) {
       fprintf(stderr, "l349\n");
       error_resolve_address(addr_operand, line_num);
     }
     mem_refs += 2;
   } else {
     sscanf(buffer, "%lx", &addr_inst);
     if (resolve_address(addr_inst, FALSE) == -1) {
      fprintf(stderr, "l357\n");
       error_resolve_address(addr_inst, line_num);
     }
     mem_refs++;
   }

   if (show_progress) {
     display_progress(ftell(infile) * 100 / infile_size);
   }
 }
 

 finalize();
 output_report();

 fclose(infile);

       exit(0);
}
