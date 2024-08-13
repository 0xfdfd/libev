#ifndef __UTILS_RANDOM_H__
#define __UTILS_RANDOM_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fill \p buffer with random data.
 * @param[out] buffer   Buffer to fill.
 * @param[in] size      Buffer size.
 */
void test_random(void* buffer, size_t size);

#ifdef __cplusplus
}
#endif
#endif
