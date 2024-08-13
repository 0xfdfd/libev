#ifndef __EV_FILESYSTEM_INTERNAL_H__
#define __EV_FILESYSTEM_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Readdir callback.
 * @param[in] info      Dirent information.
 * @param[in] arg       User defined data.
 * @return              non-zero to stop.
 */
typedef int (*ev_fs_readdir_cb)(ev_dirent_t* info, void* arg);

/**
 * @brief Close         file handle.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_close(ev_os_file_t file);

/**
 * @brief Open file.
 * @param[out] file     File handle.
 * @param[in] path      File path.
 * @param[in] flags     Open flags.
 * @param[in] mode      Creation mode.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_open(ev_os_file_t* file, const char* path, int flags,
    int mode);

/**
 * @brief Same as [seek(2)](https://linux.die.net/man/2/seek)
 * @param[in] file      File handle.
 * @param[in] whence    Directive.
 * @param[in] offset    Offset.
 * @return              #ev_errno_t
 */
EV_LOCAL int64_t ev__fs_seek(ev_os_file_t file, int whence, int64_t offset);

/**
 * @brief Same as [readv(2)](https://linux.die.net/man/2/readv)
 * @note For windows users, the \p file can NOT open with FILE_FLAG_OVERLAPPED.
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @return              #ev_errno_t
 */
EV_LOCAL ssize_t ev__fs_readv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Same as [preadv(2)](https://linux.die.net/man/2/preadv)
 * @note For windows users, the \p file can NOT open with FILE_FLAG_OVERLAPPED.
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @param[in] offset    Offset.
 * @return              #ev_errno_t
 */
EV_LOCAL ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf,
    int64_t offset);

/**
 * @brief Same as [writev(2)](https://linux.die.net/man/2/writev)
 * @note For windows users, the \p file can NOT open with FILE_FLAG_OVERLAPPED.
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @return              #ev_errno_t
 */
EV_LOCAL ssize_t ev__fs_writev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Same as [pwritev(2)](https://linux.die.net/man/2/pwritev)
 * @note For windows users, the \p file can NOT open with FILE_FLAG_OVERLAPPED.
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @param[in] offset    Offset.
 * @return              #ev_errno_t
 */
EV_LOCAL ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf,
    ssize_t offset);

/**
 * @brief Same as [fstat(2)](https://linux.die.net/man/2/fstat)
 * @param[in] file      File handle.
 * @param[out] statbuf  File information.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_fstat(ev_os_file_t file, ev_fs_stat_t* statbuf);

/**
 * @brief Same as [readdir(3)](https://man7.org/linux/man-pages/man3/readdir.3.html)
 * @param[in] path      Directory path. The path can be end with or without '/'.
 * @param[in] cb        Dirent callback.
 * @param[in] arg       User defined data.
 * @return              #ev_errno_t.
 */
EV_LOCAL int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg);

/**
 * @brief Same as [mkdir(2)](https://man7.org/linux/man-pages/man2/mkdir.2.html),
 *   and make parent directories as needed.
 * @param[in] path      Directory path.
 * @param[in] mode      The mode for the new directory.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_mkdir(const char* path, int mode);

/**
 * @brief Remove file or directory.
 * @param[in] path      File path.
 * @param[in] recursive Recursive if it is a directory.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_remove(const char* path, int recursive);

#ifdef __cplusplus
}
#endif
#endif
