#include <stdint.h>
/* !!! IT IS ASSUMED THAT <stdlib> IS INCLUDED SOMEWHERE !!! */


/* FUNCTIONS:
 *	void* freelist_create(size_t nmeb, size_t memb_size);
 *	void  freelist_destroy(void* list);
 *
 *	void* freelist_addSlot(void* list, size_t memb_size);
 *	void  freelist_freeSlot(void* list, void* slot_p) 
 */


/* INFORMATION:
 *	This freelist grows dynamically
 *	This freelist does not shrink
 *
 *	This freelist does not use realloc but allocates extra chunks
 *	-> no continous buffer
 *
 *	This freelist uses void* instead of an index
 *	-> the size of a slot is rounded up to a multiple of sizeof(void*)
 */


void* malloc(size_t size);
void free(void *ptr);

// rounds up to stride % sizeof(void*) == 0
#define _FL_STRIDE(stride) \
	(((stride) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))


// freelist descriptor
struct _fl_desc{
	void* free_head;
	void* tail_chunk;
	size_t tail_used;
	size_t tail_size;
	size_t stride;
};

/*  Datastructure:
 *	The first sizeof(void*) bytes are not used as storage
 *	- They contain a pointer to a previous chunk
 *	- This is done for freelist_destroy(void* ptr)
 *	
 *	The rest is the actual array
 *	- member has the length list->stride
 *	- no indexes, only pointers
 */

// creates the dymamic freelist and returns its adress that is used for identification
static inline void* freelist_create(size_t nmemb, size_t memb_size) {
	struct _fl_desc* list = malloc(sizeof(struct _fl_desc));
	list->stride = _FL_STRIDE(memb_size);

	list->tail_size = nmemb * list->stride + sizeof(char*);
	list->tail_chunk = malloc(list->tail_size);

	list->free_head = NULL;
	list->tail_used = 0;

	// first chunk (prev does not exist)
	*(void**)list->tail_chunk = NULL;

	return list;
}
static inline void freelist_destroy(void* ptr) {
	if (ptr == NULL) return;
	struct _fl_desc* list = ptr;

	while (list->tail_chunk != NULL) {
		void* tmp = *(void**)list->tail_chunk;
		free(list->tail_chunk);
		list->tail_chunk = tmp;
	}

	free(list);
}

// returns void* to an unused slot
// might be filled with garbage values
static inline void* freelist_addSlot(void* ptr) {
	struct _fl_desc* list = ptr;

	if (list->free_head == NULL) {

		/* create new chunk if current full */
		if (list->tail_used == list->tail_size) {
			list->tail_size = (list->tail_size << 1) - sizeof(void*);

			void* new_chunk = malloc(list->tail_size);
			*(void**)new_chunk = list->tail_chunk;
			list->tail_chunk = new_chunk; 
			list->tail_used = sizeof(void*);
		}

		void* ret = (char*)list->tail_chunk + list->tail_used;
		list->tail_used += list->stride;
		return ret;
	}
	else {
		void* ret = list->free_head;
		list->free_head = *(void**)list->free_head;
		return ret;
	}
}
static inline void freelist_freeSlot(void* ptr, void* slot_p) {
	struct _fl_desc* list = ptr;
	*(void**)slot_p = list->free_head;
	list->free_head = slot_p;
}