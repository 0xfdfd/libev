#ifndef __TEST_MEMCHECK_H__
#define __TEST_MEMCHECK_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup memory leak check.
 */
void setup_memcheck(void);

/**
 * @brief Dump memory leak check result.
 */
void dump_memcheck(void);

/**
 * @brief same as malloc.
 */
void* memcheck_malloc(size_t size);

/**
 * @brief Same as calloc.
 */
void* memcheck_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as free.
 */
void memcheck_free(void* ptr);

/**
 * @brief Same as realloc.
 */
void* memcheck_realloc(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif
#endif
