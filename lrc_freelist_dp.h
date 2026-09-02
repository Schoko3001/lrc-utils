#pragma once
// dynamic pointer based freelist

#include <stdint.h>
/* !!! IT IS ASSUMED THAT <stdlib> IS INCLUDED SOMEWHERE !!! */


/** FUNCTIONS **
 *	void* lrc_freelistdp_create(size_t nmeb, size_t memb_size);
 *	void  lrc_freelistdp_destroy(void* list);
 *
 *	void* lrc_freelistdp_addSlot(void* list, size_t memb_size);
 *	void  lrc_freelistdp_freeSlot(void* list, void* slot_p) 
 **/

/** INFORMATION **
 *	This freelist grows dynamically
 *	This freelist does not shrink
 *
 *	This freelist does not use realloc but allocates extra chunks
 *	-> no continous buffer
 *
 *	This freelist uses void* instead of an index
 *	-> the size of a slot is rounded up to a multiple of sizeof(void*)
 **/

/** PERFORMANCE **
 * 	Upsides:
 *	-> O(1) allocation/deallocation
 *	-> No realloc overhead
 *
 * 	Downsides:
 * 	-> Cache fragmentation with log2(n) buffers
 * 	-> destroy() has ~log2(n) pointer chaising
 * 	-> no shrinking 
 **/


void* malloc(size_t size);
void free(void *ptr);

/** CUSTOMIZATION **
 *	_FLDP_NEW_CHUNK_SIZE(variable)
 *	-> transformation of buffer size for each new chunk
 *	!!! sizeof(void*) will be added to the true size !!!
 *
 *	_FLDP_CHUNK_MALLOC(size)
 *	-> allocation method for memory
 *	- must return void*
 **/


#ifndef _FLDP_NEW_CHUNK_SIZE
#define _FLDP_NEW_CHUNK_SIZE(variable) (variable) = (variable) << 1
#endif

#ifndef _FLDP_CHUNK_MALLOC
#define _FLDP_CHUNK_MALLOC(size) malloc((size))
#endif



// rounds up to stride % sizeof(void*) == 0
#define _FL_STRIDE(stride) \
	(((stride) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))


// freelist descriptor
struct _fldp_desc{
	void* free_head;
	void* tailChunk;
	size_t tailChunk_used;
	size_t tailChunk_size;
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
static inline void* lrc_freelistdp_create(size_t nmemb, size_t memb_size) {
	const int stride = _FL_STRIDE(memb_size);
	struct _fldp_desc* list = malloc(sizeof(struct _fldp_desc));

	list->tailChunk_size = nmemb * stride + sizeof(char*);
	list->tailChunk = malloc(list->tailChunk_size);

	list->free_head = NULL;
	list->tailChunk_used = 0;

	// first chunk (prev does not exist)
	*(void**)list->tailChunk = NULL;

	return list;
}
static inline void lrc_freelistdp_destroy(void* ptr) {
	if (ptr == NULL) return;
	struct _fldp_desc* list = ptr;

	while (list->tailChunk != NULL) {
		void* tmp = *(void**)list->tailChunk;
		free(list->tailChunk);
		list->tailChunk = tmp;
	}

	free(list);
}

// returns void* to an unused slot
// might be filled with garbage values
static inline void* lrc_freelistdp_addSlot(void* ptr, size_t memb_size) {
	const int stride = _FL_STRIDE(memb_size);	
	struct _fldp_desc* list = ptr;

	if (list->free_head == NULL) {

		/* create new chunk if current full */
		if (list->tailChunk_used == list->tailChunk_size) {
			list->tailChunk_size -= sizeof(void*);
			_FLDP_NEW_CHUNK_SIZE(list->tailChunk_size);
			list->tailChunk_size += sizeof(void*);

			void* new_chunk = _FLDP_CHUNK_MALLOC(list->tailChunk_size);
			*(void**)new_chunk = list->tailChunk;
			list->tailChunk = new_chunk; 
			list->tailChunk_used = sizeof(void*);
		}

		void* ret = (char*)list->tailChunk + list->tailChunk_used;
		list->tailChunk_used += stride;
		return ret;
	}
	else {
		void* ret = list->free_head;
		list->free_head = *(void**)list->free_head;
		return ret;
	}
}
static inline void lrc_freelistdp_freeSlot(void* ptr, void* slot_p) {
	struct _fldp_desc* list = ptr;
	*(void**)slot_p = list->free_head;
	list->free_head = slot_p;
}