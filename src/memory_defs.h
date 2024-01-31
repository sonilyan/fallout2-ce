#ifndef MEMORY_DEFS_H
#define MEMORY_DEFS_H

#include <stddef.h>

namespace fallout {

typedef void*(MallocProc)(char *,int b, size_t size);
typedef void*(ReallocProc)(void* ptr, size_t newSize);
typedef void(FreeProc)(char *a,int b, void* ptr);

} // namespace fallout

#endif /* MEMORY_DEFS_H */
