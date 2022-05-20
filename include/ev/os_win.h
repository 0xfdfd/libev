#ifndef __EV_OS_WIN_H__
#define __EV_OS_WIN_H__

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT   0x0600
#endif

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#   define SSIZE_MAX INTPTR_MAX
#   define _SSIZE_T_
#   define _SSIZE_T_DEFINED
#endif

/**
 * @addtogroup EV_FILESYSTEM
 * @{
 */

/**
 * @brief The file is opened in append mode. Before each write, the file offset
 *   is positioned at the end of the file.
 */
#define EV_FS_O_APPEND          _O_APPEND

/**
 * @brief The file is created if it does not already exist.
 */
#define EV_FS_O_CREAT           _O_CREAT

/**
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and a minimum of metadata are flushed to disk.
 */
#define EV_FS_O_DSYNC           FILE_FLAG_WRITE_THROUGH

/**
 * @brief If the `O_CREAT` flag is set and the file already exists, fail the open.
 */
#define EV_FS_O_EXCL            _O_EXCL

/**
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and all metadata are flushed to disk.
 */
#define EV_FS_O_SYNC            FILE_FLAG_WRITE_THROUGH

/**
 * @brief If the file exists and is a regular file, and the file is opened
 *   successfully for write access, its length shall be truncated to zero.
 */
#define EV_FS_O_TRUNC           _O_TRUNC

/**
 * @brief Open the file for read-only access.
 */
#define EV_FS_O_RDONLY          _O_RDONLY

/**
 * @brief Open the file for write-only access.
 */
#define EV_FS_O_WRONLY          _O_WRONLY

/**
 * @def EV_FS_O_RDWR
 * @brief Open the file for read-write access.
 */
#define EV_FS_O_RDWR            _O_RDWR

/**
 * @brief User has read permission.
 */
#define EV_FS_S_IRUSR           _S_IREAD

/**
 * @brief User has write permission.
 */
#define EV_FS_S_IWUSR           _S_IWRITE

/**
 * @brief User has execute permission.
 */
#define EV_FS_S_IXUSR           _S_IEXEC

/**
 * @brief user (file owner) has read, write, and execute permission.
 */
#define EV_FS_S_IRWXU           (EV_FS_S_IRUSR | EV_FS_S_IWUSR | EV_FS_S_IXUSR)

typedef HANDLE                  ev_os_file_t;
#define EV_OS_FILE_INVALID      INVALID_HANDLE_VALUE

 /**
  * @} EV_FILESYSTEM
  */

/**
 * @addtogroup EV_PROCESS
 * @{
 */

typedef HANDLE                  ev_os_pid_t;
#define EV_OS_PID_INVALID       INVALID_HANDLE_VALUE

/**
 * @} EV_PROCESS
 */

typedef HANDLE                  ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      INVALID_HANDLE_VALUE

typedef SOCKET                  ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    INVALID_SOCKET

typedef DWORD                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((DWORD)(-1))

typedef HANDLE                  ev_os_thread_t;
typedef DWORD                   ev_os_tls_t;
typedef CRITICAL_SECTION        ev_os_mutex_t;
typedef HANDLE                  ev_os_sem_t;

#ifdef __cplusplus
}
#endif

#endif
