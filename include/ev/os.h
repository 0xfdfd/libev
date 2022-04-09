#ifndef __EV_OS_H__
#define __EV_OS_H__

/**
 * @def EV_FS_O_APPEND
 * @brief The file is opened in append mode. Before each write, the file offset
 *   is positioned at the end of the file.
 */

/**
 * @def EV_FS_O_CREAT
 * @brief The file is created if it does not already exist.
 */

/**
 * @def EV_FS_O_DSYNC
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and a minimum of metadata are flushed to disk.
 */

/**
 * @def EV_FS_O_EXCL
 * @brief If the `O_CREAT` flag is set and the file already exists, fail the open.
 */

/**
 * @def EV_FS_O_SYNC
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and all metadata are flushed to disk.
 */

/**
 * @def EV_FS_O_TRUNC
 * @brief If the file exists and is a regular file, and the file is opened
 *   successfully for write access, its length shall be truncated to zero.
 */

/**
 * @def EV_FS_O_RDONLY
 * @brief Open the file for read-only access.
 */

/**
 * @def EV_FS_O_WRONLY
 * @brief Open the file for write-only access.
 */

/**
 * @def EV_FS_O_RDWR
 * @brief Open the file for read-write access.
 */

/**
 * @def EV_FS_S_IRUSR
 * @brief User has read permission.
 */

/**
 * @def EV_FS_S_IWUSR
 * @brief User has write permission.
 */

/**
 * @def EV_FS_S_IXUSR
 * @brief User has execute permission.
 */

/**
 * @def EV_FS_S_IRWXU
 * @brief user (file owner) has read, write, and execute permission.
 */

#if defined(_WIN32)
#   include "ev/os_win.h"
#else
#   include "ev/os_unix.h"
#endif

#endif
