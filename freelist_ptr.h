#include <stdint.h>
/* !!! IT IS ASSUMED THAT <stdlib> IS INCLUDED SOMEWHERE !!! */



/*	addSlot()
 *	  
 *
 *
 */


/* 
 *	How does this freelist work?
 *	
 *	growing size:
 *	  freelist uses chunks for automatic expansion
 *	          chunk1  chunk2       chunk3                 chunk4
 *	  [head]  [.....] [..........] [....................] [...
 *
 *	  A new chunk is created if no empty slot exists
 *	  size of a new chunk: (prev_chunk_size * 2 + sizeof(char*))
 *
 *	  at (chunk_ptr + size) lies a pointer to the prev chunk
 *	  this pointer is used destroy the list
 *
 *	extra:
 *	  Each slot has a minimum size of sizeof(void*)
 *	  thats probably 8 bytes
 */


void* malloc(size_t size);
void* realloc(void *ptr, size_t new_size);
void free(void *ptr);


// rounds up to stride % 8 == 0
#define _FL_STRIDE(stride) \
	(((stride) + 7) & ~7)

// optimized for least additions and instructions
// remove
static inline void _fl_init_slots(char* start_p, size_t stride, size_t size) {
	stride = _FL_STRIDE(stride);
	char* end_p = start_p + size;

	while (start_p < end_p) {
		void* next = start_p + stride;
		*(char**)start_p = next;
		start_p = next;
	}
	*(void**)(start_p - stride) = NULL;
};

typedef struct _freelist{
	void* free_head;
	char* chunk;

	size_t used_size;
	size_t total_size;
} _fl;

static inline void* freelist_create(size_t nmemb, size_t stride) {
	stride = _FL_STRIDE(stride);
	_fl* list = malloc(sizeof(_fl));

	list->total_size = nmemb * stride + sizeof(char*);
	list->chunk = malloc(list->total_size);

	list->free_head = NULL;
	list->used_size = 0;

	// first chunk (prev_chunk is null)
	*(char**)list->chunk = NULL;
	
	return list;
}
// fauly
static inline void freelist_destroy(_fl* list) {
	if (list == NULL) return;

	char* prev_chunk;
	size_t new_size = (list->total_size >> 1) - sizeof(char*);
	do {
		prev_chunk = *(char**)(list->new_chunk + size);
		free(list->new_chunk);
		list->new_chunk = prev_chunk;
	} while(list->new_chunk != NULL);

	free(list);
}

static inline void* freelist_addSlot(_fl* list, size_t stride) {
	stride = _FL_STRIDE(stride);

	if (list->free_head == NULL) {

		/* create new chunk */
		if (list->used_size == list->size) {
			
		}



	}


	/* create new chunk */
	else if (list->free_head == NULL) {

		char* prev_chunk = list->chunk;
		list->size <<= 1;
		list->chunk = malloc(list->size + sizeof(char*));
		list->free_head = (void*)list->chunk;

		*(char**)list->chunk = prev_chunk;
	}

	void* assigned_slot_p = list->free_head;
	list->free_head = *(void**)list->free_head;

	return assigned_slot_p;
}
static inline void freelist_freeSlot(_fl* list, void* slot_p) {
	*(void**)slot_p = list->free_head;
	list->free_head = slot_p;
}