
#ifndef _GC_H
#define _GC_H
#include "tm.h"
#define GC_DEBUG_LIST 0

void*            tm_malloc( size_t size);
void*            tm_realloc(void* o, size_t osize, size_t nsize);
void             tm_free(void* o, size_t size);
void initMemory();
void freeMemory();

void        gcInit();
Object      gcTrack(Object obj);
void        gcDeinit();
void        gcFull();
Object bfGetMallocInfo();

void gcMark(Object);
void gcMarkList(TmList*);
void gcMarkDict(TmDict*);

#if 0
#define PRINT_OBJ_GC_INFO_START() int _gc_old = tm->allocated;
#define PRINT_OBJ_GC_INFO_END(str, addr) \
    printf("free %s at 0x%p, %d => %d, ", str, addr, _gc_old, tm->allocated);
#else
#define PRINT_OBJ_GC_INFO_START()
#define PRINT_OBJ_GC_INFO_END(str, addr)

#define GC_LOG_START(ptr, desc)
#define GC_LOG_END(ptr, desc)
#endif
#endif
