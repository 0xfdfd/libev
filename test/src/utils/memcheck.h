#ifndef __TEST_MEMCHECK_H__
#define __TEST_MEMCHECK_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup memory leak check.
 */
void mmc_setup(void);

/**
 * @brief Dump memory leak check result.
 */
void mmc_dump_exit(void);

/**
 * @brief same as malloc.
 */
void* mmc_malloc(size_t size);

/**
 * @brief Same as calloc.
 */
void* mmc_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as free.
 */
void mmc_free(void* ptr);

/**
 * @brief Same as realloc.
 */
void* mmc_realloc(void* ptr, size_t size);

/**
 * @brief Same as [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html).
 * Use #mmc_free() to free duplicated string.
 */
char* mmc_strdup(const char* str);

#ifdef __cplusplus
}
#endif
#endif
