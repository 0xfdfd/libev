#ifndef __EV_FS_WIN_INTERNAL_H__
#define __EV_FS_WIN_INTERNAL_H__

#include "fs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_dirent_w_s
{
    WCHAR*              name;           /**< Entry name */
    ev_dirent_type_t    type;           /**< Entry type */
}ev_dirent_w_t;

/**
 * @brief Directory information callback.
 * @param[in] info  Directory information.
 * @param[in] arg   User defined argument.
 * @return  non-zero to stop.
 */
typedef int (*ev_readdir_w_cb)(ev_dirent_w_t* info, void* arg);

/**
 * @brief Same as [readdir(3)](https://man7.org/linux/man-pages/man3/readdir.3.html)
 * @param[in] path      Directory path. The path can be end with or without '/'.
 * @param[in] cb        Dirent callback.
 * @param[in] arg       User defined data.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__fs_readdir_w(const WCHAR* path, ev_readdir_w_cb cb, void* arg);

#ifdef __cplusplus
}
#endif

#endif
