#ifndef __TEST_FILE_H__
#define __TEST_FILE_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write data to file.
 * @param[in] path      File path.
 * @param[in] data      Data content to write
 * @param[in] size      Data size
 * @return              #ev_errno_t
 */
int test_write_file(const char* path, const void* data, size_t size);

/**
 * @brief Check a directory exist.
 * @param[in] path      Directory path.
 * @return              #ev_errno_t
 */
int test_access_dir(const char* path);

/**
 * @brief Path to current executable.
 * @return Path.
 */
const char* test_get_self_exe(void);

/**
 * @brief Path to current executable directory.
 * @return Path
 */
const char* test_get_self_dir(void);

/**
 * @brief Get current executable name.
 * @return Executable name.
 */
const char* test_self_exe_name(void);

/**
 * @brief Get file name from full path.
 * @param file Full file path.
 * @return file name.
 */
const char* test_file_name(const char* file);

#ifdef __cplusplus
}
#endif
#endif
