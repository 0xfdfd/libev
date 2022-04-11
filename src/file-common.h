#ifndef __EV_FILE_COMMON_H__
#define __EV_FILE_COMMON_H__

#include "ev-platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Close         file handle.
 * @return              #ev_errno_t
 */
API_LOCAL int ev__fs_close(ev_os_file_t file);

/**
 * @brief Open file.
 * @param[out] file     File handle.
 * @param[in] path      File path.
 * @param[in] flags     Open flags.
 * @param[in] mode      Creation mode.
 * @return              #ev_errno_t
 */
API_LOCAL int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode);

/**
 * @brief Same as [preadv(2)](https://linux.die.net/man/2/preadv)
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @param[in] offset    Offset.
 * @return              #ev_errno_t
 */
API_LOCAL ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset);

/**
 * @brief Same as [pwritev(2)](https://linux.die.net/man/2/pwritev)
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @param[in] offset    Offset.
 * @return              #ev_errno_t
 */
API_LOCAL ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset);

/**
 * @brief Same as [fstat(2)](https://linux.die.net/man/2/fstat)
 * @param[in] file      File handle.
 * @param[out] statbuf  File information.
 * @return              #ev_errno_t
 */
API_LOCAL int ev__fs_fstat(ev_os_file_t file, ev_file_stat_t* statbuf);

#ifdef __cplusplus
}
#endif
#endif
