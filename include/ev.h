/**
 * @mainpage libev
 *
 * \section EV_OVERVIEW Overview
 *
 * [libev](https://github.com/qgymib/libev) is a rework of
 * [libuv](https://github.com/libuv/libuv), with following advantages:
 * 1. Strong types without static casts any more.
 * 2. Enhanced IPC features.
 * 3. Easy to use file system operations.
 *
 *
 * \section EV_DOCUMENTATION Documentation
 *
 * Located in the docs/ subdirectory. It use [Doxygen](http://www.doxygen.nl/)
 * to build documents, which means the source code is well documented and can
 * be read directly without build it.
 *
 * To build html-based documents, in the project root, run:
 * ```bash
 * doxygen docs/Doxyfile
 * ```
 *
 * Also documents can be browsed online [here](https://qgymib.github.io/libev/).
 *
 *
 * \section EV_BUILD_INSTRUCTIONS Build Instructions
 *
 * [CMake](https://cmake.org/) is currently the prefer way to build:
 *
 * ```bash
 * # Build:
 * $ mkdir -p build
 * $ (cd build && cmake .. -DBUILD_TESTING=ON) # generate project with tests
 * $ cmake --build build                       # add `-j <n>` with cmake >= 3.12
 * 
 * # Run tests:
 * $ (cd build && ctest -C Debug --output-on-failure)
 * ```
 *
 * \section EV_AMALGAMATE Amalgamate
 * 
 * [libev](https://github.com/qgymib/libev) support amalgamate build, which
 * allow to distribute libev's source code using only two files(`ev.h` and `ev.c`).
 *
 * > Note: Amalgamate requires python3.
 *
 * To use amalgamation, add `-DEV_AMALGAMATE_BUILD=on` when configurate cmake:
 *
 * ```bash
 * $ cmake -DEV_AMALGAMATE_BUILD=on /path/to/libev
 * $ cmake --build .
 * ```
 *
 * In `${CMAKE_CURRENT_BINARY_DIR}/amalgamate` directory, you will find the
 * generated files.
 *
 */
