#ifndef MEMORY_H
#define MEMORY_H

#include "memory_defs.h"

namespace fallout {

char* internal_strdup(const char* string);
void* internal_malloc(char *x,int b,size_t size);
void* internal_realloc(void* ptr, size_t size);
void internal_free(char* x, int b, void* ptr);
void mem_check();

} // namespace fallout

#endif /* MEMORY_H */
