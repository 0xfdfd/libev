#ifndef __TEST_FILE_H__
#define __TEST_FILE_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write data to file.
 * @param[in] path  File path.
 * @param[in] data  Data content to write
 * @param[in] size  Data size
 * @return          #ev_errno_t
 */
int test_write_file(const char* path, const void* data, size_t size);

#ifdef __cplusplus
}
#endif
#endif
