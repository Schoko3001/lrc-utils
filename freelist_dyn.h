#include <stdint.h>
/* !!! IT IS ASSUMED THAT <stdlib> IS INCLUDED SOMEWHERE !!! */

/*
*	dynamic freelist header
*
*	- The freelist doubles in size if full
*	  This requires a realloc which can be quite slow
*	- No safety checks
*	- There is an empty Slot at index 0	  
*/

/* Extra:
*	
*	A FREELIST_UINT_SIZE can be defined (8, 16, 32, 64)
*	Usecase:
*		By default a slot takes up atleast "sizeof(size_t)" bytes
*		For a list with very small members, this can waste memory
*		"#define FREELIST_UINT_SIZE 16" reduces the minimum slot size
*
*	Drawback:
*		The user has to watch out for an integer overflow of the index
*		Rule: "(n_memb + 1) * memb_size" has to be smaller than the uint limit
*/


// I did not include stdlib.h to keep the header size at a minimum
void* malloc(size_t size);
void* realloc(void *ptr, size_t new_size);


// FREELIST_UINT_SIZE
#if !defined(FREELIST_UINT_SIZE)
typedef size_t _fl_uint;
#elif FREELIST_UINT_SIZE == 64
typedef uint64_t _fl_uint;
#elif FREELIST_UINT_SIZE == 32
typedef uint32_t _fl_uint;
#elif FREELIST_UINT_SIZE == 16
typedef uint16_t _fl_uint;
#elif FREELIST_UINT_SIZE == 8
typedef uint8_t _fl_uint;
#else
#error Invalid FREELIST_UINT_SIZE definition
#endif


#define _FL_STRIDE(stride) \
	((stride) > sizeof(_fl_uint) ? (stride) : sizeof(_fl_uint))


typedef struct _freelist_dyn {
	char* buffer;
	_fl_uint free_head;
	_fl_uint size;
} _fl_dyn;

// stride might be too small, make check and confirm
static inline void* freelist_dyn_create(size_t nmemb, size_t stride) {
	stride = _FL_STRIDE(stride);
	_fl_dyn* list = malloc(sizeof(_fl_dyn));
	
	list->free_head = stride;
	list->size = (nmemb + 1) * stride;
	list->buffer = malloc(list->size);

	/* init slots */
	_fl_uint i = stride;
	while (i < list->size) {
		_fl_uint next = i + stride;
		*(_fl_uint *)(list->buffer + i) = next;
		i = next;
	}

	*(_fl_uint*)(list->buffer + i - stride) = 0;

	return list;
}
static inline void freelist_dyn_delete(_fl_dyn* list) {
	free(list->buffer);
	free(list);
}

static inline void* _freelist_dyn_addSlot_impl(_fl_dyn* list, _fl_uint* index_p, _fl_uint stride) {
	void* slot;

	/* expand existing list */
	if (list->free_head == 0) {
		list->free_head = list->size;
		list->size = (list->size << 1) - stride;
		list->buffer = realloc(list->buffer, list->size);

		/* init new slots */
		_fl_uint i = list->free_head;
		while (i < list->size) {
			_fl_uint next = i + stride;
			*(_fl_uint *)(list->buffer + i) = next;
			i = next;
		}

		*(_fl_uint*)(list->buffer + i - stride) = 0;
	}

	// lookup free slot
	*index_p = list->free_head;
	slot = list->buffer + list->free_head;
	list->free_head = *(_fl_uint*)slot;

	return (void*)slot;
}
static inline void* _freelist_dyn_getRaw_impl(_fl_dyn* list, _fl_uint index) {
	return list->buffer + index;
}
static inline void _freelist_dyn_freeSlot_impl(_fl_dyn* list, _fl_uint index) {
	*(_fl_uint*)(list->buffer + index) = list->free_head;
	list->free_head = index;
}


#define freelist_dyn_addSlot(ptr, index_p, stride) \
	_freelist_dyn_addSlot_impl(ptr, index_p, _FL_STRIDE(stride))
#define freelist_dyn_getRaw(ptr, index) \
	_freelist_dyn_getRaw_impl(ptr, index)
#define freelist_dyn_freeSlot(ptr, index) \
	_freelist_dyn_freeSlot_impl(ptr, index)