#ifndef __EV_H__
#define __EV_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_VERSION Version
 * @{
 */

/**
 * @brief Major version.
 */
#define EV_VERSION_MAJOR            0

/**
 * @brief Minor version.
 */
#define EV_VERSION_MINOR            0

/**
 * @brief Patch version.
 */
#define EV_VERSION_PATCH            8

/**
 * @brief Development version.
 */
#define EV_VERSION_PREREL           19

/**
 * @brief Version calculate helper macro.
 * @param[in] a     Major version.
 * @param[in] b     Minor version.
 * @param[in] c     Patch version.
 * @param[in] d     Development version.
 */
#define EV_VERSION(a, b, c, d)      (((a) << 24) + ((b) << 16) + ((c) << 8) + ((d) ? (d) : 255))

/**
 * @brief Current version code.
 */
#define EV_VERSION_CODE             \
    EV_VERSION(EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH, EV_VERSION_PREREL)

/**
 * @brief Get version code as c string.
 * @return      Version code.
 */
const char* ev_version_str(void);

/**
 * @brief Get version code as number.
 * @return      Version code
 */
unsigned ev_version_code(void);

/**
 * @} EV_VERSION
 */

/**
 * @defgroup EV_OS OS
 * @{
 */

#if defined(_WIN32)

#   ifndef _WIN32_WINNT
#       define _WIN32_WINNT   0x0600
#   endif
#   include <winsock2.h>
#   include <mswsock.h>
#   include <ws2tcpip.h>
#   include <windows.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <stddef.h>
#   include <stdint.h>
#   include <errno.h>

#   if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#       define SSIZE_MAX INTPTR_MAX
#       define _SSIZE_T_
#       define _SSIZE_T_DEFINED
#   endif

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

/**
 * @brief The starting point is zero or the beginning of the file.
 */
#define EV_FS_SEEK_BEG           FILE_BEGIN

/**
 * @brief The starting point is the current value of the file pointer.
 */
#define EV_FS_SEEK_CUR           FILE_CURRENT

/**
 * @brief The starting point is the current end-of-file position.
 */
#define EV_FS_SEEK_END           FILE_END

/**
 * @brief Windows system define of file.
 */
typedef HANDLE                   ev_os_file_t;

/**
 * @brief Invalid valid of #ev_os_file_t.
 */
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
#define EV_OS_THREAD_INVALID    INVALID_HANDLE_VALUE

typedef DWORD                   ev_os_tls_t;
typedef CRITICAL_SECTION        ev_os_mutex_t;
typedef HANDLE                  ev_os_sem_t;

#else   /* if defined(_WIN32)  */

#   include <errno.h>
#   include <netinet/in.h>
#   include <sys/epoll.h>
#   include <pthread.h>
#   include <semaphore.h>
#   include <fcntl.h>
#   include <stddef.h>
#   include <stdint.h>
#   include <errno.h>

#if defined(O_APPEND)
#   define EV_FS_O_APPEND       O_APPEND
#else
#   define EV_FS_O_APPEND       0
#endif

#if defined(O_CREAT)
#   define EV_FS_O_CREAT        O_CREAT
#else
#   define EV_FS_O_CREAT        0
#endif

#if defined(O_DSYNC)
#   define EV_FS_O_DSYNC        O_DSYNC
#else
#   define EV_FS_O_DSYNC        0
#endif

#if defined(O_EXCL)
#   define EV_FS_O_EXCL         O_EXCL
#else
#   define EV_FS_O_EXCL         0
#endif

#if defined(O_SYNC)
#   define EV_FS_O_SYNC         O_SYNC
#else
#   define EV_FS_O_SYNC         0
#endif

#if defined(O_TRUNC)
#   define EV_FS_O_TRUNC        O_TRUNC
#else
#   define EV_FS_O_TRUNC        0
#endif

#if defined(O_RDONLY)
#   define EV_FS_O_RDONLY       O_RDONLY
#else
#   define EV_FS_O_RDONLY       0
#endif

#if defined(O_WRONLY)
#   define EV_FS_O_WRONLY       O_WRONLY
#else
#   define EV_FS_O_WRONLY       0
#endif

#if defined(O_RDWR)
#   define EV_FS_O_RDWR         O_RDWR
#else
#   define EV_FS_O_RDWR         0
#endif

#define EV_FS_S_IRUSR           S_IRUSR
#define EV_FS_S_IWUSR           S_IWUSR
#define EV_FS_S_IXUSR           S_IXUSR
#define EV_FS_S_IRWXU           S_IRWXU

#define EV_FS_SEEK_BEG          SEEK_SET
#define EV_FS_SEEK_CUR          SEEK_CUR
#define EV_FS_SEEK_END          SEEK_END

typedef pid_t                   ev_os_pid_t;
#define EV_OS_PID_INVALID       ((pid_t)-1)

typedef int                     ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      (-1)

typedef int                     ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    (-1)

typedef pid_t                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((pid_t)(-1))

typedef int                     ev_os_file_t;
#define EV_OS_FILE_INVALID      (-1)

typedef pthread_t               ev_os_thread_t;
#define EV_OS_THREAD_INVALID    ((pthread_t)(-1))

typedef pthread_key_t           ev_os_tls_t;
typedef pthread_mutex_t         ev_os_mutex_t;
typedef sem_t                   ev_os_sem_t;

#endif  /* if defined(_WIN32)  */

/**
 * @brief Infinite timeout.
 */
#define EV_INFINITE_TIMEOUT         ((uint32_t)-1)

/**
 * @} EV_OS
 */

/**
 * @defgroup EV_ERRNO Error number
 * @{
 */

#define EV_ERRNO_POSIX_MAP(xx)  \
    xx(EV_EPERM,            EPERM,              "Operation not permitted")                  \
    xx(EV_ENOENT,           ENOENT,             "No such file or directory")                \
    xx(EV_EIO,              EIO,                "Host is unreachable")                      \
    xx(EV_E2BIG,            E2BIG,              "Argument list too long")                   \
    xx(EV_EBADF,            EBADF,              "Bad file descriptor")                      \
    xx(EV_EAGAIN,           EAGAIN,             "Resource temporarily unavailable")         \
    xx(EV_ENOMEM,           ENOMEM,             "Not enough space/cannot allocate memory")  \
    xx(EV_EACCES,           EACCES,             "Permission denied")                        \
    xx(EV_EFAULT,           EFAULT,             "Bad address")                              \
    xx(EV_EBUSY,            EBUSY,              "Device or resource busy")                  \
    xx(EV_EEXIST,           EEXIST,             "File exists")                              \
    xx(EV_EXDEV,            EXDEV,              "Improper link")                            \
    xx(EV_ENOTDIR,          ENOTDIR,            "Not a directory")                          \
    xx(EV_EISDIR,           EISDIR,             "Is a directory")                           \
    xx(EV_EINVAL,           EINVAL,             "Invalid argument")                         \
    xx(EV_EMFILE,           EMFILE,             "Too many open files")                      \
    xx(EV_ENOSPC,           ENOSPC,             "No space left on device")                  \
    xx(EV_EROFS,            EROFS,              "Read-only filesystem")                     \
    xx(EV_EPIPE,            EPIPE,              "Broken pipe")                              \
    xx(EV_ENAMETOOLONG,     ENAMETOOLONG,       "Filename too long")                        \
    xx(EV_ENOTEMPTY,        ENOTEMPTY,          "Directory not empty")                      \
    xx(EV_EADDRINUSE,       EADDRINUSE,         "Address already in use")                   \
    xx(EV_EADDRNOTAVAIL,    EADDRNOTAVAIL,      "Address not available")                    \
    xx(EV_EAFNOSUPPORT,     EAFNOSUPPORT,       "Address family not supported")             \
    xx(EV_EALREADY,         EALREADY,           "Connection already in progress")           \
    xx(EV_ECANCELED,        ECANCELED,          "Operation canceled")                       \
    xx(EV_ECONNABORTED,     ECONNABORTED,       "Connection aborted")                       \
    xx(EV_ECONNREFUSED,     ECONNREFUSED,       "Connection refused")                       \
    xx(EV_ECONNRESET,       ECONNRESET,         "Connection reset")                         \
    xx(EV_EHOSTUNREACH,     EHOSTUNREACH,       "Host is unreachable")                      \
    xx(EV_EINPROGRESS,      EINPROGRESS,        "Operation in progress")                    \
    xx(EV_EISCONN,          EISCONN,            "Socket is connected")                      \
    xx(EV_ELOOP,            ELOOP,              "Too many levels of symbolic links")        \
    xx(EV_EMSGSIZE,         EMSGSIZE,           "Message too long")                         \
    xx(EV_ENETUNREACH,      ENETUNREACH,        "Network unreachable")                      \
    xx(EV_ENOBUFS,          ENOBUFS,            "No buffer space available")                \
    xx(EV_ENOTCONN,         ENOTCONN,           "The socket is not connected")              \
    xx(EV_ENOTSOCK,         ENOTSOCK,           "Not a socket")                             \
    xx(EV_ENOTSUP,          ENOTSUP,            "Operation not supported")                  \
    xx(EV_EPROTO,           EPROTO,             "Protocol error")                           \
    xx(EV_EPROTONOSUPPORT,  EPROTONOSUPPORT,    "Protocol not supported")                   \
    xx(EV_ETIMEDOUT,        ETIMEDOUT,          "Operation timed out")

/**
 * @brief Error number
 */
typedef enum ev_errno
{
#define EV_EXPAND_ERRMAP(err, syserr, str) err = -syserr,
    EV_ERRNO_POSIX_MAP(EV_EXPAND_ERRMAP)
#undef EV_EXPAND_ERRMAP

    EV_EOF = -4095,
}ev_errno_t;

/**
 * @brief Describe the error code
 * @param[in] err   Error code
 * @return          Describe string
 */
const char* ev_strerror(int err);

/**
 * @} EV_ERRNO
 */

/**
 * @defgroup EV_ALLOCATOR Allocator
 * @{
 */

/**
 * @brief Replacement function for malloc.
 * @see https://man7.org/linux/man-pages/man3/malloc.3.html
 */
typedef void* (*ev_malloc_fn)(size_t size);

/**
 * @brief Replacement function for calloc.
 * @see https://man7.org/linux/man-pages/man3/calloc.3.html
 */
typedef void* (*ev_calloc_fn)(size_t nmemb, size_t size);

/**
 * @brief Replacement function for realloc.
 * @see https://man7.org/linux/man-pages/man3/realloc.3.html
 */
typedef void* (*ev_realloc_fn)(void* ptr, size_t size);

/**
 * @brief Replacement function for free.
 * @see https://man7.org/linux/man-pages/man3/free.3.html
 */
typedef void (*ev_free_fn)(void* ptr);

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
 * @param[in] malloc_func   Replacement function for malloc.
 * @param[in] calloc_func   Replacement function for calloc.
 * @param[in] realloc_func  Replacement function for realloc.
 * @param[in] free_func     Replacement function for free.
 * @return On success, it returns 0. if any of the function pointers is NULL it returns #EV_EINVAL.
 */
int ev_replace_allocator(ev_malloc_fn malloc_func, ev_calloc_fn calloc_func,
    ev_realloc_fn realloc_func, ev_free_fn free_func);

/**
 * @brief Same as [malloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev_malloc(size_t size);

/**
 * @brief Same as [calloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as [realloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void* ev_realloc(void* ptr, size_t size);

/**
 * @brief Same as [free(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
void ev_free(void* ptr);

/**
 * @} EV_ALLOCATOR
 */

#include "ev/async.h"
#include "ev/buf.h"
#include "ev/fs.h"
#include "ev/handle.h"
#include "ev/list.h"
#include "ev/loop.h"
#include "ev/map.h"
#include "ev/misc.h"
#include "ev/mutex.h"
#include "ev/once.h"
#include "ev/pipe.h"
#include "ev/process.h"
#include "ev/queue.h"
#include "ev/sem.h"
#include "ev/shmem.h"
#include "ev/tcp.h"
#include "ev/thread.h"
#include "ev/threadpool.h"
#include "ev/time.h"
#include "ev/timer.h"
#include "ev/udp.h"

#ifdef __cplusplus
}
#endif
#endif
