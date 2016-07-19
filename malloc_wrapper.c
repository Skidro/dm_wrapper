#include <stdio.h>
#include "list.h"

/* These are the real calls to libc malloc and free */
void *__real_malloc(size_t);
void __real_free(void*);
static void print_stats(void);

typedef struct {
	struct list_head list;
	void   *address;
	unsigned long size;
} track_t;

typedef struct {
	unsigned long allocated;
	unsigned long freed;
	unsigned long alloc_count;
	unsigned long free_count;
	unsigned long max_alloc;
	unsigned long max_size;
} usage_t;

static usage_t heap_use = {0, 0, 0, 0, 0, 0};

static struct list_head alloc_list;
static int init = 0;

/* This function intercepts the call to malloc */
void *__wrap_malloc(size_t size)
{
	void *lptr = __real_malloc(size);

	/* Find out if this is the first call to malloc */
	if (init == 0) {
		init = 1;
		
		/* Initialize a linked list to track allocations */
		INIT_LIST_HEAD(&alloc_list);
	}

	/* Allocate a tracking structure for this allocation */
	track_t *tracker = (track_t *)__real_malloc(sizeof(track_t));

	/* Initialize the list-head in the tracker */
	INIT_LIST_HEAD(&tracker->list);

	/* Store the address of the current allocation in the tracker */
	tracker->address = lptr;

	/* Store the size of the current allocation in the tracker */
	tracker->size = (unsigned long)size;

	/* Add the tracker to the allocation list */
	list_add(&tracker->list, &alloc_list);
		
	/* Update the heap usage statistics for this program */
	heap_use.allocated += (unsigned long)size;
	heap_use.alloc_count += 1;

	if (heap_use.allocated - heap_use.freed > heap_use.max_alloc) {
		heap_use.max_alloc = heap_use.allocated - heap_use.freed;
	}

	if (heap_use.max_size < (unsigned long)size) {
		heap_use.max_size = (unsigned long)size;
	}

	return lptr;
}

/* This function intercepts the call to free */
void __wrap_free(void *ptr)
{
	track_t *tracker, *next;
	int     found = 0;

	/* Free the given pointer - No questions asked */
	__real_free(ptr);

	/* Traverse the list of trackers to find out the size of this allocation */
	list_for_each_entry_safe(tracker, next, &alloc_list, list) {
		if (tracker->address == ptr) {
			/* This is the tracker we are looking for */
			heap_use.freed += tracker->size;

			/* Delete this tracker from the linked list */
			list_del_init(&tracker->list);

			/* Free the structure associated with this tracker */
			__real_free(tracker);

			/* Mark the entry as found */
			found += 1;

			/* Braek the list traversal */
			break;
		}
	}

	if (found == 0) {
		/* Tracker was not found */
		printf("Tracker for pointer : %p not found in the Allocation Linked List!\n", ptr);
	} else if (found > 1) {
		/* Multiple trackers found */
		printf("Multiple trackers for pointer : %p found in the Allocation Linked List!\n", ptr);
	}
	
	heap_use.free_count += 1;

	print_stats();

	return;
}

/* This funciton is used to print-out the debug usage statistics */
static void print_stats(void)
{
	/* Print out the usage statistics to standard error stream for the ease of re-direction */
	fprintf(stderr, "================= Free  Request : %lu\n", heap_use.free_count);
	fprintf(stderr, " Memory Allcoation Requests     :	%10lu\n", heap_use.alloc_count);
	fprintf(stderr, " Dynamic Memory Touched         :	%10lu Bytes | %5lu KBytes\n", heap_use.allocated, heap_use.allocated/1024);
	fprintf(stderr, " Dynamic Memory Freed           :	%10lu Bytes | %5lu KBytes\n", heap_use.freed, heap_use.freed/1024);
	fprintf(stderr, " Maximum Allocation Size        :	%10lu Bytes | %5lu KBytes\n", heap_use.max_size, heap_use.max_size/1024);
	fprintf(stderr, " Maximum Allocated Memory       :	%10lu Bytes | %5lu KBytes\n\n", heap_use.max_alloc, heap_use.max_alloc/1024);

	return;
}
