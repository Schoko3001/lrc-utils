#include <stdint.h>
/* !!! IT IS ASSUMED THAT <stdlib> IS INCLUDED SOMEWHERE !!! */



void* malloc(size_t size);
void* realloc(void *ptr, size_t new_size);


typedef struct _swapdelete{
	size_t capacity;
	size_t size;
	char* array;
} _sd;




static inline void* swapdelete_create() {

}
static inline void* swapdelete_destroy() {

}


static inline size_t swapdelete_addSlot() {

}
static inline size_t swapdelete_delSlot(_sd* array, size_t index) {

}