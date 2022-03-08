#ifndef __EV_ALLOCATOR_INTERNAL_H__
#define __EV_ALLOCATOR_INTERNAL_H__

#include "ev/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

void* ev__malloc(size_t size);
void* ev__calloc(size_t nmemb, size_t size);
void* ev__realloc(void* ptr, size_t size);
void ev__free(void* ptr);

#ifdef __cplusplus
}
#endif
#endif
