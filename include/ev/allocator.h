#ifndef __EV_ALLOCATOR_H__
#define __EV_ALLOCATOR_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ALLOCATOR Allocator
 * @{
 */

/**
 * @brief Replacement function for realloc.
 * @see https://man7.org/linux/man-pages/man3/realloc.3.html
 */
typedef void *(*ev_realloc_fn)(void *ptr, size_t size);

/**
 * @brief Override the use of the standard library's malloc(3), calloc(3),
 *   realloc(3), free(3), memory allocation functions.
 *
 * This function must be called before any other function is called or after
 * all resources have been freed and thus doesn't reference any allocated
 * memory chunk.
 *
 * @warning There is no protection against changing the allocator multiple
 *   times. If the user changes it they are responsible for making sure the
 *   allocator is changed while no memory was allocated with the previous
 *   allocator, or that they are compatible.
 * @warngin Allocator must be thread-safe.
 *
 * @param[in] new_allocator   Replacement function..
 * @param[out] old_allocator  Old allocator functions.
 * @return #ev_errno_t.
 */
EV_API int ev_replace_allocator(ev_realloc_fn  new_allocator,
                                 ev_realloc_fn *old_allocator);

/**
 * @brief Same as [malloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void *ev_malloc(size_t size);

/**
 * @brief Same as [calloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void *ev_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as
 * [realloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void *ev_realloc(void *ptr, size_t size);

/**
 * @brief Same as [free(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void ev_free(void *ptr);

/**
 * @brief Same as
 * [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html)
 */
EV_API char *ev__strdup(const char *str);

/**
 * @} EV_ALLOCATOR
 */

#ifdef __cplusplus
}
#endif
#endif
