#ifndef __EV_ALLOCATOR_INTERNAL_H__
#define __EV_ALLOCATOR_INTERNAL_H__

#include "ev/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Same as [malloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev__malloc(size_t size);

/**
 * @brief Same as [calloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev__calloc(size_t nmemb, size_t size);

/**
 * @brief Same as [realloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev__realloc(void* ptr, size_t size);

/**
 * @brief Same as [free(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void ev__free(void* ptr);

/**
 * @brief Same as [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html)
 */
char* ev__strdup(const char* str);

#ifdef __cplusplus
}
#endif

#endif
