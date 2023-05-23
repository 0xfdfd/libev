/**
 * MIT License
 * 
 * Copyright (c) 2021 qgymib
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define EV_AMALGAMATE_BUILD
#define EV_EXPOSE_SYMBOLS
#define _GNU_SOURCE
#include "ev.h"
#line 1 "src/defs.h"
#ifndef __EV_DEFINES_INTERNAL_H__
#define __EV_DEFINES_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */

#if defined(_WIN32)
#   include <WS2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define EV_IPC_FRAME_HDR_MAGIC  (0x48465645)
#elif __BYTE_ORDER == __BIG_ENDIAN
#   define EV_IPC_FRAME_HDR_MAGIC  (0x45564648)
#else
#   error unknown endian
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define EV_MIN(a, b)    ((a) < (b) ? (a) : (b))

#define EV_JOIN(a, b)   EV_JOIN_2(a, b)
#define EV_JOIN_2(a, b) a##b

/**
 * @brief Align \p size to \p align, who's value is larger or equal to \p size
 *   and can be divided with no remainder by \p align.
 * @note \p align must equal to 2^n
 */
#define ALIGN_SIZE(size, align) \
    (((uintptr_t)(size) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1))

#define ACCESS_ONCE(TYPE, var)  (*(volatile TYPE*) &(var))

/**
 * @brief exchange value of \p v1 and \p v2.
 * @note \p v1 and \p v2 must have the same type.
 * @param[in] TYPE      Type of \p v1 and \p v2.
 * @param[in,out] v1    value1
 * @param[in,out] v2    value2
 */
#define EXCHANGE_VALUE(TYPE, v1, v2)    \
    do {\
        TYPE _tmp = v1;\
        v1 = v2;\
        v2 = _tmp;\
    } while(0)

/**
 * @def EV_COUNT_ARG
 * @brief Count the number of arguments in macro
 */
#ifdef _MSC_VER // Microsoft compilers
#   define EV_COUNT_ARG(...)  _EV_INTERNAL_EXPAND_ARGS_PRIVATE(_EV_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define _EV_INTERNAL_EXPAND_ARGS_PRIVATE(...) EV_EXPAND(_EV_INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#else // Non-Microsoft compilers
#   define EV_COUNT_ARG(...) _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#endif

#define ENSURE_LAYOUT(TYPE_A, FIELD_A_1, FIELD_A_2, TYPE_B, FIELD_B_1, FIELD_B_2)   \
    assert(sizeof(TYPE_A) == sizeof(TYPE_B));\
    assert(offsetof(TYPE_A, FIELD_A_1) == offsetof(TYPE_B, FIELD_B_1));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_1) == sizeof(((TYPE_B*)0)->FIELD_B_1));\
    assert(offsetof(TYPE_A, FIELD_A_2) == offsetof(TYPE_B, FIELD_B_2));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_2) == sizeof(((TYPE_B*)0)->FIELD_B_2))

#define EV_ABORT(fmt, ...)  \
    do {\
        fprintf(stderr, "%s:%d:" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
        abort();\
    } while (0)

#if defined(__GNUC__) || defined(__clang__)
#   define EV_NORETURN  __attribute__ ((__noreturn__))
#elif defined(_MSC_VER)
#   define EV_NORETURN  __declspec(noreturn)
#else
#   define EV_NORETURN
#endif

/**
 * @brief The maximum length for a path.
 * In Windows API, the Ansi version of `MAX_PATH` is defined as 260.
 * The unicode version does not define macro as `MAX_PATH`, but NTFS does
 * support up to 32768 characters in length, but only when using the Unicode
 * APIs.
 *
 * @see https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
 * @see https://docs.microsoft.com/en-us/cpp/c-runtime-library/path-field-limits?view=msvc-170
 */
#if defined(_WIN32)
#   define WIN32_UNICODE_PATH_MAX 32768
#endif

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

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/allocator.h"
#ifndef __EV_ALLOCATOR_INTERNAL_H__
#define __EV_ALLOCATOR_INTERNAL_H__

/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Same as [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html)
 */
EV_LOCAL char* ev__strdup(const char* str);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/async.h"
#ifndef __EV_ASYNC_COMMON_H__
#define __EV_ASYNC_COMMON_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Force destroy #ev_async_t.
 * @param[in] handle    A initialized #ev_async_t handler.
 */
EV_LOCAL void ev__async_exit_force(ev_async_t* handle);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/handle.h"
#ifndef __EV_HANDLE_INTERNAL_H__
#define __EV_HANDLE_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ev_handle_flag
{
    /* Used by all handles. Bit 0-7. */
    EV_HANDLE_CLOSING           = 0x01 << 0x00,     /**< 1. Handle is going to close */
    EV_HANDLE_CLOSED            = 0x01 << 0x01,     /**< 2. Handle is closed */
    EV_HANDLE_ACTIVE            = 0x01 << 0x02,     /**< 4. Handle is busy */

    /* #EV_ROLE_EV_TCP */
    EV_HANDLE_TCP_LISTING       = 0x01 << 0x08,     /**< 256. This is a listen socket and is listening */
    EV_HANDLE_TCP_ACCEPTING     = 0x01 << 0x09,     /**< 512. This is a socket waiting for accept */
    EV_HANDLE_TCP_STREAMING     = 0x01 << 0x0A,     /**< 1024. This is a socket waiting for read or write */
    EV_HANDLE_TCP_CONNECTING    = 0x01 << 0x0B,     /**< 2048. This is a connect and waiting for connect complete */
    EV_HABDLE_TCP_BOUND         = 0x01 << 0x0C,     /**< 4096. Socket is bond to address */

    /* #EV_ROLE_EV_UDP */
    EV_HANDLE_UDP_IPV6          = 0x01 << 0x08,     /**< 256. This socket have IPv6 ability */
    EV_HANDLE_UDP_CONNECTED     = 0x01 << 0x09,     /**< 512. This socket is connected */
    EV_HANDLE_UDP_BOUND         = 0x01 << 0x0A,     /**< 1024. Socket is bond to address */
    EV_HANDLE_UDP_BYPASS_IOCP   = 0x01 << 0x0B,     /**< 2048. FILE_SKIP_SET_EVENT_ON_HANDLE | FILE_SKIP_COMPLETION_PORT_ON_SUCCESS */

    /* #EV_ROLE_EV_PIPE */
    EV_HANDLE_PIPE_IPC          = 0x01 << 0x08,     /**< 256. This pipe is support IPC */
    EV_HANDLE_PIPE_STREAMING    = 0x01 << 0x09,     /**< 512. This pipe is initialized by #ev_stream_t */
} ev_handle_flag_t;

/**
 * @brief Initialize a handle.
 *
 * A initialized handle will be linked with \p loop. By default the \p handle
 * is in #ev_loop_t::handles::idle_list. If the \p handle is active (The event
 * counter is non-zero), the handle is moved into #ev_loop_t::handles::active_list.
 *
 * @note Once a handle is initialized, it must call #ev__handle_exit() when no
 *   longer needed.
 * @param[in] loop      The loop own the handle
 * @param[out] handle   A pointer to the structure
 * @param[in] role      Who we are
 */
EV_LOCAL void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role);

/**
 * @brief Close the handle
 * @note The handle will not closed until close_cb was called, which was given
 *   by #ev__handle_init()
 * @note #ev__handle_exit() never reset active_events counter for you. You always
 *   need to balance active_events counter yourself.
 * @param[in] handle    handler
 * @param[in] close_cb  Close callback. If non-null, the \p close_cb will be
 *   called in next event loop. If null, the handle will be closed synchronously.
 */
EV_LOCAL void ev__handle_exit(ev_handle_t* handle, ev_handle_cb close_cb);

/**
 * @brief Add active event counter. If active event counter is non-zero,
 *   #EV_HANDLE_ACTIVE is appended.
 * @param[in] handle    Handler.
 */
EV_LOCAL void ev__handle_event_add(ev_handle_t* handle);

/**
 * @brief Decrease active event counter. If active event counter is zero,
 *   #EV_HANDLE_ACTIVE is removed.
 * @param[in] handle    Handler.
 */
EV_LOCAL void ev__handle_event_dec(ev_handle_t* handle);

/**
 * @brief Check if the handle is in active state
 * @param[in] handle    handler
 * @return              bool
 */
EV_LOCAL int ev__handle_is_active(ev_handle_t* handle);

/**
 * @brief Check if the handle is in closing or closed state
 * @param[in] handle    handler
 * @return              bool
 */
EV_LOCAL int ev__handle_is_closing(ev_handle_t* handle);

/**
 * @brief Queue a task.
 * This task will be execute in next loop.
 * @param[in] handle    handler.
 * @param[in] callback  Task callback
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__backlog_submit(ev_handle_t* handle, ev_handle_cb callback);

/**
 * @brief Process backlog events.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__process_backlog(ev_loop_t* loop);

/**
 * @brief Process endgame events.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__process_endgame(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/loop.h"
#ifndef __EV_LOOP_INTERNAL_H__
#define __EV_LOOP_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

typedef enum ev_ipc_frame_flag
{
    EV_IPC_FRAME_FLAG_INFORMATION = 1,
}ev_ipc_frame_flag_t;

/**
 * @brief Get event loop for the handle.
 * @param[in] handle    handler
 * @return              Event loop
 */
EV_LOCAL ev_loop_t* ev__handle_loop(ev_handle_t* handle);

/**
 * @brief Check IPC frame header
 * @param[in] buffer    Buffer to check
 * @param[in] size      Buffer size
 * @return              bool
 */
EV_LOCAL int ev__ipc_check_frame_hdr(const void* buffer, size_t size);

/**
 * @brief Initialize IPC frame header
 * @param[out] hdr      Frame header to initialize
 * @param[in] flags     Control flags
 * @param[in] exsz      Extra information size
 * @param[in] dtsz      Data size
 */
EV_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t* hdr,
    uint8_t flags, uint16_t exsz, uint32_t dtsz);

/**
 * @brief Update loop time
 * @param[in] loop  loop handler
 */
EV_LOCAL void ev__loop_update_time(ev_loop_t* loop);

/**
 * @brief Get minimal length of specific \p addr type.
 * @param[in] addr  A valid sockaddr buffer
 * @return          A valid minimal length, or (socklen_t)-1 if error.
 */
EV_LOCAL socklen_t ev__get_addr_len(const struct sockaddr* addr);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup write request
 * @param[in] req   Write request
 */
EV_LOCAL void ev__write_exit(ev_write_t* req);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup read request
 * @param[in] req   read request
 */
EV_LOCAL void ev__read_exit(ev_read_t* req);

/**
 * @brief Initialize backend
 * @param[in] loop      loop handler
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__loop_init_backend(ev_loop_t* loop);

/**
 * @brief Destroy backend
 * @param[in] loop  loop handler
 */
EV_LOCAL void ev__loop_exit_backend(ev_loop_t* loop);

/**
 * @brief Wait for IO event and process
 * @param[in] loop  loop handler
 * @param[in] timeout   timeout in milliseconds
 */
EV_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/fs.h"
#ifndef __EV_FILE_INTERNAL_H__
#define __EV_FILE_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

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
EV_LOCAL int ev__fs_seek(ev_os_file_t file, int whence, ssize_t offset);

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
    ssize_t offset);

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

#line 1 "src/misc.h"
#ifndef __EV_MISC_INTERNAL_H__
#define __EV_MISC_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr    System error
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__translate_sys_error(int syserr);

EV_LOCAL int ev__translate_posix_sys_error(int syserr);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/pipe.h"
#ifndef __EV_PIPE_COMMON_INTERNAL_H__
#define __EV_PIPE_COMMON_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize #ev_pipe_read_req_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Read complete callback
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__pipe_read_init(ev_pipe_read_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_read_cb cb);

/**
 * @brief Initialize #ev_pipe_write_req_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Write complete callback
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__pipe_write_init(ev_pipe_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_write_cb cb);

/**
 * @brief Initialize #ev_pipe_write_req_t
 *
 * The extend version of #ev__pipe_write_init(). You should use this function if
 * any of following condition is meet:
 *
 *   + The value of \p nbuf is larger than #EV_IOV_MAX.<br>
 *     In this case you should pass \p iov_bufs as storage, the minimum value of
 *     \p iov_size can be calculated by #EV_IOV_BUF_SIZE(). \p take the ownership
 *     of \p iov_bufs, so you cannot modify or free \p iov_bufs until \p callback
 *     is called.
 *
 *   + Need to transfer a handle to peer.<br>
 *     In this case you should set the type of handle via \p handle_role and pass
 *     the address of the handle via \p handle_addr. \p req does not take the ownership
 *     of the handle, but the handle should not be closed or destroy until \p callback
 *     is called.
 *
 * @param[out] req          A write request to be initialized
 * @param[in] callback      Write complete callback
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer list size
 * @param[in] handle_role   The type of handle to send
 * @param[in] handle_addr   The address of handle to send
 * @param[in] handle_size   The size of handle to send
 * @return                  #ev_errno_t
 */
EV_LOCAL int ev__pipe_write_init_ext(ev_pipe_write_req_t* req, ev_pipe_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/ringbuffer.h"
#ifndef __EV_RINGBUFFER_INTERNAL_H__
#define __EV_RINGBUFFER_INTERNAL_H__

/* AMALGAMATE: #include "loop.h" */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ring buffer flags
 * @see ring_buffer_reserve
 * @see ring_buffer_commit
 */
typedef enum ring_buffer_flag
{
    RINGBUFFER_FLAG_OVERWRITE   = 0x01 << 0x00,         /**< Overwrite data if no enough free space */
    RINGBUFFER_FLAG_DISCARD     = 0x01 << 0x01,         /**< Discard operation */
    RINGBUFFER_FLAG_ABANDON     = 0x01 << 0x02,         /**< Abandon token */
}ring_buffer_flag_t;

typedef enum ring_buffer_node_state
{
    RINGBUFFER_STAT_WRITING,                            /** Writing */
    RINGBUFFER_STAT_COMMITTED,                          /** Committed */
    RINGBUFFER_STAT_READING,                            /** Reading */
}ring_buffer_node_state_t;

/**
 * @brief Ring buffer data token
 */
typedef struct ring_buffer_token
{
    union
    {
        size_t                      size;               /**< Data length */
        void*                       _align;             /**< Padding field used for make sure address align */
    }size;

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4200)
#endif
    uint8_t                         data[];             /**< Data body */
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif
}ring_buffer_token_t;

typedef struct ring_buffer_node
{
    struct
    {
        size_t                      off_next;           /**< Next position in chain. NOT `NULL` in any condition. */
        size_t                      off_prev;           /**< Previous position in chain. NOT `NULL` in any condition. */
    }chain_pos;

    struct
    {
        size_t                      off_newer;          /**< Newer node. `0` if not exists. */
        size_t                      off_older;          /**< Older node. `NULL` if not exists. */
    }chain_time;

    ring_buffer_node_state_t        state;              /**< Node status */
    ring_buffer_token_t             token;              /**< User data */
}ring_buffer_node_t;

/**
 * @brief Ring buffer counter
 */
typedef struct ring_buffer_counter
{
    size_t                          committed;          /**< The number of committed nodes */
    size_t                          writing;            /**< The number of writing nodes */
    size_t                          reading;            /**< The number of reading nodes */
}ring_buffer_counter_t;

/**
 * @brief Ring buffer
 */
typedef struct ring_buffer
{
    struct
    {
        size_t                      capacity;           /**< Available length. */
    }config;

    struct
    {
        size_t                      off_HEAD;           /**< The newest node which is in READING/WRITING/COMMITTED status. */
        size_t                      off_TAIL;           /**< The oldest node which is in READING/WRITING/COMMITTED status. */
        size_t                      off_reserve;        /**< The oldest node which is in WRITING/COMMITTED status. */
    }node;

    ring_buffer_counter_t           counter;            /** Counter */

    /**
     * @brief Ring buffer base.
     * Data store start at the address of basis.
     * However first `sizeof(void*)` bytes is always zero, by which design all
     * the node position will not be 0, and access to 0 offset data will cause crash.
     */
    void*                           basis;
}ring_buffer_t;

/**
 * @brief Initialize ring buffer on the give memory.
 * @param[in,out] buffer    Memory area
 * @param[in] size          The size of memory area
 * @return                  Initialized ring buffer, the address is equal it \p buffer
 */
EV_LOCAL ring_buffer_t* ring_buffer_init(void* buffer, size_t size);

/**
 * @brief Acquire a token for write.
 * @see ring_buffer_commit
 * @param[in,out] handler   The pointer to the ring buffer
 * @param[in] size          The size of area you want
 * @param[in] flags         #ring_buffer_flag_t
 * @return                  A token for write. This token must be committed by #ring_buffer_commit
 */
EV_LOCAL ring_buffer_token_t* ring_buffer_reserve(ring_buffer_t* handler, size_t size, int flags);

/**
 * @brief Acquire a token for read.
 * @see ring_buffer_commit
 * @param[in,out] handler   The pointer to the ring buffer
 * @return                  A token for read. This token must be committed by #ring_buffer_commit
 */
EV_LOCAL ring_buffer_token_t* ring_buffer_consume(ring_buffer_t* handler);

/**
 * @brief Commit a token.
 *
 * If the token was created by #ring_buffer_reserve, then this token can be
 * consumed by #ring_buffer_consume. If flag contains
 * #RINGBUFFER_FLAG_DISCARD, then this token will be
 * destroyed.
 *
 * If the token was created by #ring_buffer_consume, then this token will be
 * destroyed. If flag contains
 * #RINGBUFFER_FLAG_DISCARD, then this token will be
 * marked as readable, and #ring_buffer_consume will be able to get this
 * token.
 *
 * If there are two or more consumers, discard a reading token may be failed.
 * Consider the following condition:
 * 1. `CONSUMER_A` acquire a reading token `READ_A` (success)
 * 2. `CONSUMER_B` acquire a reading token `READ_B` (success)
 * 3. `CONSUMER_A` discard `READ_A` (failure)
 * > This happens because ring buffer must guarantee data order is FIFO. If
 * > `CONSUMER_A` is able to discard `READ_A`, then next consumer will get
 * > `READ_A` which is older than `READ_B`. This condition must not happen.
 *
 * @pre In the precondition, parameter `token' must be returned from either
 *   #ring_buffer_consume or #ring_buffer_consume.
 * @post In the postcondition, parameter `token' will be invalid to user if
 *   return code is zero.
 * @param[in,out] handler   The pointer to the ring buffer
 * @param[in,out] token     The token going to be committed
 * @param[in] flags         #ring_buffer_flag_t
 * @return                  0 if success, otherwise failure
 */
EV_LOCAL int ring_buffer_commit(ring_buffer_t* handler, ring_buffer_token_t* token, int flags);

/**
 * @brief Get counter
 * @param[in] handler   The ring buffer
 * @param[out] counter  The pointer to counter
 * @return              The sum of all counter
 */
EV_LOCAL size_t ring_buffer_count(ring_buffer_t* handler, ring_buffer_counter_t* counter);

/**
 * @brief Get how much bytes the ring buffer structure cost
 * @return          size of ring_buffer_t
 */
EV_LOCAL size_t ring_buffer_heap_cost(void);

/**
 * @brief Calculate the how much space of given size data will take place.
 * @param[in] size      The size of data
 * @return              The space of data will take place
 */
EV_LOCAL size_t ring_buffer_node_cost(size_t size);

/**
 * @brief Get the begin node of ring buffer.
 * @param[in] handler   The ring buffer
 * @return              The iterator
 */
EV_LOCAL ring_buffer_token_t* ring_buffer_begin(const ring_buffer_t* handler);

/**
 * @brief Get next token.
 * @param[in] handler   The ring buffer
 * @param[in] token     The ring buffer token
 * @return              Next token
 */
EV_LOCAL ring_buffer_token_t* ring_buffer_next(const ring_buffer_t* handler,
    const ring_buffer_token_t* token);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/threadpool.h"
#ifndef __EV_THREADPOOL_INTERNAL_H__
#define __EV_THREADPOOL_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Work type.
 */
typedef enum ev_work_type
{
    /**
     * @brief CPU work
     */
    EV_THREADPOOL_WORK_CPU      = 0,

    /**
     * @brief Fast IO. Typically file system operations.
     */
    EV_THREADPOOL_WORK_IO_FAST  = 1,

    /**
     * @brief Slow IO. Typically network operations.
     */
    EV_THREADPOOL_WORK_IO_SLOW  = 2,
} ev_work_type_t;

typedef struct ev_threadpool ev_threadpool_t;

/**
 * @brief Thread pool handle type.
 */
struct ev_threadpool
{
    ev_os_thread_t*                 threads;        /**< Threads */
    size_t                          thrnum;         /**< The number of threads */

    ev_list_t                       loop_table;     /**< Loop table */

    ev_mutex_t                      mutex;          /**< Thread pool mutex */
    ev_sem_t                        p2w_sem;        /**< Semaphore for pool to worker */
    int                             looping;        /**< Looping flag */

    ev_queue_node_t                 work_queue[3];  /**< Work queue. Index is #ev_work_type_t */
};

#define EV_THREADPOOL_INVALID   \
    {\
        NULL,\
        0,\
        EV_LIST_INIT,\
        EV_MUTEX_INVALID,\
        EV_SEM_INVALID,\
        0,\
        {\
            EV_QUEUE_NODE_INVALID,\
            EV_QUEUE_NODE_INVALID,\
            EV_QUEUE_NODE_INVALID,\
        },\
    }

EV_LOCAL void ev__loop_link_to_default_threadpool(ev_loop_t* loop);

EV_LOCAL int ev_loop_unlink_threadpool(ev_loop_t* loop);

/**
 * @brief Initialize thread pool
 * @param[out] pool     Thread pool
 * @param[in] opt       Thread option
 * @param[in] storage   Storage to save thread
 * @param[in] num       Storage size
 * @return              #ev_errno_t
 */
EV_LOCAL int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num);

/**
 * @brief Exit thread pool
 * @param[in] pool      Thread pool
 */
EV_LOCAL void ev_threadpool_exit(ev_threadpool_t* pool);

/**
 * @brief Link loop with thread pool.
 *
 * Some actions require a linked thread pool.
 *
 * @param[in] loop      The event loop.
 * @param[in] pool      The Thread pool.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev_loop_link_threadpool(ev_loop_t* loop, ev_threadpool_t* pool);

/**
 * @brief Submit task into thread pool
 * @warning This function is NOT MT-Safe and must be called in the thread where
 *   \p loop is running.
 * @param[in] pool      Thread pool
 * @param[in] loop      Which event loop to call \p done_cb
 * @param[in] token     Work token
 * @param[in] type      Work type
 * @param[in] work_cb   Work callback
 * @param[in] done_cb   Work done callback
 * @return              #ev_errno_t
 */
EV_LOCAL int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_work_t* token, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Submit task to threadpool.
 * @param[in] loop      Event loop.
 * @param[in] work      Work token.
 * @param[in] type      Work type.
 * @param[in] work_cb   Work callback.
 * @param[in] done_cb   Work done callback.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__loop_submit_threadpool(ev_loop_t* loop,
    ev_work_t* work, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Process thread pool events.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__threadpool_process(ev_loop_t* loop);

/**
 * @brief Wakeup event loop.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/timer.h"
#ifndef __EV_TIMER_INTERNAL_H__
#define __EV_TIMER_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize timer context.
 * @param[out] loop Event loop
 */
EV_LOCAL void ev__init_timer(ev_loop_t* loop);

/**
 * @brief Process timer.
 * @param[in] loop  Event loop
 */
EV_LOCAL void ev__process_timer(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/log.h"
#ifndef __EV_LOG_INTERNAL_H__
#define __EV_LOG_INTERNAL_H__

/* AMALGAMATE: #include "loop.h" */

#ifdef __cplusplus
extern "C" {
#endif

#define EV_LOG_TRACE(fmt, ...)  \
    ev__log(EV_LOG_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_INFO(fmt, ...)   \
    ev__log(EV_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_ERROR(fmt, ...)  \
    ev__log(EV_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_FATAL(fmt, ...)  \
    ev__log(EV_LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum ev_log_level
{
    EV_LOG_TRACE,
    EV_LOG_INFO,
    EV_LOG_ERROR,
    EV_LOG_FATAL,
}ev_log_level_t;

/**
 * @brief Log
 * @param[in] level Log level
 * @param[in] file  File name
 * @param[in] func  Functon name
 * @param[in] line  Line number
 * @param[in] fmt   Log format
 * @param[in] ...   Argument list
 */
EV_LOCAL void ev__log(ev_log_level_t level, const char* file, const char* func,
    int line, const char* fmt, ...);

/**
 * @brief Dump data as hex
 * @param[in] data  Buffer to dump
 * @param[in] size  Data size
 * @param[in] width Line width
 */
EV_LOCAL void ev__dump_hex(const void* data, size_t size, size_t width);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/udp.h"
#ifndef __EV_UDP_COMMON_INTERNAL_H__
#define __EV_UDP_COMMON_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert \p interface_addr to \p dst.
 * @param[out] dst              A buffer to store address.
 * @param[in] interface_addr    Interface address
 * @param[in] is_ipv6           Whether a IPv6 address. Only valid if \p interface_addr is NULL.
 * @return                      #ev_errno_t
 */
EV_LOCAL int ev__udp_interface_addr_to_sockaddr(struct sockaddr_storage* dst,
        const char* interface_addr, int is_ipv6);

/**
 * @brief Queue a UDP receive request
 * @param[in] udp   UDP handle
 * @param[in] req   Receive request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req);

/**
 * @brief Queue a UDP Send request
 * @param[in] udp   UDP handle
 * @param[in] req   Send request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req,
    const struct sockaddr* addr, socklen_t addrlen);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/allocator.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "allocator.h" */
#include <stdlib.h>
#include <string.h>

typedef struct ev_allocator
{
    ev_malloc_fn    malloc_func;    /**< malloc */
    ev_calloc_fn    calloc_func;    /**< calloc */
    ev_realloc_fn   realloc_func;   /**< realloc */
    ev_free_fn      free_func;      /**< free */
}ev_allocator_t;

static void* _ev_malloc(size_t size)
{
    return malloc(size);
}

static void* _ev_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* _ev_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void _ev_free(void* ptr)
{
    free(ptr);
}

static ev_allocator_t ev__allocator = {
    _ev_malloc,     /* .malloc_func */
    _ev_calloc,     /* .calloc_func */
    _ev_realloc,    /* .realloc_func */
    _ev_free,       /* ._ev_free */
};

int ev_replace_allocator(ev_malloc_fn malloc_func, ev_calloc_fn calloc_func,
    ev_realloc_fn realloc_func, ev_free_fn free_func)
{
    if (malloc_func == NULL || calloc_func == NULL || realloc_func == NULL || free_func == NULL)
    {
        return EV_EINVAL;
    }

    ev__allocator.malloc_func = malloc_func;
    ev__allocator.calloc_func = calloc_func;
    ev__allocator.realloc_func = realloc_func;
    ev__allocator.free_func = free_func;

    return 0;
}

void* ev_calloc(size_t nmemb, size_t size)
{
    return ev__allocator.calloc_func(nmemb, size);
}

void* ev_malloc(size_t size)
{
    return ev__allocator.malloc_func(size);
}

void* ev_realloc(void* ptr, size_t size)
{
    return ev__allocator.realloc_func(ptr, size);
}

void ev_free(void* ptr)
{
    ev__allocator.free_func(ptr);
}

EV_LOCAL char* ev__strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    char* m = ev_malloc(len);
    if (m == NULL)
    {
        return NULL;
    }
    return memcpy(m, s, len);
}

#line 1 "src/fs.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "fs.h" */
/* AMALGAMATE: #include "threadpool.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "misc.h" */
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#   define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

typedef struct ev_dirent_record_s
{
    ev_list_node_t      node;   /**< List node */
    ev_dirent_t         data;   /**< Dirent info */
} ev_dirent_record_t;

typedef struct fs_readdir_helper
{
    ev_fs_req_t*        req;    /**< File system request */
    ssize_t             cnt;    /**< Dirent counter */
} fs_readdir_helper_t;

typedef struct fs_remove_helper
{
    const char*         parent_path;
    int                 ret;
} fs_remove_helper_t;

static void _ev_fs_erase_req(ev_file_t* file, ev_fs_req_t* req)
{
    ev_list_erase(&file->work_queue, &req->node);
}

static void _ev_fs_cleanup_req_as_open(ev_fs_req_t* token)
{
    if (token->req.as_open.path != NULL)
    {
        ev_free(token->req.as_open.path);
        token->req.as_open.path = NULL;
    }
}

static void _ev_fs_cleanup_req_as_seek(ev_fs_req_t* req)
{
    (void)req;
}

static void _ev_fs_cleanup_req_as_read(ev_fs_req_t* req)
{
    ev__read_exit(&req->req.as_read.read_req);
}

static void _ev_fs_cleanup_req_as_write(ev_fs_req_t* req)
{
    ev__write_exit(&req->req.as_write.write_req);
}

static void _ev_fs_cleanup_req_as_fstat(ev_fs_req_t* req)
{
    (void)req;
}

static void _ev_fs_cleanup_req_as_readdir(ev_fs_req_t* req)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&req->rsp.dirents)) != NULL)
    {
        ev_dirent_record_t* rec = EV_CONTAINER_OF(it, ev_dirent_record_t, node);
        ev_free((char*)rec->data.name);
        ev_free(rec);
    }

    if (req->req.as_readdir.path != NULL)
    {
        ev_free(req->req.as_readdir.path);
        req->req.as_readdir.path = NULL;
    }
}

static void _ev_fs_cleanup_req_as_readfile(ev_fs_req_t* req)
{
    if (req->req.as_readfile.path != NULL)
    {
        ev_free(req->req.as_readfile.path);
        req->req.as_readfile.path = NULL;
    }

    if (req->rsp.filecontent.data != NULL)
    {
        ev_free(req->rsp.filecontent.data);
        req->rsp.filecontent.data = NULL;
    }
    req->rsp.filecontent.size = 0;
}

static void _ev_fs_cleanup_req_as_mkdir(ev_fs_req_t* req)
{
    if (req->req.as_mkdir.path != NULL)
    {
        ev_free(req->req.as_mkdir.path);
        req->req.as_mkdir.path = NULL;
    }
}

static void _ev_fs_cleanup_req_as_remove(ev_fs_req_t* req)
{
    if (req->req.as_remove.path != NULL)
    {
        ev_free(req->req.as_remove.path);
        req->req.as_remove.path = NULL;
    }
}

static void _ev_fs_init_req(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb, ev_fs_req_type_t type)
{
    req->req_type = type;
    req->cb = cb;
    req->file = file;
    req->result = EV_EINPROGRESS;

    if (file != NULL)
    {
        ev_list_push_back(&file->work_queue, &req->node);
    }
}

static int _ev_fs_init_req_as_open(ev_fs_req_t* token, ev_file_t* file,
    const char* path, int flags, int mode, ev_file_cb cb)
{
    _ev_fs_init_req(token, file, cb, EV_FS_REQ_OPEN);

    if ((token->req.as_open.path = ev__strdup(path)) == NULL)
    {
        return EV_ENOMEM;
    }

    token->req.as_open.flags = flags;
    token->req.as_open.mode = mode;

    return 0;
}

static int _ev_fs_init_req_as_seek(ev_fs_req_t* token, ev_file_t* file,
    int whence, ssize_t offset, ev_file_cb cb)
{
    _ev_fs_init_req(token, file, cb, EV_FS_REQ_SEEK);

    token->req.as_seek.whence = whence;
    token->req.as_seek.offset = offset;

    return 0;
}

static int _ev_fs_init_req_as_read(ev_fs_req_t* req, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_fs_init_req(req, file, cb, EV_FS_REQ_READ);

    int ret = ev__read_init(&req->req.as_read.read_req, bufs, nbuf);
    if (ret != 0)
    {
        return ret;
    }
    req->req.as_read.offset = offset;

    return 0;
}

static int _ev_fs_init_req_as_write(ev_fs_req_t* token, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_fs_init_req(token, file, cb, EV_FS_REQ_WRITE);

    int ret = ev__write_init(&token->req.as_write.write_req, bufs, nbuf);
    if (ret != 0)
    {
        return ret;
    }
    token->req.as_write.offset = offset;

    return 0;
}

static void _ev_fs_init_req_as_fstat(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb)
{
    _ev_fs_init_req(req, file, cb, EV_FS_REQ_FSTAT);
}

static int _ev_fs_init_req_as_readdir(ev_fs_req_t* req, const char* path, ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_READDIR);
    req->req.as_readdir.path = ev__strdup(path);
    if (req->req.as_readdir.path == NULL)
    {
        return EV_ENOMEM;
    }

    ev_list_init(&req->rsp.dirents);

    return 0;
}

static int _ev_fs_init_req_as_readfile(ev_fs_req_t* req, const char* path,
    ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_READFILE);

    req->req.as_readfile.path = ev__strdup(path);
    if (req->req.as_readfile.path == NULL)
    {
        return EV_ENOMEM;
    }

    req->rsp.filecontent = ev_buf_make(NULL, 0);

    return 0;
}

static int _ev_fs_init_req_as_mkdir(ev_fs_req_t* req, const char* path, int mode,
    ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_MKDIR);

    req->req.as_mkdir.path = ev__strdup(path);
    if (req->req.as_mkdir.path == NULL)
    {
        return EV_ENOMEM;
    }
    req->req.as_mkdir.mode = mode;

    return 0;
}

static int _ev_fs_init_req_as_remove(ev_fs_req_t* req, const char* path, int recursion, ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_REMOVE);

    req->req.as_remove.path = ev__strdup(path);
    if (req->req.as_remove.path == NULL)
    {
        return EV_ENOMEM;
    }
    req->req.as_remove.recursion = recursion;

    return 0;
}

/**
 * @brief Check if \p file have pending task.
 * @return  bool
 */
static int _ev_file_have_pending(ev_file_t* file)
{
    return ev_list_size(&file->work_queue) != 0;
}

static void _ev_file_do_close_callback(ev_file_t* file)
{
    if (file->close_cb != NULL)
    {
        file->close_cb(file);
    }
}

static void _ev_file_do_close_callback_if_necessary(ev_file_t* file)
{
    if (file == NULL)
    {
        return;
    }
    if (file->close_cb != NULL && !_ev_file_have_pending(file))
    {
        _ev_file_do_close_callback(file);
    }
}

static void _ev_file_on_close(ev_handle_t* handle)
{
    ev_file_t* file = EV_CONTAINER_OF(handle, ev_file_t, base);
    _ev_file_do_close_callback(file);
}

static void _ev_file_cancel_all_pending_task(ev_file_t* file, size_t* failure_count)
{
    *failure_count = 0;

    ev_list_node_t* it = ev_list_begin(&file->work_queue);
    for (; it != NULL; it = ev_list_next(it))
    {
        ev_fs_req_t* req = EV_CONTAINER_OF(it, ev_fs_req_t, node);
        if (ev_loop_cancel(&req->work_token) != 0)
        {
            *failure_count += 1;
        }
    }
}

static int _ev_fs_on_readdir_entry(ev_dirent_t* info, void* arg)
{
    fs_readdir_helper_t* helper = arg;

    ev_dirent_record_t* rec = ev_malloc(sizeof(ev_dirent_record_t));
    if (rec == NULL)
    {
        goto err_nomem;
    }

    rec->data.type = info->type;
    rec->data.name = ev__strdup(info->name);
    if (rec->data.name == NULL)
    {
        goto err_nomem;
    }

    ev_list_push_back(&helper->req->rsp.dirents, &rec->node);
    helper->cnt++;

    return 0;

err_nomem:
    if (rec != NULL)
    {
        ev_free(rec);
    }
    helper->req->result = EV_ENOMEM;
    return -1;
}

static void _ev_fs_on_done(ev_work_t* work, int status)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == 0);
        req->result = EV_ECANCELED;
    }

    if (file != NULL)
    {
        ev__handle_event_dec(&file->base);
        _ev_fs_erase_req(file, req);
    }
    req->cb(req);

    /**
     * As describe in #ev_file_exit(), we have to check close status.
     */
    _ev_file_do_close_callback_if_necessary(file);
}

static void _ev_file_on_open(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev_file_open_sync(file, req->req.as_open.path,
        req->req.as_open.flags, req->req.as_open.mode);
}

static void _ev_file_on_read(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->req.as_read.read_req;

    req->result = ev_file_read_sync(file, read_req->data.bufs,
        read_req->data.nbuf);
}

static void _ev_file_on_pread(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->req.as_read.read_req;

    req->result = ev_file_pread_sync(file, read_req->data.bufs,
        read_req->data.nbuf, req->req.as_read.offset);
}

static void _ev_file_on_write(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->req.as_write.write_req;

    req->result = ev_file_write_sync(file, write_req->bufs,
        write_req->nbuf);
}

static void _ev_file_on_pwrite(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->req.as_write.write_req;

    req->result = ev_file_pwrite_sync(file, write_req->bufs,
        write_req->nbuf, req->req.as_write.offset);
}

static void _ev_file_on_fstat(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev_file_stat_sync(file, &req->rsp.fileinfo);
}

static void _ev_fs_on_readdir(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);

    fs_readdir_helper_t helper = { req, 0 };
    req->result = ev__fs_readdir(req->req.as_readdir.path,
        _ev_fs_on_readdir_entry, &helper);

    /* If operation success, replace result with dirent counter */
    if (req->result == 0)
    {
        req->result = helper.cnt;
    }
}

static void _ev_fs_on_readfile(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    const char* path = req->req.as_readfile.path;

    ev_os_file_t file = EV_OS_FILE_INVALID;
    req->result = ev__fs_open(&file, path, EV_FS_O_RDONLY, 0);
    if (req->result != 0)
    {
        return;
    }

    ev_fs_stat_t statbuf = EV_FS_STAT_INVALID;
    req->result = ev__fs_fstat(file, &statbuf);
    if (req->result != 0)
    {
        goto close_file;
    }

    void* data = ev_malloc(statbuf.st_size);
    req->rsp.filecontent = ev_buf_make(data, statbuf.st_size);

    if (req->rsp.filecontent.data == NULL)
    {
        req->result = EV_ENOMEM;
        goto close_file;
    }

    req->result = ev__fs_preadv(file, &req->rsp.filecontent, 1, 0);

close_file:
    ev__fs_close(file);
}

static void _ev_fs_on_mkdir(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    const char* path = req->req.as_mkdir.path;
    int mode = req->req.as_mkdir.mode;

    req->result = ev_fs_mkdir_sync(path, mode);
}

static void _ev_fs_on_remove(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    const char* path = req->req.as_remove.path;

    req->result = ev_fs_remove_sync(path, req->req.as_remove.recursion);
}

static int _ev_file_read_template(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb, ev_work_cb work_cb)
{
    ev_loop_t* loop = file->base.loop;

    int ret = _ev_fs_init_req_as_read(req, file, bufs, nbuf, offset, cb);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&file->base);

    ret = ev__loop_submit_threadpool(loop, &req->work_token,
        EV_THREADPOOL_WORK_IO_FAST, work_cb, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_read(req);
        ev__handle_event_dec(&file->base);
        return ret;
    }

    return 0;
}

static int _ev_file_pwrite_template(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb, ev_work_cb work_cb)
{
    int ret;
    ev_loop_t* loop = file->base.loop;

    ret = _ev_fs_init_req_as_write(req, file, bufs, nbuf, offset, cb);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&file->base);

    ret = ev__loop_submit_threadpool(loop, &req->work_token,
        EV_THREADPOOL_WORK_IO_FAST, work_cb, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_write(req);
        ev__handle_event_dec(&file->base);
        return ret;
    }

    return 0;
}

static void _ev_fs_on_seek(ev_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev__fs_seek(file->file, req->req.as_seek.whence, req->req.as_seek.offset);
}

static int _ev_fs_remove(const char* path)
{
    int errcode;
    if (remove(path) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

static  int _ev_fs_remove_helper(ev_dirent_t* info, void* arg)
{
    fs_remove_helper_t* helper = arg;
    const char* parent_path = helper->parent_path;
    size_t parent_path_sz = strlen(parent_path);
    size_t name_sz = strlen(info->name);
    size_t full_path_sz = parent_path_sz + 1 + name_sz;

    char* full_path = ev_malloc(full_path_sz + 1);
    snprintf(full_path, full_path_sz + 1, "%s/%s", parent_path, info->name);

    helper->ret = ev__fs_remove(full_path, 1);
    ev_free(full_path);

    return helper->ret;
}

int ev_file_init(ev_loop_t* loop, ev_file_t* file)
{
    file->file = EV_OS_FILE_INVALID;
    file->close_cb = NULL;
    ev__handle_init(loop, &file->base, EV_ROLE_EV_FILE);
    ev_list_init(&file->work_queue);

    return 0;
}

void ev_file_exit(ev_file_t* file, ev_file_close_cb cb)
{
    size_t failure_count;

    /**
     * Cancel all pending task. Do note that some tasks might be cancel failed
     * as they are currently executing.
     */
    _ev_file_cancel_all_pending_task(file, &failure_count);

    /**
     * It should be safe to close handle, event there are some works in
     * progress.
     */
    if (file->file != EV_OS_FILE_INVALID)
    {
        ev__fs_close(file->file);
        file->file = EV_OS_FILE_INVALID;
    }

    file->close_cb = cb;

    /**
     * If all pending task was cancel, it is safe to close handle, because they
     * are in the backlog queue.
     */
    if (failure_count == 0)
    {
        ev__handle_exit(&file->base, _ev_file_on_close);
        return;
    }

    /**
     * Now we have some problem, because some task is executing.
     *
     * We cannot close handle directly as these tasks will surely callback after
     * this handle closed.
     *
     * So we have to check close status in file operations callback. Once no
     * pending tasks, we can close this handle.
     */
}

int ev_file_open(ev_file_t* file, ev_fs_req_t* token, const char* path,
    int flags, int mode, ev_file_cb cb)
{
    ev_loop_t* loop = file->base.loop;

    int ret = _ev_fs_init_req_as_open(token, file, path, flags, mode, cb);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&file->base);

    ret = ev__loop_submit_threadpool(loop, &token->work_token,
        EV_THREADPOOL_WORK_IO_FAST, _ev_file_on_open, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_open(token);
        ev__handle_event_dec(&file->base);
        return ret;
    }

    return 0;
}

int ev_file_open_sync(ev_file_t* file, const char* path, int flags, int mode)
{
    return ev__fs_open(&file->file, path, flags, mode);
}

int ev_file_seek(ev_file_t* file, ev_fs_req_t* req, int whence, ssize_t offset, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.loop;
    _ev_fs_init_req_as_seek(req, file, whence, offset, cb);

    ev__handle_event_add(&file->base);

    ret = ev__loop_submit_threadpool(loop, &req->work_token,
        EV_THREADPOOL_WORK_IO_FAST, _ev_fs_on_seek, _ev_fs_on_done);
    if (ret != 0)
    {
        ev__handle_event_dec(&file->base);
        return ret;
    }

    return 0;
}

int ev_file_read(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb)
{
    return _ev_file_read_template(file, req, bufs, nbuf, 0, cb, _ev_file_on_read);
}

ssize_t ev_file_read_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf)
{
    return ev__fs_readv(file->file, bufs, nbuf);
}

int ev_file_pread(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    return _ev_file_read_template(file, req, bufs, nbuf, offset, cb, _ev_file_on_pread);
}

ssize_t ev_file_pread_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf,
    ssize_t offset)
{
    return ev__fs_preadv(file->file, bufs, nbuf, offset);
}

int ev_file_write(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb)
{
    return _ev_file_pwrite_template(file, req, bufs, nbuf, 0, cb, _ev_file_on_write);
}

ssize_t ev_file_write_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf)
{
    return ev__fs_writev(file->file, bufs, nbuf);
}

int ev_file_pwrite(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    return _ev_file_pwrite_template(file, req, bufs, nbuf, offset, cb, _ev_file_on_pwrite);
}

ssize_t ev_file_pwrite_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf,
    ssize_t offset)
{
    return ev__fs_pwritev(file->file, bufs, nbuf, offset);
}

int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.loop;

    _ev_fs_init_req_as_fstat(req, file, cb);
    ev__handle_event_add(&file->base);

    ret = ev__loop_submit_threadpool(loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_fstat, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_fstat(req);
        ev__handle_event_dec(&file->base);
        return ret;
    }

    return 0;
}

int ev_file_stat_sync(ev_file_t* file, ev_fs_stat_t* stat)
{
    return ev__fs_fstat(file->file, stat);
}

int ev_fs_readdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb callback)
{
    int ret;

    ret = _ev_fs_init_req_as_readdir(req, path, callback);
    if (ret != 0)
    {
        return ret;
    }

    ret = ev__loop_submit_threadpool(loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_readdir, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_readdir(req);
        return ret;
    }

    return 0;
}

int ev_fs_readfile(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb)
{
    int ret;

    ret = _ev_fs_init_req_as_readfile(req, path, cb);
    if (ret != 0)
    {
        return ret;
    }

    ret = ev__loop_submit_threadpool(loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_readfile, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_readfile(req);
        return ret;
    }

    return 0;
}

int ev_fs_mkdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path, int mode,
    ev_file_cb cb)
{
    int ret;

    ret = _ev_fs_init_req_as_mkdir(req, path, mode, cb);
    if (ret != 0)
    {
        return ret;
    }

    ret = ev__loop_submit_threadpool(loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_mkdir, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_mkdir(req);
        return ret;
    }

    return 0;
}

int ev_fs_mkdir_sync(const char* path, int mode)
{
    return ev__fs_mkdir(path, mode);
}

int ev_fs_remove(ev_loop_t* loop, ev_fs_req_t* req, const char* path, int recursion, ev_file_cb cb)
{
    int ret;

    ret = _ev_fs_init_req_as_remove(req, path, recursion, cb);
    if (ret != 0)
    {
        return ret;
    }

    ret = ev__loop_submit_threadpool(loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_remove, _ev_fs_on_done);
    if (ret != 0)
    {
        _ev_fs_cleanup_req_as_remove(req);
        return ret;
    }

    return 0;
}

int ev_fs_remove_sync(const char* path, int recursion)
{
    return ev__fs_remove(path, recursion);
}

ev_fs_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req)
{
    return &req->rsp.fileinfo;
}

void ev_fs_req_cleanup(ev_fs_req_t* req)
{
    switch (req->req_type)
    {
    case EV_FS_REQ_OPEN:
        _ev_fs_cleanup_req_as_open(req);
        break;

    case EV_FS_REQ_SEEK:
        _ev_fs_cleanup_req_as_seek(req);
        break;

    case EV_FS_REQ_READ:
        _ev_fs_cleanup_req_as_read(req);
        break;

    case EV_FS_REQ_WRITE:
        _ev_fs_cleanup_req_as_write(req);
        break;

    case EV_FS_REQ_FSTAT:
        _ev_fs_cleanup_req_as_fstat(req);
        break;

    case EV_FS_REQ_READDIR:
        _ev_fs_cleanup_req_as_readdir(req);
        break;

    case EV_FS_REQ_READFILE:
        _ev_fs_cleanup_req_as_readfile(req);
        break;

    case EV_FS_REQ_MKDIR:
        _ev_fs_cleanup_req_as_mkdir(req);
        break;

    default:
        EV_ABORT("req:%d", req->req_type);
    }
}

ev_file_t* ev_fs_get_file(ev_fs_req_t* req)
{
    return req->file;
}

ev_dirent_t* ev_fs_get_first_dirent(ev_fs_req_t* req)
{
    ev_list_node_t* it = ev_list_begin(&req->rsp.dirents);
    if (it == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* rec = EV_CONTAINER_OF(it, ev_dirent_record_t, node);
    return &rec->data;
}

ev_dirent_t* ev_fs_get_next_dirent(ev_dirent_t* curr)
{
    if (curr == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* rec = EV_CONTAINER_OF(curr, ev_dirent_record_t, data);

    ev_list_node_t* next_node = ev_list_next(&rec->node);
    if (next_node == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* next_rec = EV_CONTAINER_OF(next_node, ev_dirent_record_t, node);
    return &next_rec->data;
}

ev_buf_t* ev_fs_get_filecontent(ev_fs_req_t* req)
{
    return &req->rsp.filecontent;
}

EV_LOCAL int ev__fs_remove(const char* path, int recursive)
{
    int ret;

    struct stat buf;
    if (stat(path, &buf) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    if (!S_ISDIR(buf.st_mode) || !recursive)
    {
        goto finish;
    }

    fs_remove_helper_t helper;
    helper.parent_path = path;
    helper.ret = 0;

    if ((ret = ev__fs_readdir(path, _ev_fs_remove_helper, &helper)) != 0)
    {
        return helper.ret;
    }

finish:
    return _ev_fs_remove(path);
}

#line 1 "src/handle.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "handle.h" */
#include <assert.h>

/**
 * @brief Set handle as inactive
 * @see ev__handle_event_dec()
 * @param[in] handle    handler
 */
static void ev__handle_deactive(ev_handle_t* handle)
{
    if (!(handle->data.flags & EV_HANDLE_ACTIVE))
    {
        return;
    }
    handle->data.flags &= ~EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->loop;
    ev_list_erase(&loop->handles.active_list, &handle->handle_queue);
    ev_list_push_back(&loop->handles.idle_list, &handle->handle_queue);
}

/**
 * @brief Force set handle as active, regardless the active event counter.
 * @see ev__handle_event_add()
 * @param[in] handle    handler
 */
static void ev__handle_active(ev_handle_t* handle)
{
    if (handle->data.flags & EV_HANDLE_ACTIVE)
    {
        return;
    }
    handle->data.flags |= EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->loop;
    ev_list_erase(&loop->handles.idle_list, &handle->handle_queue);
    ev_list_push_back(&loop->handles.active_list, &handle->handle_queue);
}

static void _ev_to_close_handle(ev_handle_t* handle)
{
    ev__handle_event_dec(handle);

    /**
     * Deactive but not reset #ev_handle_t::data::active_events, for debug
     * purpose.
     * The #ev_handle_t::data::active_events should be zero by now.
     */
    ev__handle_deactive(handle);

    handle->data.flags |= EV_HANDLE_CLOSED;
    ev_list_erase(&handle->loop->handles.idle_list, &handle->handle_queue);

    handle->endgame.close_cb(handle);
}

EV_LOCAL void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role)
{
    handle->loop = loop;
    ev_list_push_back(&loop->handles.idle_list, &handle->handle_queue);

    handle->data.role = role;
    handle->data.flags = 0;
    handle->data.active_events = 0;

    handle->backlog.status = EV_ENOENT;
    handle->backlog.cb = NULL;
    handle->backlog.node = (ev_list_node_t)EV_LIST_NODE_INIT;

    handle->endgame.close_cb = NULL;
    handle->endgame.node = (ev_list_node_t)EV_LIST_NODE_INIT;
}

EV_LOCAL void ev__handle_exit(ev_handle_t* handle, ev_handle_cb close_cb)
{
    assert(!ev__handle_is_closing(handle));

    handle->data.flags |= EV_HANDLE_CLOSING;
    handle->endgame.close_cb = close_cb;

    if (close_cb != NULL)
    {
        ev__handle_event_add(handle);
        ev_list_push_back(&handle->loop->endgame_queue, &handle->endgame.node);
    }
    else
    {
        ev__handle_deactive(handle);
        handle->data.flags |= EV_HANDLE_CLOSED;
        ev_list_erase(&handle->loop->handles.idle_list, &handle->handle_queue);
    }
}

EV_LOCAL void ev__handle_event_add(ev_handle_t* handle)
{
    handle->data.active_events++;

    if (handle->data.active_events != 0)
    {
        ev__handle_active(handle);
    }
}

EV_LOCAL void ev__handle_event_dec(ev_handle_t* handle)
{
    assert(handle->data.active_events != 0);

    handle->data.active_events--;

    if (handle->data.active_events == 0)
    {
        ev__handle_deactive(handle);
    }
}

EV_LOCAL int ev__handle_is_active(ev_handle_t* handle)
{
    return handle->data.flags & EV_HANDLE_ACTIVE;
}

EV_LOCAL int ev__handle_is_closing(ev_handle_t* handle)
{
    return handle->data.flags & (EV_HANDLE_CLOSING | EV_HANDLE_CLOSED);
}

EV_LOCAL int ev__backlog_submit(ev_handle_t* handle, ev_handle_cb callback)
{
    if (handle->backlog.status != EV_ENOENT)
    {
        return EV_EEXIST;
    }

    handle->backlog.status = EV_EEXIST;
    handle->backlog.cb = callback;
    ev__handle_event_add(handle);

    ev_list_push_back(&handle->loop->backlog_queue, &handle->backlog.node);

    return 0;
}

EV_LOCAL void ev__process_backlog(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->backlog_queue)) != NULL)
    {
        ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, backlog.node);

        ev__handle_event_dec(handle);
        handle->backlog.status = EV_ENOENT;

        handle->backlog.cb(handle);
    }
}

EV_LOCAL void ev__process_endgame(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->endgame_queue)) != NULL)
    {
        ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, endgame.node);
        _ev_to_close_handle(handle);
    }
}

#line 1 "src/list.c"
#include <string.h>
/* AMALGAMATE: #include "ev.h" */

static void _list_lite_set_once(ev_list_t* handler, ev_list_node_t* node)
{
    handler->head = node;
    handler->tail = node;
    node->p_after = NULL;
    node->p_before = NULL;
    handler->size = 1;
}

void ev_list_init(ev_list_t* handler)
{
    memset(handler, 0, sizeof(*handler));
}

void ev_list_push_back(ev_list_t* handler, ev_list_node_t* node)
{
    if (handler->head == NULL)
    {
        _list_lite_set_once(handler, node);
        return;
    }

    node->p_after = NULL;
    node->p_before = handler->tail;
    handler->tail->p_after = node;
    handler->tail = node;
    handler->size++;
}

void ev_list_insert_before(ev_list_t* handler, ev_list_node_t* pos, ev_list_node_t* node)
{
    if (handler->head == pos)
    {
        ev_list_push_front(handler, node);
        return;
    }

    node->p_before = pos->p_before;
    node->p_after = pos;
    pos->p_before->p_after = node;
    pos->p_before = node;
    handler->size++;
}

void ev_list_insert_after(ev_list_t* handler,  ev_list_node_t* pos, ev_list_node_t* node)
{
    if (handler->tail == pos)
    {
        ev_list_push_back(handler, node);
        return;
    }

    node->p_before = pos;
    node->p_after = pos->p_after;
    pos->p_after->p_before = node;
    pos->p_after = node;
    handler->size++;
}

void ev_list_push_front(ev_list_t* handler, ev_list_node_t* node)
{
    if (handler->head == NULL)
    {
        _list_lite_set_once(handler, node);
        return;
    }

    node->p_before = NULL;
    node->p_after = handler->head;
    handler->head->p_before = node;
    handler->head = node;
    handler->size++;
}

ev_list_node_t* ev_list_begin(const ev_list_t* handler)
{
    return handler->head;
}

ev_list_node_t* ev_list_end(const ev_list_t* handler)
{
    return handler->tail;
}

ev_list_node_t* ev_list_next(const ev_list_node_t* node)
{
    return node->p_after;
}

ev_list_node_t* ev_list_prev(const ev_list_node_t* node)
{
    return node->p_before;
}

void ev_list_erase(ev_list_t* handler, ev_list_node_t* node)
{
    handler->size--;

    /* Only one node */
    if (handler->head == node && handler->tail == node)
    {
        handler->head = NULL;
        handler->tail = NULL;
        goto fin;
    }

    if (handler->head == node)
    {
        node->p_after->p_before = NULL;
        handler->head = node->p_after;
        goto fin;
    }

    if (handler->tail == node)
    {
        node->p_before->p_after = NULL;
        handler->tail = node->p_before;
        goto fin;
    }

    node->p_before->p_after = node->p_after;
    node->p_after->p_before = node->p_before;

fin:
    node->p_after = NULL;
    node->p_before = NULL;
}

ev_list_node_t* ev_list_pop_front(ev_list_t* handler)
{
    ev_list_node_t* node = handler->head;
    if (node == NULL)
    {
        return NULL;
    }

    ev_list_erase(handler, node);
    return node;
}

ev_list_node_t* ev_list_pop_back(ev_list_t* handler)
{
    ev_list_node_t* node = handler->tail;
    if (node == NULL)
    {
        return NULL;
    }

    ev_list_erase(handler, node);
    return node;
}

size_t ev_list_size(const ev_list_t* handler)
{
    return handler->size;
}

void ev_list_migrate(ev_list_t* dst, ev_list_t* src)
{
    if (src->head == NULL)
    {
        return;
    }

    if (dst->tail == NULL)
    {
        *dst = *src;
    }
    else
    {
        dst->tail->p_after = src->head;
        dst->tail->p_after->p_before = dst->tail;
        dst->tail = src->tail;
        dst->size += src->size;
    }

    src->head = NULL;
    src->tail = NULL;
    src->size = 0;
}

#line 1 "src/log.c"
/* AMALGAMATE: #include "log.h" */
#include <stdarg.h>
#include <stdio.h>

static const char* _ev_filename(const char* file)
{
    const char* pos = file;

    for (; *file; ++file)
    {
        if (*file == '\\' || *file == '/')
        {
            pos = file + 1;
        }
    }
    return pos;
}

static char _ev_ascii_to_char(unsigned char c)
{
    if (c >= 32 && c <= 126)
    {
        return c;
    }
    return '.';
}

EV_LOCAL void ev__log(ev_log_level_t level, const char* file, const char* func,
    int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    const char* prefix;
    switch (level)
    {
    case EV_LOG_INFO:
        prefix = "I";
        break;
    case EV_LOG_ERROR:
        prefix = "E";
        break;
    case EV_LOG_FATAL:
        prefix = "F";
        break;
    default:
        prefix = "T";
        break;
    }

    printf("[%s %s:%d %s] ", prefix, _ev_filename(file), line, func);
    vprintf(fmt, ap);
    printf("\n");

    va_end(ap);
}

EV_LOCAL void ev__dump_hex(const void* data, size_t size, size_t width)
{
    const unsigned char* pdat = (unsigned char*)data;

    size_t idx_line;
    for (idx_line = 0; idx_line < size; idx_line += width)
    {
        size_t idx_colume;
        /* printf hex */
        for (idx_colume = 0; idx_colume < width; idx_colume++)
        {
            const char* postfix = (idx_colume < width - 1) ? "" : "|";

            if (idx_colume + idx_line < size)
            {
                fprintf(stdout, "%02x %s", pdat[idx_colume + idx_line], postfix);
            }
            else
            {
                fprintf(stdout, "   %s", postfix);
            }
        }
        fprintf(stdout, " ");
        /* printf char */
        for (idx_colume = 0; (idx_colume < width) && (idx_colume + idx_line < size); idx_colume++)
        {
            fprintf(stdout, "%c", _ev_ascii_to_char(pdat[idx_colume + idx_line]));
        }
        fprintf(stdout, "\n");
    }

}

#line 1 "src/loop.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "timer.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "threadpool.h" */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if !defined(_WIN32)
#   include <net/if.h>
#   include <sys/un.h>
#endif

typedef struct ev_strerror_pair
{
    int             errn;           /**< Error number */
    const char*     info;           /**< Error string */
}ev_strerror_pair_t;

static int _ev_loop_init(ev_loop_t* loop)
{
    memset(loop, 0, sizeof(*loop));

    loop->hwtime = 0;
    ev_list_init(&loop->handles.idle_list);
    ev_list_init(&loop->handles.active_list);

    ev_list_init(&loop->backlog_queue);
    ev_list_init(&loop->endgame_queue);

    ev__init_timer(loop);

    loop->threadpool.pool = NULL;
    loop->threadpool.node = (ev_list_node_t)EV_LIST_NODE_INIT;
    ev_mutex_init(&loop->threadpool.mutex, 0);
    ev_list_init(&loop->threadpool.work_queue);

    ev__loop_link_to_default_threadpool(loop);

    return 0;
}

static void _ev_loop_exit(ev_loop_t* loop)
{
    ev_mutex_exit(&loop->threadpool.mutex);
    ev_loop_unlink_threadpool(loop);
}

/**
 * @return bool. non-zero for have active events.
 */
static int _ev_loop_alive(ev_loop_t* loop)
{
    return ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->backlog_queue)
        || ev_list_size(&loop->endgame_queue);
}

static uint32_t _ev_backend_timeout_timer(ev_loop_t* loop)
{
    ev_map_node_t* it = ev_map_begin(&loop->timer.heap);
    if (it == NULL)
    {
        return (uint32_t)-1;
    }

    ev_timer_t* timer = EV_CONTAINER_OF(it, ev_timer_t, node);
    if (timer->data.active <= loop->hwtime)
    {
        return 0;
    }
    uint64_t dif = timer->data.active - loop->hwtime;
    return dif > UINT32_MAX ? UINT32_MAX : (uint32_t)dif;
}

/**
 * @brief Calculate wait timeout
 * 
 * Calculate timeout as small as posibile.
 * 
 * @param[in] loop  Event loop
 * @return          Timeout in milliseconds
 */
static uint32_t _ev_backend_timeout(ev_loop_t* loop)
{
    if (loop->mask.b_stop)
    {
        return 0;
    }

    if (ev_list_size(&loop->backlog_queue) != 0 || ev_list_size(&loop->endgame_queue) != 0)
    {
        return 0;
    }

    return _ev_backend_timeout_timer(loop);
}

static size_t _ev_calculate_read_capacity(const ev_read_t* req)
{
    size_t total = 0;

    size_t i;
    for (i = 0; i < req->data.nbuf; i++)
    {
        total += req->data.bufs[i].size;
    }

    return total;
}

static size_t _ev_calculate_write_size(const ev_write_t* req)
{
    size_t total = 0;

    size_t i;
    for (i = 0; i < req->nbuf; i++)
    {
        total += req->bufs[i].size;
    }
    return total;
}

EV_LOCAL void ev__loop_update_time(ev_loop_t* loop)
{
    loop->hwtime = ev_hrtime() / 1000;
}

EV_LOCAL int ev__ipc_check_frame_hdr(const void* buffer, size_t size)
{
    const ev_ipc_frame_hdr_t* hdr = buffer;
    if (size < sizeof(ev_ipc_frame_hdr_t))
    {
        return 0;
    }

    if (hdr->hdr_magic != EV_IPC_FRAME_HDR_MAGIC)
    {
        return 0;
    }

    return 1;
}

EV_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t* hdr, uint8_t flags, uint16_t exsz, uint32_t dtsz)
{
    hdr->hdr_magic = EV_IPC_FRAME_HDR_MAGIC;
    hdr->hdr_flags = flags;
    hdr->hdr_version = 0;
    hdr->hdr_exsz = exsz;

    hdr->hdr_dtsz = dtsz;
    hdr->reserved = 0;
}

EV_LOCAL ev_loop_t* ev__handle_loop(ev_handle_t* handle)
{
    return handle->loop;
}

int ev_loop_init(ev_loop_t* loop)
{
    int ret;
    if ((ret = _ev_loop_init(loop)) != 0)
    {
        return ret;
    }

    if ((ret = ev__loop_init_backend(loop)) != 0)
    {
        _ev_loop_exit(loop);
        return ret;
    }

    ev__loop_update_time(loop);
    return 0;
}

int ev_loop_exit(ev_loop_t* loop)
{
    if (ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->handles.idle_list))
    {
        return EV_EBUSY;
    }

    ev__loop_exit_backend(loop);
    _ev_loop_exit(loop);

    return 0;
}

void ev_loop_stop(ev_loop_t* loop)
{
    loop->mask.b_stop = 1;
}

int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode)
{
    uint32_t timeout;

    int ret;
    while ((ret = _ev_loop_alive(loop)) != 0 && !loop->mask.b_stop)
    {
        ev__loop_update_time(loop);

        ev__process_timer(loop);
        ev__process_backlog(loop);
        ev__process_endgame(loop);

        if ((ret = _ev_loop_alive(loop)) == 0)
        {
            break;
        }

        /* Calculate timeout */
        timeout = mode != EV_LOOP_MODE_NOWAIT ?
            _ev_backend_timeout(loop) : 0;

        ev__poll(loop, timeout);

        /**
         * #EV_LOOP_MODE_ONCE implies forward progress: at least one callback must have
         * been invoked when it returns. #_ev_poll_win() can return without doing
         * I/O (meaning: no callbacks) when its timeout expires - which means we
         * have pending timers that satisfy the forward progress constraint.
         *
         * #EV_LOOP_MODE_NOWAIT makes no guarantees about progress so it's omitted from
         * the check.
         */
        if (mode == EV_LOOP_MODE_ONCE)
        {
            ev__loop_update_time(loop);
            ev__process_timer(loop);
        }

        /* Callback maybe added */
        ev__process_backlog(loop);
        ev__process_endgame(loop);

        if (mode != EV_LOOP_MODE_DEFAULT)
        {
            break;
        }
    }

    /* Prevent #ev_loop_t::mask::b_stop from compile optimization */
    if (loop->mask.b_stop)
    {
        loop->mask.b_stop = 0;
    }

    return ret;
}

EV_LOCAL int ev__write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf)
{
    req->nbuf = nbuf;

    if (nbuf <= ARRAY_SIZE(req->bufsml))
    {
        req->bufs = req->bufsml;
    }
    else
    {
        req->bufs = ev_malloc(sizeof(ev_buf_t) * nbuf);
        if (req->bufs == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->bufs, bufs, sizeof(ev_buf_t) * nbuf);
    req->size = 0;
    req->capacity = _ev_calculate_write_size(req);
    return 0;
}

EV_LOCAL void ev__write_exit(ev_write_t* req)
{
    if (req->bufs != req->bufsml)
    {
        ev_free(req->bufs);
    }

    req->bufs = NULL;
    req->nbuf = 0;
}

EV_LOCAL int ev__read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf)
{
    req->data.nbuf = nbuf;

    if (nbuf <= ARRAY_SIZE(req->data.bufsml))
    {
        req->data.bufs = req->data.bufsml;
    }
    else
    {
        req->data.bufs = ev_malloc(sizeof(ev_buf_t) * nbuf);
        if (req->data.bufs == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);
    req->data.capacity = _ev_calculate_read_capacity(req);
    req->data.size = 0;
    return 0;
}

EV_LOCAL void ev__read_exit(ev_read_t* req)
{
    if (req->data.bufs != req->data.bufsml)
    {
        ev_free(req->data.bufs);
    }
    req->data.bufs = NULL;
    req->data.nbuf = 0;
}

const char* ev_strerror(int err)
{
#define EV_EXPAND_ERRMAP(err, syserr, str) case err: return str;

    switch (err)
    {
	/* Success */
	case 0:                     return "Operation success";
    case EV_EOF:                return "End of file";
    /* posix */
    EV_ERRNO_POSIX_MAP(EV_EXPAND_ERRMAP);
    /* Unknown error */
    default:                    break;
    }
    return "Unknown error";

#undef EV_EXPAND_ERRMAP
}

EV_LOCAL socklen_t ev__get_addr_len(const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET)
    {
        return sizeof(struct sockaddr_in);
    }
    if (addr->sa_family == AF_INET6)
    {
        return sizeof(struct sockaddr_in6);
    }
#if defined(AF_UNIX) && !defined(_WIN32)
    if (addr->sa_family == AF_UNIX)
    {
        return sizeof(struct sockaddr_un);
    }
#endif

    return (socklen_t)-1;
}

void ev_loop_walk(ev_loop_t* loop, ev_walk_cb cb, void* arg)
{
    ev_list_t* walk_lists[] = {
        &loop->handles.active_list,
        &loop->handles.idle_list,
    };

    size_t i;
    for (i = 0; i < ARRAY_SIZE(walk_lists); i++)
    {
        ev_list_node_t* it = ev_list_begin(walk_lists[i]);
        for (; it != NULL; it = ev_list_next(it))
        {
            ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, handle_queue);
            if (cb(handle, arg) != 0)
            {
                return;
            }
        }
    }
}

#line 1 "src/map.c"
/* AMALGAMATE: #include "ev.h" */
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
/* AMALGAMATE: #include "ev.h" */

/*
* red-black trees properties:  http://en.wikipedia.org/wiki/Rbtree
*
*  1) A node is either red or black
*  2) The root is black
*  3) All leaves (NULL) are black
*  4) Both children of every red node are black
*  5) Every simple path from root to leaves contains the same number
*     of black nodes.
*
*  4 and 5 give the O(log n) guarantee, since 4 implies you cannot have two
*  consecutive red nodes in a path and every red node is therefore followed by
*  a black. So if B is the number of black nodes on every simple path (as per
*  5), then the longest possible path due to 4 is 2B.
*
*  We shall indicate color with case, where black nodes are uppercase and red
*  nodes will be lowercase. Unknown color nodes shall be drawn as red within
*  parentheses and have some accompanying text comment.
*/

#define RB_RED      0
#define RB_BLACK    1

#define __rb_color(pc)     ((uintptr_t)(pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define __rb_parent(pc)    ((ev_map_node_t*)(pc & ~3))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)
#define rb_parent(r)   ((ev_map_node_t*)((uintptr_t)((r)->__rb_parent_color) & ~3))

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  \
    ((node)->__rb_parent_color == (struct ev_map_node*)(node))

static void rb_set_black(ev_map_node_t *rb)
{
    rb->__rb_parent_color =
        (ev_map_node_t*)((uintptr_t)(rb->__rb_parent_color) | RB_BLACK);
}

static ev_map_node_t *rb_red_parent(ev_map_node_t *red)
{
    return (ev_map_node_t*)red->__rb_parent_color;
}

static void rb_set_parent_color(ev_map_node_t *rb, ev_map_node_t *p, int color)
{
    rb->__rb_parent_color = (struct ev_map_node*)((uintptr_t)p | color);
}

static void __rb_change_child(ev_map_node_t* old_node, ev_map_node_t* new_node,
    ev_map_node_t* parent, ev_map_t* root)
{
    if (parent)
    {
        if (parent->rb_left == old_node)
        {
            parent->rb_left = new_node;
        }
        else
        {
            parent->rb_right = new_node;
        }
    }
    else
    {
        root->rb_root = new_node;
    }
}

/*
* Helper function for rotations:
* - old's parent and color get assigned to new
* - old gets assigned new as a parent and 'color' as a color.
*/
static void __rb_rotate_set_parents(ev_map_node_t* old, ev_map_node_t* new_node,
    ev_map_t* root, int color)
{
    ev_map_node_t* parent = rb_parent(old);
    new_node->__rb_parent_color = old->__rb_parent_color;
    rb_set_parent_color(old, new_node, color);
    __rb_change_child(old, new_node, parent, root);
}

static void __rb_insert(ev_map_node_t* node, ev_map_t* root)
{
    ev_map_node_t* parent = rb_red_parent(node), *gparent, *tmp;

    for (;;) {
        /*
        * Loop invariant: node is red
        *
        * If there is a black parent, we are done.
        * Otherwise, take some corrective action as we don't
        * want a red root or two consecutive red nodes.
        */
        if (!parent) {
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        }
        else if (rb_is_black(parent))
            break;

        gparent = rb_red_parent(parent);

        tmp = gparent->rb_right;
        if (parent != tmp) {    /* parent == gparent->rb_left */
            if (tmp && rb_is_red(tmp)) {
                /*
                * Case 1 - color flips
                *
                *       G            g
                *      / \          / \
                *     p   u  -->   P   U
                *    /            /
                *   n            n
                *
                * However, since g's parent might be red, and
                * 4) does not allow this, we need to recurse
                * at g.
                */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_right;
            if (node == tmp) {
                /*
                * Case 2 - left rotate at parent
                *
                *      G             G
                *     / \           / \
                *    p   U  -->    n   U
                *     \           /
                *      n         p
                *
                * This still leaves us in violation of 4), the
                * continuation into Case 3 will fix that.
                */
                parent->rb_right = tmp = node->rb_left;
                node->rb_left = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent,
                    RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                parent = node;
                tmp = node->rb_right;
            }

            /*
            * Case 3 - right rotate at gparent
            *
            *        G           P
            *       / \         / \
            *      p   U  -->  n   g
            *     /                 \
            *    n                   U
            */
            gparent->rb_left = tmp;  /* == parent->rb_right */
            parent->rb_right = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            break;
        }
        else {
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                /* Case 1 - color flips */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_left;
            if (node == tmp) {
                /* Case 2 - right rotate at parent */
                parent->rb_left = tmp = node->rb_right;
                node->rb_right = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent,
                    RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                parent = node;
                tmp = node->rb_left;
            }

            /* Case 3 - left rotate at gparent */
            gparent->rb_right = tmp;  /* == parent->rb_left */
            parent->rb_left = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            break;
        }
    }
}

static void rb_set_parent(ev_map_node_t* rb, ev_map_node_t*p)
{
    rb->__rb_parent_color = (struct ev_map_node*)(rb_color(rb) | (uintptr_t)p);
}

static ev_map_node_t* __rb_erase_augmented(ev_map_node_t* node, ev_map_t* root)
{
    ev_map_node_t *child = node->rb_right, *tmp = node->rb_left;
    ev_map_node_t *parent, *rebalance;
    uintptr_t pc;

    if (!tmp) {
        /*
        * Case 1: node to erase has no more than 1 child (easy!)
        *
        * Note that if there is one child it must be red due to 5)
        * and node must be black due to 4). We adjust colors locally
        * so as to bypass __rb_erase_color() later on.
        */
        pc = (uintptr_t)(node->__rb_parent_color);
        parent = __rb_parent(pc);
        __rb_change_child(node, child, parent, root);
        if (child) {
            child->__rb_parent_color = (struct ev_map_node*)pc;
            rebalance = NULL;
        }
        else
            rebalance = __rb_is_black(pc) ? parent : NULL;
        tmp = parent;
    }
    else if (!child) {
        /* Still case 1, but this time the child is node->rb_left */
        pc = (uintptr_t)(node->__rb_parent_color);
        tmp->__rb_parent_color = (ev_map_node_t*)pc;
        parent = __rb_parent(pc);
        __rb_change_child(node, tmp, parent, root);
        rebalance = NULL;
        tmp = parent;
    }
    else {
        ev_map_node_t* successor = child, *child2;
        tmp = child->rb_left;
        if (!tmp) {
            /*
            * Case 2: node's successor is its right child
            *
            *    (n)          (s)
            *    / \          / \
            *  (x) (s)  ->  (x) (c)
            *        \
            *        (c)
            */
            parent = successor;
            child2 = successor->rb_right;
        }
        else {
            /*
            * Case 3: node's successor is leftmost under
            * node's right child subtree
            *
            *    (n)          (s)
            *    / \          / \
            *  (x) (y)  ->  (x) (y)
            *      /            /
            *    (p)          (p)
            *    /            /
            *  (s)          (c)
            *    \
            *    (c)
            */
            do {
                parent = successor;
                successor = tmp;
                tmp = tmp->rb_left;
            } while (tmp);
            parent->rb_left = child2 = successor->rb_right;
            successor->rb_right = child;
            rb_set_parent(child, successor);
        }

        successor->rb_left = tmp = node->rb_left;
        rb_set_parent(tmp, successor);

        pc = (uintptr_t)(node->__rb_parent_color);
        tmp = __rb_parent(pc);
        __rb_change_child(node, successor, tmp, root);
        if (child2) {
            successor->__rb_parent_color = (struct ev_map_node*)pc;
            rb_set_parent_color(child2, parent, RB_BLACK);
            rebalance = NULL;
        }
        else {
            uintptr_t pc2 = (uintptr_t)(successor->__rb_parent_color);
            successor->__rb_parent_color = (struct ev_map_node*)pc;
            rebalance = __rb_is_black(pc2) ? parent : NULL;
        }
        tmp = successor;
    }

    return rebalance;
}

/*
* Inline version for rb_erase() use - we want to be able to inline
* and eliminate the dummy_rotate callback there
*/
static void
____rb_erase_color(ev_map_node_t* parent, ev_map_t* root)
{
    ev_map_node_t* node = NULL, *sibling, *tmp1, *tmp2;

    for (;;) {
        /*
        * Loop invariants:
        * - node is black (or NULL on first iteration)
        * - node is not the root (parent is not NULL)
        * - All leaf paths going through parent and node have a
        *   black node count that is 1 lower than other leaf paths.
        */
        sibling = parent->rb_right;
        if (node != sibling) {  /* node == parent->rb_left */
            if (rb_is_red(sibling)) {
                /*
                * Case 1 - left rotate at parent
                *
                *     P               S
                *    / \             / \
                *   N   s    -->    p   Sr
                *      / \         / \
                *     Sl  Sr      N   Sl
                */
                parent->rb_right = tmp1 = sibling->rb_left;
                sibling->rb_left = parent;
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                    RB_RED);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_right;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /*
                    * Case 2 - sibling color flip
                    * (p could be either color here)
                    *
                    *    (p)           (p)
                    *    / \           / \
                    *   N   S    -->  N   s
                    *      / \           / \
                    *     Sl  Sr        Sl  Sr
                    *
                    * This leaves us violating 5) which
                    * can be fixed by flipping p to black
                    * if it was red, or by recursing at p.
                    * p is red when coming from Case 1.
                    */
                    rb_set_parent_color(sibling, parent,
                        RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /*
                * Case 3 - right rotate at sibling
                * (p could be either color here)
                *
                *   (p)           (p)
                *   / \           / \
                *  N   S    -->  N   Sl
                *     / \             \
                *    sl  Sr            s
                *                       \
                *                        Sr
                */
                sibling->rb_left = tmp1 = tmp2->rb_right;
                tmp2->rb_right = sibling;
                parent->rb_right = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                    RB_BLACK);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /*
            * Case 4 - left rotate at parent + color flips
            * (p and sl could be either color here.
            *  After rotation, p becomes black, s acquires
            *  p's color, and sl keeps its color)
            *
            *      (p)             (s)
            *      / \             / \
            *     N   S     -->   P   Sr
            *        / \         / \
            *      (sl) sr      N  (sl)
            */
            parent->rb_right = tmp2 = sibling->rb_left;
            sibling->rb_left = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                RB_BLACK);
            break;
        }
        else {
            sibling = parent->rb_left;
            if (rb_is_red(sibling)) {
                /* Case 1 - right rotate at parent */
                parent->rb_left = tmp1 = sibling->rb_right;
                sibling->rb_right = parent;
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                    RB_RED);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* Case 2 - sibling color flip */
                    rb_set_parent_color(sibling, parent,
                        RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* Case 3 - right rotate at sibling */
                sibling->rb_right = tmp1 = tmp2->rb_left;
                tmp2->rb_left = sibling;
                parent->rb_left = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                    RB_BLACK);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* Case 4 - left rotate at parent + color flips */
            parent->rb_left = tmp2 = sibling->rb_right;
            sibling->rb_right = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                RB_BLACK);
            break;
        }
    }
}

/**
 * @brief Inserting data into the tree.
 *
 * The insert instead must be implemented
 * in two steps: First, the code must insert the element in order as a red leaf
 * in the tree, and then the support library function #_ev_map_low_insert_color
 * must be called.
 *
 * @param node      The node you want to insert
 * @param parent    The position you want to insert
 * @param rb_link   Will be set to `node`
 * @see _ev_map_low_insert_color
 */
static void _ev_map_low_link_node(ev_map_node_t* node,
    ev_map_node_t* parent, ev_map_node_t** rb_link)
{
    node->__rb_parent_color = parent;
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
    return;
}

/**
 * @brief re-balancing ("recoloring") the tree.
 * @param node      The node just linked
 * @param root      The map
 * @see eaf_map_low_link_node
 */
static void _ev_map_low_insert_color(ev_map_node_t* node, ev_map_t* root)
{
    __rb_insert(node, root);
}

/**
 * @brief Delete the node from the map.
 * @warning The node must already in the map.
 * @param root      The pointer to the map
 * @param node      The node
 */
static void _ev_map_low_erase(ev_map_t* root, ev_map_node_t* node)
{
    ev_map_node_t* rebalance;
    rebalance = __rb_erase_augmented(node, root);
    if (rebalance)
        ____rb_erase_color(rebalance, root);
}

/**
 * @brief Returns an iterator to the beginning
 * @param root      The pointer to the map
 * @return          An iterator
 */
static ev_map_node_t* _ev_map_low_first(const ev_map_t* root)
{
    ev_map_node_t* n = root->rb_root;

    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

/**
 * @brief Returns an iterator to the end
 * @param root      The pointer to the map
 * @return          An iterator
 */
static ev_map_node_t* _ev_map_low_last(const ev_map_t* root)
{
    ev_map_node_t* n = root->rb_root;

    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

/**
 * @brief Get an iterator next to the given one.
 * @param node      Current iterator
 * @return          Next iterator
 */
static ev_map_node_t* _ev_map_low_next(const ev_map_node_t* node)
{
    ev_map_node_t* parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /*
    * If we have a right-hand child, go down and then left as far
    * as we can.
    */
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node = node->rb_left;
        return (ev_map_node_t *)node;
    }

    /*
    * No right-hand children. Everything down and left is smaller than us,
    * so any 'next' node must be in the general direction of our parent.
    * Go up the tree; any time the ancestor is a right-hand child of its
    * parent, keep going up. First time it's a left-hand child of its
    * parent, said parent is our 'next' node.
    */
    while ((parent = rb_parent(node)) != NULL && node == parent->rb_right)
        node = parent;

    return parent;
}

/**
 * @brief Get an iterator before the given one.
 * @param node      Current iterator
 * @return          Previous iterator
 */
static ev_map_node_t* _ev_map_low_prev(const ev_map_node_t* node)
{
    ev_map_node_t* parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /*
    * If we have a left-hand child, go down and then right as far
    * as we can.
    */
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node = node->rb_right;
        return (ev_map_node_t *)node;
    }

    /*
    * No left-hand children. Go up till we find an ancestor which
    * is a right-hand child of its parent.
    */
    while ((parent = rb_parent(node)) != NULL && node == parent->rb_left)
        node = parent;

    return parent;
}

void ev_map_init(ev_map_t* handler, ev_map_cmp_fn cmp, void* arg)
{
    handler->rb_root = NULL;
    handler->cmp.cmp = cmp;
    handler->cmp.arg = arg;
    handler->size = 0;
}

ev_map_node_t* ev_map_insert(ev_map_t* handler, ev_map_node_t* node)
{
    ev_map_node_t **new_node = &(handler->rb_root), *parent = NULL;

    /* Figure out where to put new node */
    while (*new_node)
    {
        int result = handler->cmp.cmp(node, *new_node, handler->cmp.arg);

        parent = *new_node;
        if (result < 0)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (result > 0)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            return *new_node;
        }
    }

    handler->size++;
    _ev_map_low_link_node(node, parent, new_node);
    _ev_map_low_insert_color(node, handler);

    return NULL;
}

void ev_map_erase(ev_map_t* handler, ev_map_node_t* node)
{
    handler->size--;
    _ev_map_low_erase(handler, node);
}

size_t ev_map_size(const ev_map_t* handler)
{
    return handler->size;
}

ev_map_node_t* ev_map_find(const ev_map_t* handler, const ev_map_node_t* key)
{
    ev_map_node_t* node = handler->rb_root;

    while (node)
    {
        int result = handler->cmp.cmp(key, node, handler->cmp.arg);

        if (result < 0)
        {
            node = node->rb_left;
        }
        else if (result > 0)
        {
            node = node->rb_right;
        }
        else
        {
            return node;
        }
    }

    return NULL;
}

ev_map_node_t* ev_map_find_lower(const ev_map_t* handler, const ev_map_node_t* key)
{
    ev_map_node_t* lower_node = NULL;
    ev_map_node_t* node = handler->rb_root;
    while (node)
    {
        int result = handler->cmp.cmp(key, node, handler->cmp.arg);
        if (result < 0)
        {
            node = node->rb_left;
        }
        else if (result > 0)
        {
            lower_node = node;
            node = node->rb_right;
        }
        else
        {
            return node;
        }
    }

    return lower_node;
}

ev_map_node_t* ev_map_find_upper(const ev_map_t* handler, const ev_map_node_t* key)
{
    ev_map_node_t* upper_node = NULL;
    ev_map_node_t* node = handler->rb_root;

    while (node)
    {
        int result = handler->cmp.cmp(key, node, handler->cmp.arg);

        if (result < 0)
        {
            upper_node = node;
            node = node->rb_left;
        }
        else if (result > 0)
        {
            node = node->rb_right;
        }
        else
        {
            if (upper_node == NULL)
            {
                upper_node = node->rb_right;
            }
            break;
        }
    }

    return upper_node;
}

ev_map_node_t* ev_map_begin(const ev_map_t* handler)
{
    return _ev_map_low_first(handler);
}

ev_map_node_t* ev_map_end(const ev_map_t* handler)
{
    return _ev_map_low_last(handler);
}

ev_map_node_t* ev_map_next(const ev_map_node_t* node)
{
    return _ev_map_low_next(node);
}

ev_map_node_t* ev_map_prev(const ev_map_node_t* node)
{
    return _ev_map_low_prev(node);
}

#line 1 "src/misc.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc.h" */
/* AMALGAMATE: #include "loop.h" */
#include <assert.h>
#include <string.h>

#if !defined(_WIN32)
#   include <net/if.h>
#   include <sys/un.h>
#endif

static void _ev_set_scope_id(struct sockaddr_in6* addr, const char* ip)
{
    const char* zone_index = strchr(ip, '%');
    if (zone_index == NULL)
    {
        return;
    }

    char address_part[40];
    size_t address_part_size = zone_index - ip;
    if (address_part_size >= sizeof(address_part))
    {
        address_part_size = sizeof(address_part) - 1;
    }

    memcpy(address_part, ip, address_part_size);
    address_part[address_part_size] = '\0';
    ip = address_part;

    zone_index++; /* skip '%' */
    /* NOTE: unknown interface (id=0) is silently ignored */
#ifdef _WIN32
    addr->sin6_scope_id = atoi(zone_index);
#else
    addr->sin6_scope_id = if_nametoindex(zone_index);
#endif
}

int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_port = htons((uint16_t)port);

    return inet_pton(AF_INET, ip, &addr->sin_addr) ? 0 : EV_EINVAL;
}

int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons((uint16_t)port);
    _ev_set_scope_id(addr, ip);

    return inet_pton(AF_INET6, ip, &addr->sin6_addr) ? 0 : EV_EINVAL;
}

int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len)
{
    if (port != NULL)
    {
        *port = ntohs(addr->sin_port);
    }

    if (ip != NULL)
    {
        return inet_ntop(AF_INET, &addr->sin_addr, ip, len) != NULL ?
            0 : EV_ENOSPC;
    }

    return 0;
}

int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len)
{
    if (port != NULL)
    {
        *port = ntohs(addr->sin6_port);
    }

    if (ip != NULL)
    {
        return inet_ntop(AF_INET6, &addr->sin6_addr, ip, len) != NULL ?
            0 : EV_ENOSPC;
    }

    return 0;
}

int ev_ip_addr(const char* ip, int port, struct sockaddr* addr, size_t size)
{
    if (strstr(ip, ":") != NULL)
    {
        if (size < sizeof(struct sockaddr_in6))
        {
            return EV_EINVAL;
        }
        return ev_ipv6_addr(ip, port, (struct sockaddr_in6*)addr);
    }

    if (size < sizeof(struct sockaddr_in))
    {
        return EV_EINVAL;
    }
    return ev_ipv4_addr(ip, port, (struct sockaddr_in*)addr);
}

int ev_ip_name(const struct sockaddr* addr, int* port, char* ip, size_t len)
{
	if (addr->sa_family == AF_INET)
	{
		return ev_ipv4_name((struct sockaddr_in*)addr, port, ip, len);
	}
	return ev_ipv6_name((struct sockaddr_in6*)addr, port, ip, len);
}

ev_buf_t ev_buf_make(void* buf, size_t len)
{
    ev_buf_t tmp;

#if defined(_WIN32)
    tmp.data = (CHAR*)buf;
    tmp.size = (ULONG)len;
#else
    tmp.data = buf;
    tmp.size = len;
#endif

    return tmp;
}

void ev_buf_make_n(ev_buf_t bufs[], size_t nbuf, ...)
{
    va_list ap;
    va_start(ap, nbuf);
    ev_buf_make_v(bufs, nbuf, ap);
    va_end(ap);
}

void ev_buf_make_v(ev_buf_t bufs[], size_t nbuf, va_list ap)
{
    size_t i;
    for (i = 0; i < nbuf; i++)
    {
        void* v_b = va_arg(ap, void*);
        size_t v_l = va_arg(ap, size_t);
        bufs[i] = ev_buf_make(v_b, v_l);
    }
}

EV_LOCAL int ev__translate_posix_sys_error(int syserr)
{
#define EV_EXPAND_ERRMAP(err, syserr, str) case syserr: return err;

    switch (syserr)
    {
    /* Success */
    case 0:                 return 0;
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:       return EV_EAGAIN;
#endif
    EV_ERRNO_POSIX_MAP(EV_EXPAND_ERRMAP);

    /* Unknown */
    default:                break;
    }

    EV_ABORT("unknown system errno %d.", syserr);

#undef EV_EXPAND_ERRMAP
}

#line 1 "src/pipe.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "pipe.h" */

int ev_pipe_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_write_cb cb)
{
    return ev_pipe_write_ex(pipe, req, bufs, nbuf, EV_ROLE_UNKNOWN, NULL, 0, cb);
}

EV_LOCAL int ev__pipe_read_init(ev_pipe_read_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_read_cb cb)
{
    int ret;
    if ((ret = ev__read_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }
    req->ucb = cb;

    req->handle.os_socket = EV_OS_SOCKET_INVALID;
    return 0;
}

EV_LOCAL int ev__pipe_write_init(ev_pipe_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_write_cb cb)
{
    return ev__pipe_write_init_ext(req, cb, bufs, nbuf, EV_ROLE_UNKNOWN, NULL, 0);
}

EV_LOCAL int ev__pipe_write_init_ext(ev_pipe_write_req_t* req, ev_pipe_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    int ret;
    if ((ret = ev__write_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }
    req->ucb = callback;

    req->handle.role = handle_role;
    switch (handle_role)
    {
        /* no handle need to send */
    case EV_ROLE_UNKNOWN:
        break;

    case EV_ROLE_EV_TCP:
        if (handle_size != sizeof(ev_tcp_t))
        {
            return EV_EINVAL;
        }
        req->handle.u.os_socket = ((ev_tcp_t*)handle_addr)->sock;
        break;

        /* not support other type */
    default:
        return EV_EINVAL;
    }

    return 0;
}

#line 1 "src/queue.c"
/* AMALGAMATE: #include "ev.h" */
#include <stdlib.h>

#define EV_QUEUE_NEXT(node)         ((node)->p_next)
#define EV_QUEUE_PREV(node)         ((node)->p_prev)
#define EV_QUEUE_PREV_NEXT(node)    (EV_QUEUE_NEXT(EV_QUEUE_PREV(node)))
#define EV_QUEUE_NEXT_PREV(node)    (EV_QUEUE_PREV(EV_QUEUE_NEXT(node)))

void ev_queue_init(ev_queue_node_t* head)
{
    EV_QUEUE_NEXT(head) = head;
    EV_QUEUE_PREV(head) = head;
}

void ev_queue_push_back(ev_queue_node_t* head, ev_queue_node_t* node)
{
    EV_QUEUE_NEXT(node) = head;
    EV_QUEUE_PREV(node) = EV_QUEUE_PREV(head);
    EV_QUEUE_PREV_NEXT(node) = node;
    EV_QUEUE_PREV(head) = node;
}

void ev_queue_push_front(ev_queue_node_t* head, ev_queue_node_t* node)
{
    EV_QUEUE_NEXT(node) = EV_QUEUE_NEXT(head);
    EV_QUEUE_PREV(node) = head;
    EV_QUEUE_NEXT_PREV(node) = node;
    EV_QUEUE_NEXT(head) = node;
}

void ev_queue_erase(ev_queue_node_t* node)
{
    EV_QUEUE_PREV_NEXT(node) = EV_QUEUE_NEXT(node);
    EV_QUEUE_NEXT_PREV(node) = EV_QUEUE_PREV(node);
}

ev_queue_node_t* ev_queue_pop_front(ev_queue_node_t* head)
{
    ev_queue_node_t* node = ev_queue_head(head);
    if (node == NULL)
    {
        return NULL;
    }

    ev_queue_erase(node);
    return node;
}

ev_queue_node_t* ev_queue_pop_back(ev_queue_node_t* head)
{
    ev_queue_node_t* node = EV_QUEUE_PREV(head);
    if (node == head)
    {
        return NULL;
    }

    ev_queue_erase(node);
    return node;
}

ev_queue_node_t* ev_queue_head(ev_queue_node_t* head)
{
    ev_queue_node_t* node = EV_QUEUE_NEXT(head);
    return node == head ? NULL : node;
}

ev_queue_node_t* ev_queue_next(ev_queue_node_t* head, ev_queue_node_t* node)
{
    ev_queue_node_t* next = EV_QUEUE_NEXT(node);
    return next == head ? NULL : next;
}

int ev_queue_empty(const ev_queue_node_t* node)
{
    return EV_QUEUE_NEXT(node) == node;
}

#line 1 "src/ringbuffer.c"
/* AMALGAMATE: #include "ringbuffer.h" */

#define EV_RB_BEG_POS(rb) \
    sizeof((rb)->basis)

#define EV_RB_BEG_NODE(rb)  \
    EV_RB_NODE(rb, EV_RB_BEG_POS(rb))

#define EV_RB_NODE(rb, pos)   \
    ((ring_buffer_node_t*)((uintptr_t)&((rb)->basis) + (pos)))

#define EV_RB_BASIS_DIFF(rb, ptr)   \
    ((uintptr_t)(ptr) - (uintptr_t)&((rb)->basis))

static void _ring_buffer_reinit(ring_buffer_t* rb)
{
    rb->counter.writing = 0;
    rb->counter.committed = 0;
    rb->counter.reading = 0;

    rb->node.off_reserve = 0;
    rb->node.off_HEAD = 0;
    rb->node.off_TAIL = 0;
}

/**
 * @brief Create the first node in empty ring buffer.
 * @param rb         ring buffer
 * @param data_len   length of user data
 * @param node_size  size of node
 * @return           token
 */
static ring_buffer_token_t* _ring_buffer_reserve_empty(ring_buffer_t* rb,
    size_t data_len, size_t node_size)
{
    /* check capacity */
    if (node_size > rb->config.capacity)
    {
        return NULL;
    }

    /* The value of ring_buffer_t::basis MUST be 0 */
    rb->node.off_HEAD = EV_RB_BEG_POS(rb);

    /* Initialize node */
    {
        ring_buffer_node_t* head = EV_RB_NODE(rb, rb->node.off_HEAD);
        head->state = RINGBUFFER_STAT_WRITING;
        head->token.size.size = data_len;

        /* Initialize position chain */
        head->chain_pos.off_next = rb->node.off_HEAD;
        head->chain_pos.off_prev = rb->node.off_HEAD;

        /* Initialize time chain */
        head->chain_time.off_newer = 0;
        head->chain_time.off_older = 0;
    }

    /* Initialize other resources */
    rb->node.off_TAIL = rb->node.off_HEAD;
    rb->node.off_reserve = rb->node.off_HEAD;
    rb->counter.writing++;

    ring_buffer_node_t* rb_oldest_reserve = EV_RB_NODE(rb, rb->node.off_reserve);
    return &rb_oldest_reserve->token;
}

/**
 * @brief Update time chain for \p new_node
 * @param rb         RingBuffer
 * @param new_node   New node
 */
static void _ring_buffer_update_time_for_new_node(ring_buffer_t* rb,
    ring_buffer_node_t* new_node)
{
    /* update chain_time */
    new_node->chain_time.off_newer = 0;
    new_node->chain_time.off_older = rb->node.off_HEAD;
    EV_RB_NODE(rb, new_node->chain_time.off_older)->chain_time.off_newer = EV_RB_BASIS_DIFF(rb, new_node);

    /* update HEAD */
    rb->node.off_HEAD = EV_RB_BASIS_DIFF(rb, new_node);
}

/**
 * @brief Perform overwrite
 */
static ring_buffer_token_t* _ring_buffer_perform_overwrite(
    ring_buffer_t* rb, uint8_t* start_point, ring_buffer_node_t* node_start,
    ring_buffer_node_t* node_end, size_t counter_lost_nodes,
    size_t data_len)
{
    ring_buffer_node_t* rb_tail = EV_RB_NODE(rb, rb->node.off_TAIL);
    ring_buffer_node_t* newer_node = EV_RB_NODE(rb, node_end->chain_time.off_newer);

    /*
     * here [node_start, node_end] will be overwrite,
     * so off_reserve need to move forward.
     * if TAIL was overwrite, then move TAIL too.
     */
    if (rb_tail == EV_RB_NODE(rb, rb->node.off_reserve))
    {
        rb->node.off_TAIL = EV_RB_BASIS_DIFF(rb, newer_node);
    }
    rb->node.off_reserve = EV_RB_BASIS_DIFF(rb, newer_node);

    /* generate new node */
    ring_buffer_node_t* new_node = (ring_buffer_node_t*)start_point;

    /* Update position chain */
    new_node->chain_pos.off_next = node_end->chain_pos.off_next;
    EV_RB_NODE(rb, new_node->chain_pos.off_next)->chain_pos.off_prev = EV_RB_BASIS_DIFF(rb, new_node);
    new_node->chain_pos.off_prev = node_start->chain_pos.off_prev;
    EV_RB_NODE(rb, new_node->chain_pos.off_prev)->chain_pos.off_next = EV_RB_BASIS_DIFF(rb, new_node);

    /* Update time chain */
    if (node_start->chain_time.off_older != 0)
    {
        EV_RB_NODE(rb, node_start->chain_time.off_older)->chain_time.off_newer =
            node_end->chain_time.off_newer;
    }
    if (node_end->chain_time.off_newer != 0)
    {
        EV_RB_NODE(rb, node_end->chain_time.off_newer)->chain_time.off_older =
            node_start->chain_time.off_older;
    }
    _ring_buffer_update_time_for_new_node(rb, new_node);

    /* Update counter */
    rb->counter.committed -= counter_lost_nodes;
    rb->counter.writing += 1;

    /* Update data length */
    new_node->token.size.size = data_len;

    /* Update node status */
    new_node->state = RINGBUFFER_STAT_WRITING;

    return &new_node->token;
}

/**
 * @brief Try to overwrite
 */
static ring_buffer_token_t* _ring_buffer_reserve_try_overwrite(
    ring_buffer_t* rb, size_t data_len, size_t node_size)
{
    ring_buffer_node_t* rb_oldest_reserve = EV_RB_NODE(rb, rb->node.off_reserve);
    /* Overwrite only works for committed nodes */
    if (rb->node.off_reserve == 0
        || rb_oldest_reserve->state != RINGBUFFER_STAT_COMMITTED)
    {
        return NULL;
    }

    /* Short cut: if only exists one node, check whether whole ring buffer can hold this data */
    if (rb_oldest_reserve->chain_pos.off_next == rb->node.off_reserve)
    {
        if (rb->config.capacity < node_size)
        {
            return NULL;
        }

        /* If we can, re-initialize and add this node */
        _ring_buffer_reinit(rb);
        return _ring_buffer_reserve_empty(rb, data_len, node_size);
    }

    /* Step 1. Calculate where overwrite start */
    const ring_buffer_node_t* backward_node = EV_RB_NODE(rb, rb_oldest_reserve->chain_pos.off_prev);
    uint8_t* start_point = (backward_node < rb_oldest_reserve) ?
        ((uint8_t*)backward_node + ring_buffer_node_cost(backward_node->token.size.size)) :
        (uint8_t*)EV_RB_BEG_NODE(rb);

    /* Step 2. Calculate whether continuous committed nodes could hold needed data */
    size_t sum_size = 0;
    size_t counter_lost_nodes = 1;
    ring_buffer_node_t* node_end = rb_oldest_reserve;

    while (1)
    {
        sum_size = (uint8_t*)node_end + ring_buffer_node_cost(node_end->token.size.size) - (uint8_t*)start_point;

        ring_buffer_node_t* forward_of_node_end = EV_RB_NODE(rb, node_end->chain_pos.off_next);
        if (!(sum_size < node_size /* overwrite minimum nodes */
            && forward_of_node_end->state == RINGBUFFER_STAT_COMMITTED /* only overwrite committed node */
            && node_end->chain_pos.off_next == node_end->chain_time.off_newer /* node must both physical and time continuous */
            && forward_of_node_end > node_end /* cannot interrupt by array boundary */
            ))
        {
            break;
        }
        node_end = EV_RB_NODE(rb, node_end->chain_pos.off_next);
        counter_lost_nodes++;
    }

    /* Step 3. check if condition allow to overwrite */
    if (sum_size < node_size)
    {
        return NULL;
    }

    /* Step 4. perform overwrite */
    return _ring_buffer_perform_overwrite(rb, start_point, rb_oldest_reserve,
        node_end, counter_lost_nodes, data_len);
}

inline static void _ring_buffer_insert_new_node(ring_buffer_t* rb,
    ring_buffer_node_t* new_node, size_t data_len)
{
    ring_buffer_node_t* rb_head = EV_RB_NODE(rb, rb->node.off_HEAD);

    /* initialize token */
    new_node->state = RINGBUFFER_STAT_WRITING;
    new_node->token.size.size = data_len;

    /* update chain_pos */
    new_node->chain_pos.off_next = rb_head->chain_pos.off_next;
    new_node->chain_pos.off_prev = rb->node.off_HEAD;
    EV_RB_NODE(rb, new_node->chain_pos.off_next)->chain_pos.off_prev = EV_RB_BASIS_DIFF(rb, new_node);
    EV_RB_NODE(rb, new_node->chain_pos.off_prev)->chain_pos.off_next = EV_RB_BASIS_DIFF(rb, new_node);

    _ring_buffer_update_time_for_new_node(rb, new_node);
}

/**
 * @brief Update off_reserve
 */
static void _ring_buffer_reserve_update_oldest_reserve(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    if (rb->node.off_reserve == 0)
    {
        rb->node.off_reserve = EV_RB_BASIS_DIFF(rb, node);
    }
}

static ring_buffer_token_t* _ring_buffer_reserve_none_empty(
    ring_buffer_t* rb, size_t data_len, size_t node_size, int flags)
{
    ring_buffer_node_t* rb_head = EV_RB_NODE(rb, rb->node.off_HEAD);

    /**
     * Get next possible node in right side of HEAD
     * If there is a node exists, the address of that node is larger or equal to calculated address.
     */
    size_t next_possible_pos = rb->node.off_HEAD + ring_buffer_node_cost(rb_head->token.size.size);
    ring_buffer_node_t* next_possible_node = EV_RB_NODE(rb, next_possible_pos);

    /* If have a existing node on right side of HEAD, we can use that space */
    if (rb_head->chain_pos.off_next > rb->node.off_HEAD)
    {
        /* If space is enough, generate token */
        if (rb_head->chain_pos.off_next - next_possible_pos >= node_size)
        {
            rb->counter.writing++;
            _ring_buffer_insert_new_node(rb, next_possible_node, data_len);
            _ring_buffer_reserve_update_oldest_reserve(rb, next_possible_node);
            return &next_possible_node->token;
        }

        /* Otherwise overwrite */
        return (flags & RINGBUFFER_FLAG_OVERWRITE) ?
            _ring_buffer_reserve_try_overwrite(rb, data_len, node_size) :
            NULL;
    }

    /* If no existing node on right side, try to create token */
    if ((rb->config.capacity - (next_possible_pos - EV_RB_BEG_POS(rb))) >= node_size)
    {
        rb->counter.writing++;
        _ring_buffer_insert_new_node(rb, next_possible_node, data_len);
        _ring_buffer_reserve_update_oldest_reserve(rb, next_possible_node);
        return &next_possible_node->token;
    }

    /* if area on the most left cache is enough, make token */
    if (rb_head->chain_pos.off_next - EV_RB_BEG_POS(rb) >= node_size)
    {
        next_possible_node = EV_RB_BEG_NODE(rb);
        rb->counter.writing++;
        _ring_buffer_insert_new_node(rb, next_possible_node, data_len);
        _ring_buffer_reserve_update_oldest_reserve(rb, next_possible_node);
        return &next_possible_node->token;
    }

    /* in other condition, overwrite if needed */
    return (flags & RINGBUFFER_FLAG_OVERWRITE) ?
        _ring_buffer_reserve_try_overwrite(rb, data_len, node_size) : NULL;
}

inline static int _ring_buffer_commit_for_write_confirm(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    (void)rb;

    /* Update counter */
    rb->counter.writing--;
    rb->counter.committed++;

    /* Update node status */
    node->state = RINGBUFFER_STAT_COMMITTED;

    return 0;
}

/**
 * @brief Update position chain and time chain after delete node.
 * @param node   The node to delete
 */
static void _ring_buffer_delete_node_update_chain(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    /* Update position chain */
    EV_RB_NODE(rb, node->chain_pos.off_prev)->chain_pos.off_next = node->chain_pos.off_next;
    EV_RB_NODE(rb, node->chain_pos.off_next)->chain_pos.off_prev = node->chain_pos.off_prev;

    /* Update time chain */
    if (node->chain_time.off_older != 0)
    {
        EV_RB_NODE(rb, node->chain_time.off_older)->chain_time.off_newer = node->chain_time.off_newer;
    }
    if (node->chain_time.off_newer != 0)
    {
        EV_RB_NODE(rb, node->chain_time.off_newer)->chain_time.off_older = node->chain_time.off_older;
    }
}

/**
 * @brief Completely remove a node from ring buffer
 * @param rb    ring buffer
 * @param node  node to be delete
 */
static void _ring_buffer_delete_node(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    /**
     * Short cut: If only one node in ring buffer, re-initialize.
     * Here use `p_forward` to check if it meets the condition:
     * 1. `chain_pos.off_next` always point to next node. If it point to self, there is only one node.
     * 2. Access to `node` means `node` is in CPU cache line. By access `p_forward` can avoid cache miss.
     */
    if (node->chain_pos.off_next == EV_RB_BASIS_DIFF(rb, node))
    {
        _ring_buffer_reinit(rb);
        return;
    }

    /* Delete node and update chains */
    _ring_buffer_delete_node_update_chain(rb, node);

    /* Update off_reserve */
    if (rb->node.off_reserve == EV_RB_BASIS_DIFF(rb, node))
    {
        rb->node.off_reserve = node->chain_time.off_newer;
    }

    /* Update TAIL if necessary */
    if (node->chain_time.off_older == 0)
    {
        rb->node.off_TAIL = node->chain_time.off_newer;
        return;
    }

     /* Update HEAD if necessary */
    if (node->chain_time.off_newer == 0)
    {
        rb->node.off_HEAD = node->chain_time.off_older;
        return;
    }

    return;
}

static int _ring_buffer_commit_for_write_discard(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    rb->counter.writing--;
    _ring_buffer_delete_node(rb, node);
    return 0;
}

static int _ring_buffer_commit_for_write(ring_buffer_t* rb,
    ring_buffer_node_t* node, int flags)
{
    return (flags & RINGBUFFER_FLAG_DISCARD) ?
        _ring_buffer_commit_for_write_discard(rb, node) :
        _ring_buffer_commit_for_write_confirm(rb, node);
}

static int _ring_buffer_commit_for_consume_confirm(ring_buffer_t* rb,
    ring_buffer_node_t* node)
{
    rb->counter.reading--;
    _ring_buffer_delete_node(rb, node);
    return 0;
}

/**
 * @brief Discard a consumed token.
 * the only condition a consumed token can be discard is no one consume newer token
 */
static int _ring_buffer_commit_for_consume_discard(ring_buffer_t* rb,
    ring_buffer_node_t* node, int flags)
{
    /* If existing a newer consumer, should fail. */
    if (node->chain_time.off_newer != 0
        && EV_RB_NODE(rb, node->chain_time.off_newer)->state == RINGBUFFER_STAT_READING)
    {
        return (flags & RINGBUFFER_FLAG_ABANDON) ?
            _ring_buffer_commit_for_consume_confirm(rb, node) : -1;
    }

    /* Update counter and status */
    rb->counter.reading--;
    rb->counter.committed++;
    node->state = RINGBUFFER_STAT_COMMITTED;

    /* if no newer node, then off_reserve should point to this node */
    if (node->chain_time.off_newer == 0)
    {
        rb->node.off_reserve = EV_RB_BASIS_DIFF(rb, node);
        return 0;
    }

    /* if node is just older than off_reserve, then off_reserve should move back */
    if (rb->node.off_reserve != 0
        && EV_RB_NODE(rb, rb->node.off_reserve)->chain_time.off_older == EV_RB_BASIS_DIFF(rb, node))
    {
        rb->node.off_reserve = EV_RB_BASIS_DIFF(rb, node);
        return 0;
    }

    return 0;
}

static int _ring_buffer_commit_for_consume(ring_buffer_t* rb,
    ring_buffer_node_t* node, int flags)
{
    return (flags & RINGBUFFER_FLAG_DISCARD) ?
        _ring_buffer_commit_for_consume_discard(rb, node, flags) :
        _ring_buffer_commit_for_consume_confirm(rb, node);
}

EV_LOCAL size_t ring_buffer_heap_cost(void)
{
    /* need to align with machine size */
    return ALIGN_SIZE(sizeof(struct ring_buffer), sizeof(void*));
}

EV_LOCAL size_t ring_buffer_node_cost(size_t size)
{
    return ALIGN_SIZE(sizeof(ring_buffer_node_t) + size, sizeof(void*));
}

EV_LOCAL ring_buffer_t* ring_buffer_init(void* buffer, size_t size)
{
    /* Calculate start address */
    ring_buffer_t* handler = buffer;

    /* Check space size */
    if (ring_buffer_heap_cost() + ring_buffer_node_cost(0) >= size)
    {
        return NULL;
    }

    /* setup necessary field */
    handler->config.capacity = size - ring_buffer_heap_cost();
    handler->basis = NULL;

    /* initialize */
    _ring_buffer_reinit(handler);

    return handler;
}

EV_LOCAL ring_buffer_token_t* ring_buffer_reserve(ring_buffer_t* handler, size_t len,
    int flags)
{
    /* node must aligned */
    const size_t node_size = ring_buffer_node_cost(len);

    /* empty ring buffer */
    if (handler->node.off_TAIL == 0)
    {
        return _ring_buffer_reserve_empty(handler, len, node_size);
    }

    /* non empty ring buffer */
    return _ring_buffer_reserve_none_empty(handler, len, node_size, flags);
}

EV_LOCAL ring_buffer_token_t* ring_buffer_consume(ring_buffer_t* handler)
{
    ring_buffer_node_t* rb_oldest_reserve = EV_RB_NODE(handler, handler->node.off_reserve);
    if (handler->node.off_reserve == 0
        || rb_oldest_reserve->state != RINGBUFFER_STAT_COMMITTED)
    {
        return NULL;
    }

    handler->counter.committed--;
    handler->counter.reading++;

    ring_buffer_node_t* token_node = rb_oldest_reserve;
    handler->node.off_reserve = rb_oldest_reserve->chain_time.off_newer;
    token_node->state = RINGBUFFER_STAT_READING;

    return &token_node->token;
}

EV_LOCAL int ring_buffer_commit(ring_buffer_t* handler, ring_buffer_token_t* token, int flags)
{
    ring_buffer_node_t* node = EV_CONTAINER_OF(token, ring_buffer_node_t, token);

    return node->state == RINGBUFFER_STAT_WRITING ?
        _ring_buffer_commit_for_write(handler, node, flags) :
        _ring_buffer_commit_for_consume(handler, node, flags);
}

EV_LOCAL size_t ring_buffer_count(ring_buffer_t* handler, ring_buffer_counter_t* counter)
{
    if (counter != NULL)
    {
        *counter = handler->counter;
    }

    return handler->counter.committed + handler->counter.reading + handler->counter.writing;
}

EV_LOCAL ring_buffer_token_t* ring_buffer_begin(const ring_buffer_t* handler)
{
    ring_buffer_node_t* rb_tail = EV_RB_NODE(handler, handler->node.off_TAIL);
    return &(rb_tail->token);
}

EV_LOCAL ring_buffer_token_t* ring_buffer_next(const ring_buffer_t* handler, const ring_buffer_token_t* token)
{
    ring_buffer_node_t* node = EV_CONTAINER_OF(token, ring_buffer_node_t, token);
    if (node->chain_time.off_newer == 0)
    {
        return NULL;
    }

    node = EV_RB_NODE(handler, node->chain_time.off_newer);
    return &(node->token);
}

#line 1 "src/shmem.c"
/* AMALGAMATE: #include "shmem.h" */

void* ev_shm_addr(ev_shm_t* shm)
{
    return shm->addr;
}

size_t ev_shm_size(ev_shm_t* shm)
{
    return shm->size;
}

#line 1 "src/threadpool.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "threadpool.h" */
#include <assert.h>

typedef struct ev_threadpool_default
{
    ev_threadpool_t pool;
    ev_os_thread_t  storage[4];
} ev_threadpool_default_t;

static ev_threadpool_default_t s_default_threadpool;

static void _ev_threadpool_on_init_default(void)
{
    int ret = ev_threadpool_init(&s_default_threadpool.pool, NULL,
        s_default_threadpool.storage, ARRAY_SIZE(s_default_threadpool.storage));
    if (ret != 0)
    {
        EV_ABORT("%s(%d)", ev_strerror(ret), ret);
    }
}

static void _ev_threadpool_init_default(void)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_threadpool_on_init_default);
}

static void _ev_threadpool_on_loop(ev_handle_t* handle)
{
    ev_work_t* work = EV_CONTAINER_OF(handle, ev_work_t, base);

    ev__handle_event_dec(&work->base);
    ev__handle_exit(&work->base, NULL);

    work->data.done_cb(work, work->data.status);
}

static void _ev_threadpool_submit_to_loop(ev_work_t* work)
{
    ev_loop_t* loop = work->base.loop;

    work->base.backlog.cb = _ev_threadpool_on_loop;
    ev_mutex_enter(&loop->threadpool.mutex);
    {
        ev_list_push_back(&loop->threadpool.work_queue, &work->base.backlog.node);
    }
    ev_mutex_leave(&loop->threadpool.mutex);

    ev__threadpool_wakeup(loop);
}

static ev_work_t* _ev_threadpool_get_work_locked(ev_threadpool_t* pool)
{
    ev_queue_node_t* it;
    size_t i;

    ev_mutex_enter(&pool->mutex);
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        it = ev_queue_pop_front(&pool->work_queue[i]);
        if (it != NULL)
        {
            ev_queue_init(it);
            break;
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (it == NULL)
    {
        return NULL;
    }

    return EV_CONTAINER_OF(it, ev_work_t, node);
}

static void _ev_threadpool_commit(ev_work_t* work, int status)
{
    work->data.status = status;
    _ev_threadpool_submit_to_loop(work);
}

static void _ev_threadpool_do_work(ev_threadpool_t* pool)
{
    ev_work_t* work = _ev_threadpool_get_work_locked(pool);
    if (work == NULL)
    {
        return;
    }

    work->data.status = EV_EBUSY;
    work->data.work_cb(work);

    _ev_threadpool_commit(work, 0);
}

static void _ev_threadpool_cleanup(ev_threadpool_t* pool)
{
    ev_work_t* work;
    while ((work = _ev_threadpool_get_work_locked(pool)) != NULL)
    {
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_worker(void* arg)
{
    ev_threadpool_t* pool = arg;

    while (pool->looping)
    {
        ev_sem_wait(&pool->p2w_sem);
        _ev_threadpool_do_work(pool);
    }
    _ev_threadpool_cleanup(pool);
}

static void _cancel_work_queue_for_loop(ev_threadpool_t* pool,
        ev_queue_node_t* wqueue, ev_loop_t* loop)
{
    ev_queue_node_t* it;

    ev_mutex_enter(&pool->mutex);
    {
        it = ev_queue_head(wqueue);
        while (it != NULL)
        {
            ev_work_t* work = EV_CONTAINER_OF(it, ev_work_t, node);
            it = ev_queue_next(wqueue, it);

            if (work->base.loop != loop)
            {
                continue;
            }

            _ev_threadpool_commit(work, EV_ECANCELED);
        }
    }
    ev_mutex_leave(&pool->mutex);
}

int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num)
{
    int ret;
    size_t i, idx;

    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        ev_queue_init(&pool->work_queue[i]);
    }

    pool->threads = storage;
    pool->thrnum = num;
    pool->looping = 1;
    ev_mutex_init(&pool->mutex, 0);
    ev_sem_init(&pool->p2w_sem, 0);

    for (idx = 0; idx < num; idx++)
    {
        ret = ev_thread_init(&storage[idx], opt, _ev_threadpool_worker, pool);
        if (ret < 0)
        {
            goto err_release_thread;
        }
    }

    return 0;

err_release_thread:
    pool->looping = 0;
    for (i = 0; i < idx; i++)
    {
        ev_thread_exit(&storage[i], EV_INFINITE_TIMEOUT);
    }
    ev_sem_exit(&pool->p2w_sem);
    ev_mutex_exit(&pool->mutex);
    return ret;
}

int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_work_t* work, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb)
{
    if (!pool->looping)
    {
        return EV_EACCES;
    }
    assert(type < ARRAY_SIZE(pool->work_queue));

    ev__handle_init(loop, &work->base, EV_ROLE_EV_WORK);
    ev__handle_event_add(&work->base);
    work->data.pool = pool;
    work->data.status = EV_ELOOP;
    work->data.work_cb = work_cb;
    work->data.done_cb = done_cb;

    ev_mutex_enter(&pool->mutex);
    {
        ev_queue_push_back(&pool->work_queue[type], &work->node);
    }
    ev_mutex_leave(&pool->mutex);

    ev_sem_post(&pool->p2w_sem);

    return 0;
}

int ev_loop_cancel(ev_work_t* work)
{
    ev_threadpool_t* pool = work->data.pool;

    int cancelled;
    ev_mutex_enter(&pool->mutex);
    {
        cancelled = !ev_queue_empty(&work->node);
        if (cancelled)
        {
            ev_queue_erase(&work->node);
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (!cancelled)
    {
        return EV_EBUSY;
    }

    _ev_threadpool_commit(work, EV_ECANCELED);

    return 0;
}

static void _threadpool_cancel_work_queue(ev_threadpool_t* pool, ev_queue_node_t* wqueue)
{
    (void)pool;
    ev_queue_node_t* node;
    while ((node = ev_queue_pop_front(wqueue)) != NULL)
    {
        ev_work_t* work = EV_CONTAINER_OF(node, ev_work_t, node);
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_cancel_all(ev_threadpool_t* pool)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _threadpool_cancel_work_queue(pool, &pool->work_queue[i]);
    }
}

void ev_threadpool_exit(ev_threadpool_t* pool)
{
    size_t i;
    int errcode;

    /* stop loop */
    pool->looping = 0;
    for (i = 0; i < pool->thrnum; i++)
    {
        ev_sem_post(&pool->p2w_sem);
    }

    /* exit thread */
    for (i = 0; i < pool->thrnum; i++)
    {
        errcode = ev_thread_exit(&pool->threads[i], EV_INFINITE_TIMEOUT);
        if (errcode != 0)
        {
            EV_ABORT("ev_thread_exit:%d", errcode);
        }
    }

    /* now we can do some cleanup */
    _ev_threadpool_cancel_all(pool);

    ev_mutex_exit(&pool->mutex);
    ev_sem_exit(&pool->p2w_sem);
}

EV_LOCAL void ev__loop_link_to_default_threadpool(ev_loop_t* loop)
{
    _ev_threadpool_init_default();
    if (loop->threadpool.pool == NULL)
    {
        ev_loop_link_threadpool(loop, &s_default_threadpool.pool);
    }
}

EV_LOCAL int ev__loop_submit_threadpool(ev_loop_t* loop,
    ev_work_t* work,
    ev_work_type_t type, ev_work_cb work_cb,
    ev_work_done_cb done_cb)
{
    ev_threadpool_t* pool = loop->threadpool.pool;

    return ev_threadpool_submit(pool, loop, work, type, work_cb, done_cb);
}

EV_LOCAL int ev_loop_link_threadpool(ev_loop_t* loop, ev_threadpool_t* pool)
{
    if (loop->threadpool.pool != NULL)
    {
        return EV_EBUSY;
    }

    loop->threadpool.pool = pool;

    ev_mutex_enter(&pool->mutex);
    {
        ev_list_push_back(&pool->loop_table, &loop->threadpool.node);
    }
    ev_mutex_leave(&pool->mutex);

    return 0;
}

int ev_loop_unlink_threadpool(ev_loop_t* loop)
{
    size_t i;
    ev_threadpool_t* pool = loop->threadpool.pool;

    /**
     * Cancel all pending task from target loop.
     * However, due to #ev_threadpool_submit() is a public API, there might be
     * some tasks which not come from loop directly. Thus they can be unexpected
     * canceled as well.
     */
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _cancel_work_queue_for_loop(pool, &pool->work_queue[i], loop);
    }

    /* From now there should no futher work from loop incoming. */
    loop->threadpool.pool = NULL;
    ev_mutex_enter(&pool->mutex);
    {
        ev_list_erase(&pool->loop_table, &loop->threadpool.node);
    }
    ev_mutex_leave(&pool->mutex);

    return 0;
}

int ev_loop_queue_work(ev_loop_t* loop, ev_work_t* token,
    ev_work_cb work_cb, ev_work_done_cb done_cb)
{
    return ev_threadpool_submit(loop->threadpool.pool, loop, token,
        EV_THREADPOOL_WORK_CPU, work_cb, done_cb);
}

EV_LOCAL void ev__threadpool_process(ev_loop_t* loop)
{
    ev_list_node_t* node;
    for (;;)
    {
        ev_mutex_enter(&loop->threadpool.mutex);
        {
            node = ev_list_pop_front(&loop->threadpool.work_queue);
        }
        ev_mutex_leave(&loop->threadpool.mutex);

        if (node == NULL)
        {
            break;
        }

        ev_handle_t* handle = EV_CONTAINER_OF(node, ev_handle_t, backlog.node);
        handle->backlog.cb(handle);
    }
}

#line 1 "src/timer.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "timer.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "loop.h" */
#include <string.h>

static int _ev_cmp_timer(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_timer_t* t1 = EV_CONTAINER_OF(key1, ev_timer_t, node);
    ev_timer_t* t2 = EV_CONTAINER_OF(key2, ev_timer_t, node);

    if (t1->data.active == t2->data.active)
    {
        if (t1 == t2)
        {
            return 0;
        }
        return t1 < t2 ? -1 : 1;
    }

    return t1->data.active < t2->data.active ? -1 : 1;
}

static void _ev_timer_on_close(ev_handle_t* handle)
{
    ev_timer_t* timer = EV_CONTAINER_OF(handle, ev_timer_t, base);
    if (timer->close_cb != NULL)
    {
        timer->close_cb(timer);
    }
}

EV_LOCAL void ev__init_timer(ev_loop_t* loop)
{
    ev_map_init(&loop->timer.heap, _ev_cmp_timer, NULL);
}

EV_LOCAL void ev__process_timer(ev_loop_t* loop)
{
    ev_map_node_t* it;
    while ((it = ev_map_begin(&loop->timer.heap)) != NULL)
    {
        ev_timer_t* timer = EV_CONTAINER_OF(it, ev_timer_t, node);
        if (timer->data.active > loop->hwtime)
        {
            break;
        }

        ev_timer_stop(timer);
        if (timer->attr.repeat != 0)
        {
            ev_timer_start(timer, timer->attr.cb, timer->attr.repeat, timer->attr.repeat);
        }
        timer->attr.cb(timer);
    }
}

int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle)
{
    memset(handle, 0, sizeof(*handle));

    ev__handle_init(loop, &handle->base, EV_ROLE_EV_TIMER);
    return 0;
}

void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb)
{
    handle->close_cb = close_cb;

    ev_timer_stop(handle);
    ev__handle_exit(&handle->base, _ev_timer_on_close);
}

int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat)
{
    ev_loop_t* loop = handle->base.loop;
    if (ev__handle_is_active(&handle->base))
    {
        ev_timer_stop(handle);
    }

    handle->attr.cb = cb;
    handle->attr.timeout = timeout;
    handle->attr.repeat = repeat;
    handle->data.active = loop->hwtime + timeout;

    if (ev_map_insert(&loop->timer.heap, &handle->node) != 0)
    {
        EV_ABORT("duplicate timer");
    }
    ev__handle_event_add(&handle->base);

    return 0;
}

void ev_timer_stop(ev_timer_t* handle)
{
    if (!ev__handle_is_active(&handle->base))
    {
        return;
    }

    ev__handle_event_dec(&handle->base);
    ev_map_erase(&handle->base.loop->timer.heap, &handle->node);
}

#line 1 "src/udp.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "udp.h" */
#include <string.h>

EV_LOCAL int ev__udp_interface_addr_to_sockaddr(struct sockaddr_storage* dst,
    const char* interface_addr, int is_ipv6)
{
    struct sockaddr_in* addr_4 = (struct sockaddr_in*)dst;
    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)dst;

    if (interface_addr == NULL)
    {
        memset(dst, 0, sizeof(*dst));
        if (is_ipv6)
        {
            dst->ss_family = AF_INET6;
            addr_6->sin6_scope_id = 0;
        }
        else
        {
            dst->ss_family = AF_INET;
            addr_4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
    }
    else if (ev_ipv4_addr(interface_addr, 0, addr_4) == 0)
    {
        /* do nothing, address was parsed */
    }
    else if (ev_ipv6_addr(interface_addr, 0, addr_6) == 0)
    {
        /* do nothing, address was parsed */
    }
    else
    {
        return EV_EINVAL;
    }

    return 0;
}

int ev_udp_try_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs, size_t nbuf,
    const struct sockaddr* addr, ev_udp_write_cb cb)
{
    if (ev_list_size(&udp->send_list) != 0)
    {
        return EV_EAGAIN;
    }

    return ev_udp_send(udp, req, bufs, nbuf, addr, cb);
}

int ev_udp_recv(ev_udp_t* udp, ev_udp_read_t* req, ev_buf_t* bufs, size_t nbuf, ev_udp_recv_cb cb)
{
    int ret;
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EPIPE;
    }

    req->usr_cb = cb;
    ev__handle_init(udp->base.loop, &req->handle, EV_ROLE_EV_REQ_UDP_R);

    if ((ret = ev__read_init(&req->base, bufs, nbuf)) != 0)
    {
        goto err;
    }
    ev_list_push_back(&udp->recv_list, &req->base.node);

    if ((ret = ev__udp_recv(udp, req)) != 0)
    {
        ev__read_exit(&req->base);
        ev_list_erase(&udp->recv_list, &req->base.node);
        goto err;
    }

    return 0;

err:
    ev__handle_exit(&req->handle, NULL);
    return ret;
}

int ev_udp_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs, size_t nbuf,
    const struct sockaddr* addr, ev_udp_write_cb cb)
{
    int ret;

    req->usr_cb = cb;
    ev__handle_init(udp->base.loop, &req->handle, EV_ROLE_EV_REQ_UDP_W);

    if ((ret = ev__write_init(&req->base, bufs, nbuf)) != 0)
    {
        goto err;
    }
    ev_list_push_back(&udp->send_list, &req->base.node);

    socklen_t addrlen = addr != NULL ? ev__get_addr_len(addr) : 0;
    if ((ret = ev__udp_send(udp, req, addr, addrlen)) != 0)
    {
        goto err_cleanup_write;
    }

    return 0;

err_cleanup_write:
    ev_list_erase(&udp->send_list, &req->base.node);
    ev__write_exit(&req->base);
err:
    ev__handle_exit(&req->handle, NULL);
    return ret;
}

#line 1 "src/version.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */

#define _TOSTR(x)       #x
#define TOSTR(x)        _TOSTR(x)

#if EV_VERSION_PREREL
#   define EV_VERSION_STR   \
        TOSTR(EV_VERSION_MAJOR) "." TOSTR(EV_VERSION_MINOR) "." TOSTR(EV_VERSION_PATCH) "-dev" TOSTR(EV_VERSION_PREREL)
#else
#   define EV_VERSION_STR   \
        TOSTR(EV_VERSION_MAJOR) "." TOSTR(EV_VERSION_MINOR) "." TOSTR(EV_VERSION_PATCH)
#endif

const char* ev_version_str(void)
{
    return EV_VERSION_STR;
}

unsigned ev_version_code(void)
{
    return EV_VERSION_CODE;
}

#if defined(_WIN32) /* AMALGAMATE: ev.c (1/3) */
#line 1 "src/win/winapi.h"
#ifndef __EV_WINAPI_INTERNAL_H__
#define __EV_WINAPI_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/* AMALGAMATE: #include "loop.h" */
#include <windows.h>

#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0)  LONG NTSTATUS;
#endif

#ifndef FACILITY_NTWIN32
#   define FACILITY_NTWIN32                 0x7
#endif

#ifndef NT_SUCCESS
#   define NT_SUCCESS(Status)               (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef NT_INFORMATION
# define NT_INFORMATION(status) ((((ULONG) (status)) >> 30) == 1)
#endif

#ifndef NT_WARNING
# define NT_WARNING(status) ((((ULONG) (status)) >> 30) == 2)
#endif

#ifndef NT_ERROR
#   define NT_ERROR(status) ((((ULONG) (status)) >> 30) == 3)
#endif

#ifndef STATUS_SUCCESS
#   define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_RECEIVE_PARTIAL
#   define STATUS_RECEIVE_PARTIAL           ((NTSTATUS)0x4000000FL)
#endif

#ifndef STATUS_RECEIVE_EXPEDITED
#   define STATUS_RECEIVE_EXPEDITED         ((NTSTATUS)0x40000010L)
#endif

#ifndef STATUS_RECEIVE_PARTIAL_EXPEDITED
#   define STATUS_RECEIVE_PARTIAL_EXPEDITED ((NTSTATUS)0x40000011L)
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#   define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#endif

#ifndef STATUS_NOT_IMPLEMENTED
#   define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#endif

#ifndef STATUS_PAGEFILE_QUOTA
#   define STATUS_PAGEFILE_QUOTA            ((NTSTATUS)0xC0000007L)
#endif

#ifndef STATUS_NO_SUCH_DEVICE
#   define STATUS_NO_SUCH_DEVICE            ((NTSTATUS)0xC000000EL)
#endif

#ifndef STATUS_NO_SUCH_FILE
#   define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)
#endif

#ifndef STATUS_CONFLICTING_ADDRESSES
#   define STATUS_CONFLICTING_ADDRESSES     ((NTSTATUS)0xC0000018L)
#endif

#ifndef STATUS_ACCESS_DENIED
#   define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#   define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif

#ifndef STATUS_OBJECT_TYPE_MISMATCH
#   define STATUS_OBJECT_TYPE_MISMATCH      ((NTSTATUS)0xC0000024L)
#endif

#ifndef STATUS_OBJECT_NAME_NOT_FOUND
#   define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)
#endif

#ifndef STATUS_OBJECT_PATH_NOT_FOUND
#   define STATUS_OBJECT_PATH_NOT_FOUND     ((NTSTATUS)0xC000003AL)
#endif

#ifndef STATUS_SHARING_VIOLATION
#   define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)
#endif

#ifndef STATUS_QUOTA_EXCEEDED
#   define STATUS_QUOTA_EXCEEDED            ((NTSTATUS)0xC0000044L)
#endif

#ifndef STATUS_TOO_MANY_PAGING_FILES
#   define STATUS_TOO_MANY_PAGING_FILES     ((NTSTATUS)0xC0000097L)
#endif

#ifndef STATUS_INSUFFICIENT_RESOURCES
#   define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#endif

#ifndef STATUS_WORKING_SET_QUOTA
#   define STATUS_WORKING_SET_QUOTA         ((NTSTATUS)0xC00000A1L)
#endif

#ifndef STATUS_DEVICE_NOT_READY
#   define STATUS_DEVICE_NOT_READY          ((NTSTATUS)0xC00000A3L)
#endif

#ifndef STATUS_PIPE_DISCONNECTED
#   define STATUS_PIPE_DISCONNECTED         ((NTSTATUS)0xC00000B0L)
#endif

#ifndef STATUS_IO_TIMEOUT
#   define STATUS_IO_TIMEOUT                ((NTSTATUS)0xC00000B5L)
#endif

#ifndef STATUS_NOT_SUPPORTED
#   define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#endif

#ifndef STATUS_REMOTE_NOT_LISTENING
#   define STATUS_REMOTE_NOT_LISTENING      ((NTSTATUS)0xC00000BCL)
#endif

#ifndef STATUS_BAD_NETWORK_PATH
#   define STATUS_BAD_NETWORK_PATH          ((NTSTATUS)0xC00000BEL)
#endif

#ifndef STATUS_NETWORK_BUSY
#   define STATUS_NETWORK_BUSY              ((NTSTATUS)0xC00000BFL)
#endif

#ifndef STATUS_INVALID_NETWORK_RESPONSE
#   define STATUS_INVALID_NETWORK_RESPONSE  ((NTSTATUS)0xC00000C3L)
#endif

#ifndef STATUS_UNEXPECTED_NETWORK_ERROR
#   define STATUS_UNEXPECTED_NETWORK_ERROR  ((NTSTATUS)0xC00000C4L)
#endif

#ifndef STATUS_REQUEST_NOT_ACCEPTED
#   define STATUS_REQUEST_NOT_ACCEPTED      ((NTSTATUS)0xC00000D0L)
#endif

#ifndef STATUS_CANCELLED
#   define STATUS_CANCELLED                 ((NTSTATUS)0xC0000120L)
#endif

#ifndef STATUS_COMMITMENT_LIMIT
#   define STATUS_COMMITMENT_LIMIT          ((NTSTATUS)0xC000012DL)
#endif

#ifndef STATUS_LOCAL_DISCONNECT
#   define STATUS_LOCAL_DISCONNECT          ((NTSTATUS)0xC000013BL)
#endif

#ifndef STATUS_REMOTE_DISCONNECT
#   define STATUS_REMOTE_DISCONNECT         ((NTSTATUS)0xC000013CL)
#endif

#ifndef STATUS_REMOTE_RESOURCES
#   define STATUS_REMOTE_RESOURCES          ((NTSTATUS)0xC000013DL)
#endif

#ifndef STATUS_LINK_FAILED
#   define STATUS_LINK_FAILED               ((NTSTATUS)0xC000013EL)
#endif

#ifndef STATUS_LINK_TIMEOUT
#   define STATUS_LINK_TIMEOUT              ((NTSTATUS)0xC000013FL)
#endif

#ifndef STATUS_INVALID_CONNECTION
#   define STATUS_INVALID_CONNECTION        ((NTSTATUS)0xC0000140L)
#endif

#ifndef STATUS_INVALID_ADDRESS
#   define STATUS_INVALID_ADDRESS           ((NTSTATUS)0xC0000141L)
#endif

#ifndef STATUS_INVALID_BUFFER_SIZE
#   define STATUS_INVALID_BUFFER_SIZE       ((NTSTATUS)0xC0000206L)
#endif

#ifndef STATUS_INVALID_ADDRESS_COMPONENT
#   define STATUS_INVALID_ADDRESS_COMPONENT ((NTSTATUS)0xC0000207L)
#endif

#ifndef STATUS_TOO_MANY_ADDRESSES
#   define STATUS_TOO_MANY_ADDRESSES        ((NTSTATUS)0xC0000209L)
#endif

#ifndef STATUS_ADDRESS_ALREADY_EXISTS
#   define STATUS_ADDRESS_ALREADY_EXISTS    ((NTSTATUS)0xC000020AL)
#endif

#ifndef STATUS_CONNECTION_DISCONNECTED
#   define STATUS_CONNECTION_DISCONNECTED   ((NTSTATUS)0xC000020CL)
#endif

#ifndef STATUS_CONNECTION_RESET
#   define STATUS_CONNECTION_RESET          ((NTSTATUS)0xC000020DL)
#endif

#ifndef STATUS_TRANSACTION_ABORTED
#   define STATUS_TRANSACTION_ABORTED       ((NTSTATUS)0xC000020FL)
#endif

#ifndef STATUS_CONNECTION_REFUSED
#   define STATUS_CONNECTION_REFUSED        ((NTSTATUS)0xC0000236L)
#endif

#ifndef STATUS_GRACEFUL_DISCONNECT
#   define STATUS_GRACEFUL_DISCONNECT       ((NTSTATUS)0xC0000237L)
#endif

#ifndef STATUS_NETWORK_UNREACHABLE
#   define STATUS_NETWORK_UNREACHABLE       ((NTSTATUS)0xC000023CL)
#endif

#ifndef STATUS_HOST_UNREACHABLE
#   define STATUS_HOST_UNREACHABLE          ((NTSTATUS)0xC000023DL)
#endif

#ifndef STATUS_PROTOCOL_UNREACHABLE
#   define STATUS_PROTOCOL_UNREACHABLE      ((NTSTATUS)0xC000023EL)
#endif

#ifndef STATUS_PORT_UNREACHABLE
#   define STATUS_PORT_UNREACHABLE          ((NTSTATUS)0xC000023FL)
#endif

#ifndef STATUS_REQUEST_ABORTED
#   define STATUS_REQUEST_ABORTED           ((NTSTATUS)0xC0000240L)
#endif

#ifndef STATUS_CONNECTION_ABORTED
#   define STATUS_CONNECTION_ABORTED        ((NTSTATUS)0xC0000241L)
#endif

#ifndef STATUS_HOPLIMIT_EXCEEDED
#   define STATUS_HOPLIMIT_EXCEEDED         ((NTSTATUS)0xC000A012L)
#endif

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileObjectIdInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileQuotaInformation,
    FileReparsePointInformation,
    FileNetworkOpenInformation,
    FileAttributeTagInformation,
    FileTrackingInformation,
    FileIdBothDirectoryInformation,
    FileIdFullDirectoryInformation,
    FileValidDataLengthInformation,
    FileShortNameInformation,
    FileIoCompletionNotificationInformation,
    FileIoStatusBlockRangeInformation,
    FileIoPriorityHintInformation,
    FileSfioReserveInformation,
    FileSfioVolumeInformation,
    FileHardLinkInformation,
    FileProcessIdsUsingFileInformation,
    FileNormalizedNameInformation,
    FileNetworkPhysicalNameInformation,
    FileIdGlobalTxDirectoryInformation,
    FileIsRemoteDeviceInformation,
    FileAttributeCacheInformation,
    FileNumaNodeInformation,
    FileStandardLinkInformation,
    FileRemoteProtocolInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, * PFILE_INFORMATION_CLASS;

typedef enum _FS_INFORMATION_CLASS {
    FileFsVolumeInformation = 1,
    FileFsLabelInformation = 2,
    FileFsSizeInformation = 3,
    FileFsDeviceInformation = 4,
    FileFsAttributeInformation = 5,
    FileFsControlInformation = 6,
    FileFsFullSizeInformation = 7,
    FileFsObjectIdInformation = 8,
    FileFsDriverPathInformation = 9,
    FileFsVolumeFlagsInformation = 10,
    FileFsSectorSizeInformation = 11
} FS_INFORMATION_CLASS, * PFS_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK {
#pragma warning(push)
#pragma warning(disable: 4201) // we'll always use the Microsoft compiler
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;
#pragma warning(pop)

    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    DWORD FileAttributes;
} FILE_BASIC_INFORMATION, * PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG         NumberOfLinks;
    BOOLEAN       DeletePending;
    BOOLEAN       Directory;
} FILE_STANDARD_INFORMATION, * PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, * PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
    ULONG EaSize;
} FILE_EA_INFORMATION, * PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, * PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {
    LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, * PFILE_POSITION_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, * PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {
    ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, * PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, * PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
    FILE_BASIC_INFORMATION     BasicInformation;
    FILE_STANDARD_INFORMATION  StandardInformation;
    FILE_INTERNAL_INFORMATION  InternalInformation;
    FILE_EA_INFORMATION        EaInformation;
    FILE_ACCESS_INFORMATION    AccessInformation;
    FILE_POSITION_INFORMATION  PositionInformation;
    FILE_MODE_INFORMATION      ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION      NameInformation;
} FILE_ALL_INFORMATION, * PFILE_ALL_INFORMATION;

typedef struct _FILE_FS_VOLUME_INFORMATION {
    LARGE_INTEGER VolumeCreationTime;
    ULONG         VolumeSerialNumber;
    ULONG         VolumeLabelLength;
    BOOLEAN       SupportsObjects;
    WCHAR         VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, * PFILE_FS_VOLUME_INFORMATION;

/**
 * MinGW already has a definition for REPARSE_DATA_BUFFER, but mingw-w64 does
 * not.
 */
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
#pragma warning(push)
#pragma warning(disable : 4201)
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
        struct {
            ULONG StringCount;
            WCHAR StringList[1];
        } AppExecLinkReparseBuffer;
    };
#pragma warning(pop)
} REPARSE_DATA_BUFFER, * PREPARSE_DATA_BUFFER;
#endif

/**
 * @brief The NtQueryInformationFile routine returns various kinds of information about a file object.
 * @see https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntqueryinformationfile
 */
typedef NTSTATUS (NTAPI* fn_NtQueryInformationFile)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,ULONG Length,FILE_INFORMATION_CLASS FileInformationClass);

/**
 * @brief Converts the specified NTSTATUS code to its equivalent system error code.
 * @see https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-rtlntstatustodoserror
 */
typedef ULONG (NTAPI* fn_RtlNtStatusToDosError)(NTSTATUS Status);

/**
 * @brief Retrieves information about the volume associated with a given file, directory, storage device, or volume.
 * @see https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntqueryvolumeinformationfile
 */
typedef NTSTATUS(NTAPI* fn_NtQueryVolumeInformationFile)(HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock, PVOID FsInformation, ULONG Length,
    FS_INFORMATION_CLASS FsInformationClass);

/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho2
 */
typedef VOID(NTAPI* PIO_APC_ROUTINE)(PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG Reserved);

/**
 * @see https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntdeviceiocontrolfile
 */
typedef NTSTATUS(NTAPI* fn_NtDeviceIoControlFile)(HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG IoControlCode,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength);

typedef struct ev_winapi_s
{
    fn_NtQueryInformationFile       NtQueryInformationFile;
    fn_RtlNtStatusToDosError        RtlNtStatusToDosError;
    fn_NtQueryVolumeInformationFile NtQueryVolumeInformationFile;
    fn_NtDeviceIoControlFile        NtDeviceIoControlFile;
}ev_winapi_t;

/**
 * @brief Windows API.
 */
extern ev_winapi_t ev_winapi;

/**
 * @brief Initialize WinAPI
 */
EV_LOCAL void ev__winapi_init(void);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/win/winsock.h"
#ifndef __EV_WINSOCK_INTERNAL_H__
#define __EV_WINSOCK_INTERNAL_H__

/* AMALGAMATE: #include "winapi.h" */

#define AFD_OVERLAPPED						0x00000002
#define AFD_RECEIVE							5
#define AFD_RECEIVE_DATAGRAM				6

#ifndef TDI_RECEIVE_NORMAL
#   define TDI_RECEIVE_PARTIAL              0x00000010
#   define TDI_RECEIVE_NORMAL               0x00000020
#   define TDI_RECEIVE_PEEK                 0x00000080
#endif

#define FSCTL_AFD_BASE						FILE_DEVICE_NETWORK

#define _AFD_CONTROL_CODE(operation, method) \
    ((FSCTL_AFD_BASE) << 12 | (operation << 2) | method)

#define IOCTL_AFD_RECEIVE \
    _AFD_CONTROL_CODE(AFD_RECEIVE, METHOD_NEITHER)

#define IOCTL_AFD_RECEIVE_DATAGRAM \
    _AFD_CONTROL_CODE(AFD_RECEIVE_DATAGRAM, METHOD_NEITHER)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AFD_RECV_DATAGRAM_INFO {
	LPWSABUF BufferArray;
	ULONG BufferCount;
	ULONG AfdFlags;
	ULONG TdiFlags;
	struct sockaddr* Address;
	int* AddressLength;
} AFD_RECV_DATAGRAM_INFO, * PAFD_RECV_DATAGRAM_INFO;

typedef struct _AFD_RECV_INFO {
	LPWSABUF BufferArray;
	ULONG BufferCount;
	ULONG AfdFlags;
	ULONG TdiFlags;
} AFD_RECV_INFO, * PAFD_RECV_INFO;

extern int ev_tcp_non_ifs_lsp_ipv4;
extern int ev_tcp_non_ifs_lsp_ipv6;

extern struct sockaddr_in ev_addr_ip4_any_;
extern struct sockaddr_in6 ev_addr_ip6_any_;

EV_LOCAL int WSAAPI ev__wsa_recv_workaround(SOCKET socket, WSABUF* buffers,
	DWORD buffer_count, DWORD* bytes, DWORD* flags, WSAOVERLAPPED* overlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

/**
 * @brief 
 */
EV_LOCAL int WSAAPI ev__wsa_recvfrom_workaround(SOCKET socket, WSABUF* buffers,
	DWORD buffer_count, DWORD* bytes, DWORD* flags, struct sockaddr* addr,
	int* addr_len, WSAOVERLAPPED* overlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

/**
 * @brief Initialize winsock.
 */
EV_LOCAL void ev__winsock_init(void);

/**
 * @brief Convert typeof NTSTATUS error to typeof WinSock error
 * @param[in] status  NTSTATUS error
 * @return WinSock error
 */
EV_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/win/async_win.h"
#ifndef __EV_ASYNC_WIN_INTERNAL_H__
#define __EV_ASYNC_WIN_INTERNAL_H__

/* AMALGAMATE: #include "async.h" */

#endif

#line 1 "src/win/fs_win.h"
#ifndef __EV_FS_WIN_INTERNAL_H__
#define __EV_FS_WIN_INTERNAL_H__

/* AMALGAMATE: #include "fs.h" */

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

#line 1 "src/win/udp_win.h"
#ifndef __EV_UDP_WIN_INTERNAL_H__
#define __EV_UDP_WIN_INTERNAL_H__

/* AMALGAMATE: #include "udp.h" */

#endif

#line 1 "src/win/loop_win.h"
#ifndef __EV_LOOP_WIN_INTERNAL_H__
#define __EV_LOOP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "ev.h" */

#define EV_INVALID_PID_WIN  0

typedef struct ev_loop_win_ctx
{
    /**
     * Frequency of the high-resolution clock.
     */
    uint64_t                    hrtime_frequency_;

    struct
    {
        char                    zero_[1];                   /**< A zero length buffer */
    } net;

    struct
    {
        ev_tls_t                thread_key;                 /**< Thread handle */
    }thread;
} ev_loop_win_ctx_t;

extern ev_loop_win_ctx_t        g_ev_loop_win_ctx;          /**< Global runtime for Windows */

/**
 * @brief Initialize windows context.
 */
EV_LOCAL void ev__init_once_win(void);

/**
 * @brief Initialize IOCP request
 * @param[out] req      A pointer to the IOCP request
 * @param[in] callback  A callback when the request is finish
 * @param[in] arg       User defined argument passed to callback
 */
EV_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg);

/**
 * @brief Post to specific IOCP request.
 * @param[in] loop      Event loop
 * @param[in] req       IOCP request
 */
EV_LOCAL void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req);

/**
 * @brief Set \p sock as reusable address.
 * @param[in] sock  Socket to set reusable.
 * @param[in] opt   0 if not reusable, otherwise reusable.
 * @return          #ev_errnot_t
 */
EV_LOCAL int ev__reuse_win(SOCKET sock, int opt);

/**
 * @brief Set \p sock as IPv6 only.
 * @param[in] sock  Socket to set IPv6 only.
 * @param[in] opt   0 if IPv4 available, otherwise IPv6 only.
 * @return          #ev_errnot_t
 */
EV_LOCAL int ev__ipv6only_win(SOCKET sock, int opt);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/win/process_win.h"
#ifndef __EV_PROCESS_WIN_INTERNAL_H__
#define __EV_PROCESS_WIN_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */

#endif

#line 1 "src/win/pipe_win.h"
#ifndef __EV_PIPE_WIN_INTERNAL_H__
#define __EV_PIPE_WIN_INTERNAL_H__

/* AMALGAMATE: #include "pipe.h" */

#endif

#line 1 "src/win/misc_win.h"
#ifndef __EV_MISC_WIN_INTERNAL_H__
#define __EV_MISC_WIN_INTERNAL_H__

/* AMALGAMATE: #include "misc.h" */

#define EV_FATAL_SYSCALL(errcode, syscall)	\
	ev__fatal_syscall(__FILE__, __LINE__, errcode, syscall)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maps a character string to a UTF-16 (wide character) string.
 * @param[out] dst  Pointer to store wide string. Use #ev_free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
EV_LOCAL ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src);

/**
 * @brief Maps a UTF-16 (wide character) string to a character string.
 * @param[out] dst  Pointer to store wide string. Use #ev_free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
EV_LOCAL ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src);

EV_LOCAL void ev__fatal_syscall(const char* file, int line,
	DWORD errcode, const char* syscall);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/win/thread_win.h"
#ifndef __EV_THREAD_WIN_INTERNAL_H__
#define __EV_THREAD_WIN_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize thread context.
 */
EV_LOCAL void ev__thread_init_win(void);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/win/threadpool_win.h"
#ifndef __EV_THREADPOOL_WIN_INTERNAL_H__
#define __EV_THREADPOOL_WIN_INTERNAL_H__

/* AMALGAMATE: #include "threadpool.h" */

#ifdef __cplusplus
extern "C" {
#endif

EV_LOCAL void ev__threadpool_init_win(ev_loop_t* loop);
EV_LOCAL void ev__threadpool_exit_win(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/win/tcp_win.h"
#ifndef __EV_TCP_WIN_INTERNAL_H__
#define __EV_TCP_WIN_INTERNAL_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open fd for read/write.
 * @param[in] tcp   TCP handle
 * @param[in] fd    fd
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__tcp_open_win(ev_tcp_t* tcp, SOCKET fd);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/win/async_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "async_win.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "loop_win.h" */
#include <assert.h>

static void _async_on_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(iocp, ev_async_t, backend.io);

    handle->backend.async_sent = 0;
    handle->active_cb(handle);
}

static void _ev_async_on_close_win(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);

    if (async->close_cb != NULL)
    {
        async->close_cb(async);
    }
}

static void _ev_asyc_exit_win(ev_async_t* handle, ev_async_cb close_cb)
{
    handle->close_cb = close_cb;
    ev__handle_event_dec(&handle->base);
    ev__handle_exit(&handle->base, close_cb != NULL ? _ev_async_on_close_win : NULL);
}

EV_LOCAL void ev__async_exit_force(ev_async_t* handle)
{
    _ev_asyc_exit_win(handle, NULL);
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    handle->active_cb = cb;
    handle->close_cb = NULL;
    handle->backend.async_sent = 0;

    ev__iocp_init(&handle->backend.io, _async_on_iocp_win, NULL);
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC);
    ev__handle_event_add(&handle->base);

    return 0;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    _ev_asyc_exit_win(handle, close_cb);
}

void ev_async_wakeup(ev_async_t* handle)
{
    if (!InterlockedOr(&handle->backend.async_sent, 1))
    {
        ev__iocp_post(handle->base.loop, &handle->backend.io);
    }
}

#line 1 "src/win/fs_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "winapi.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "fs_win.h" */
#include <assert.h>
#include <stdio.h>

#define MILLION ((int64_t) 1000 * 1000)
#define BILLION ((int64_t) 1000 * 1000 * 1000)
#define NSEC_IN_SEC (1 * 1000 * 1000 * 1000)

#ifndef S_IFLNK
#   define S_IFLNK 0xA000
#endif

typedef struct file_open_info_win_s
{
    DWORD   access;
    DWORD   share;
    DWORD   attributes;
    DWORD   disposition;
} file_open_info_win_t;

typedef struct fs_readdir_win_helper
{
    ev_fs_readdir_cb    cb;
    void*               arg;
    int                 errcode;
} fs_readdir_win_helper_t;

static int _ev_fs_wide_to_utf8(WCHAR* w_source_ptr, DWORD w_source_len,
    char** target_ptr, uint64_t* target_len_ptr)
{
    int r;
    int target_len;
    char* target;
    target_len = WideCharToMultiByte(CP_UTF8,
        0,
        w_source_ptr,
        w_source_len,
        NULL,
        0,
        NULL,
        NULL);

    if (target_len == 0)
    {
        return -1;
    }

    if (target_len_ptr != NULL)
    {
        *target_len_ptr = target_len;
    }

    if (target_ptr == NULL)
    {
        return 0;
    }

    target = ev_malloc(target_len + 1);
    if (target == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    }

    r = WideCharToMultiByte(CP_UTF8,
        0,
        w_source_ptr,
        w_source_len,
        target,
        target_len,
        NULL,
        NULL);
    assert(r == target_len);
    target[target_len] = '\0';
    *target_ptr = target;
    return 0;
}

static int _ev_file_get_open_attributes(int flags, int mode, file_open_info_win_t* info)
{
    info->access = 0;
    info->share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    info->attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS;
    info->disposition = 0;

    switch (flags & (EV_FS_O_RDONLY | EV_FS_O_WRONLY | EV_FS_O_RDWR))
    {
    case EV_FS_O_RDONLY:
        info->access = FILE_GENERIC_READ;
        break;
    case EV_FS_O_WRONLY:
        info->access = FILE_GENERIC_WRITE;
        break;
    case EV_FS_O_RDWR:
        info->access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
        break;
    default:
        return EV_EINVAL;
    }

    if (flags & EV_FS_O_APPEND)
    {
        info->access &= ~FILE_WRITE_DATA;
        info->access |= FILE_APPEND_DATA;
    }

    switch (flags & (EV_FS_O_CREAT | EV_FS_O_EXCL | EV_FS_O_TRUNC))
    {
    case 0:
    case EV_FS_O_EXCL:
        info->disposition = OPEN_EXISTING;
        break;
    case EV_FS_O_CREAT:
        info->disposition = OPEN_ALWAYS;
        break;
    case EV_FS_O_CREAT | EV_FS_O_EXCL:
    case EV_FS_O_CREAT | EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = CREATE_NEW;
        break;
    case EV_FS_O_TRUNC:
    case EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = TRUNCATE_EXISTING;
        break;
    case EV_FS_O_CREAT | EV_FS_O_TRUNC:
        info->disposition = CREATE_ALWAYS;
        break;
    default:
        return EV_EINVAL;
    }

    if ((flags & EV_FS_O_CREAT) && !(mode & _S_IWRITE))
    {
        info->attributes |= FILE_ATTRIBUTE_READONLY;
    }

    switch (flags & (EV_FS_O_DSYNC | EV_FS_O_SYNC))
    {
    case 0:
        break;
    case EV_FS_O_DSYNC:
#if EV_FS_O_SYNC != EV_FS_O_DSYNC
    case EV_FS_O_SYNC:
#endif
        info->attributes |= FILE_FLAG_WRITE_THROUGH;
        break;
    default:
        return EV_EINVAL;
    }

    return 0;
}

static int _ev_fs_readlink_handle(HANDLE handle, char** target_ptr,
    uint64_t* target_len_ptr)
{
    char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    REPARSE_DATA_BUFFER* reparse_data = (REPARSE_DATA_BUFFER*)buffer;
    WCHAR* w_target;
    DWORD w_target_len;
    DWORD bytes;
    size_t i;
    size_t len;

    if (!DeviceIoControl(handle,
        FSCTL_GET_REPARSE_POINT,
        NULL,
        0,
        buffer,
        sizeof buffer,
        &bytes,
        NULL)) {
        return -1;
    }

    if (reparse_data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
        /* Real symlink */
        w_target = reparse_data->SymbolicLinkReparseBuffer.PathBuffer +
            (reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset /
                sizeof(WCHAR));
        w_target_len =
            reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength /
            sizeof(WCHAR);

        /* Real symlinks can contain pretty much everything, but the only thing we
         * really care about is undoing the implicit conversion to an NT namespaced
         * path that CreateSymbolicLink will perform on absolute paths. If the path
         * is win32-namespaced then the user must have explicitly made it so, and
         * we better just return the unmodified reparse data. */
        if (w_target_len >= 4 &&
            w_target[0] == L'\\' &&
            w_target[1] == L'?' &&
            w_target[2] == L'?' &&
            w_target[3] == L'\\') {
            /* Starts with \??\ */
            if (w_target_len >= 6 &&
                ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
                    (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
                w_target[5] == L':' &&
                (w_target_len == 6 || w_target[6] == L'\\')) {
                /* \??\<drive>:\ */
                w_target += 4;
                w_target_len -= 4;

            }
            else if (w_target_len >= 8 &&
                (w_target[4] == L'U' || w_target[4] == L'u') &&
                (w_target[5] == L'N' || w_target[5] == L'n') &&
                (w_target[6] == L'C' || w_target[6] == L'c') &&
                w_target[7] == L'\\') {
                /* \??\UNC\<server>\<share>\ - make sure the final path looks like
                 * \\<server>\<share>\ */
                w_target += 6;
                w_target[0] = L'\\';
                w_target_len -= 6;
            }
        }

    }
    else if (reparse_data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
        /* Junction. */
        w_target = reparse_data->MountPointReparseBuffer.PathBuffer +
            (reparse_data->MountPointReparseBuffer.SubstituteNameOffset /
                sizeof(WCHAR));
        w_target_len = reparse_data->MountPointReparseBuffer.SubstituteNameLength /
            sizeof(WCHAR);

        /* Only treat junctions that look like \??\<drive>:\ as symlink. Junctions
         * can also be used as mount points, like \??\Volume{<guid>}, but that's
         * confusing for programs since they wouldn't be able to actually
         * understand such a path when returned by uv_readlink(). UNC paths are
         * never valid for junctions so we don't care about them. */
        if (!(w_target_len >= 6 &&
            w_target[0] == L'\\' &&
            w_target[1] == L'?' &&
            w_target[2] == L'?' &&
            w_target[3] == L'\\' &&
            ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
                (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
            w_target[5] == L':' &&
            (w_target_len == 6 || w_target[6] == L'\\'))) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }

        /* Remove leading \??\ */
        w_target += 4;
        w_target_len -= 4;

    }
    else if (reparse_data->ReparseTag == IO_REPARSE_TAG_APPEXECLINK) {
        /* String #3 in the list has the target filename. */
        if (reparse_data->AppExecLinkReparseBuffer.StringCount < 3) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }
        w_target = reparse_data->AppExecLinkReparseBuffer.StringList;
        /* The StringList buffer contains a list of strings separated by "\0",   */
        /* with "\0\0" terminating the list. Move to the 3rd string in the list: */
        for (i = 0; i < 2; ++i) {
            len = wcslen(w_target);
            if (len == 0) {
                SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
                return -1;
            }
            w_target += len + 1;
        }
        w_target_len = (DWORD)wcslen(w_target);
        if (w_target_len == 0) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }
        /* Make sure it is an absolute path. */
        if (!(w_target_len >= 3 &&
            ((w_target[0] >= L'a' && w_target[0] <= L'z') ||
                (w_target[0] >= L'A' && w_target[0] <= L'Z')) &&
            w_target[1] == L':' &&
            w_target[2] == L'\\')) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }

    }
    else {
        /* Reparse tag does not indicate a symlink. */
        SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
        return -1;
    }

    ssize_t utf8_len = ev__wide_to_utf8(target_ptr, w_target);
    if (utf8_len < 0)
    {
        return -1;
    }

    *target_len_ptr = utf8_len;
    return 0;
}

static void _ev_filetime_to_timespec(ev_timespec_t* ts, int64_t filetime)
{
    filetime -= 116444736 * BILLION;
    ts->tv_sec = (long)(filetime / (10 * MILLION));
    ts->tv_nsec = (long)((filetime - ts->tv_sec * 10 * MILLION) * 100U);
    if (ts->tv_nsec < 0)
    {
        ts->tv_sec -= 1;
        ts->tv_nsec += NSEC_IN_SEC;
    }
}

static int _ev_file_wrap_fstat_win(HANDLE handle, ev_fs_stat_t* statbuf, int do_lstat)
{
    FILE_ALL_INFORMATION file_info;
    FILE_FS_VOLUME_INFORMATION volume_info;
    NTSTATUS nt_status;
    IO_STATUS_BLOCK io_status;

    nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status,
        &file_info,
        sizeof(file_info),
        FileAllInformation);

    /* Buffer overflow (a warning status code) is expected here. */
    if (NT_ERROR(nt_status))
    {
        SetLastError(ev_winapi.RtlNtStatusToDosError(nt_status));
        return -1;
    }

    nt_status = ev_winapi.NtQueryVolumeInformationFile(handle,
        &io_status,
        &volume_info,
        sizeof volume_info,
        FileFsVolumeInformation);

    /* Buffer overflow (a warning status code) is expected here. */
    if (io_status.Status == STATUS_NOT_IMPLEMENTED)
    {
        statbuf->st_dev = 0;
    }
    else if (NT_ERROR(nt_status))
    {
        SetLastError(ev_winapi.RtlNtStatusToDosError(nt_status));
        return -1;
    }
    else
    {
        statbuf->st_dev = volume_info.VolumeSerialNumber;
    }

    /* Todo: st_mode should probably always be 0666 for everyone. We might also
     * want to report 0777 if the file is a .exe or a directory.
     *
     * Currently it's based on whether the 'readonly' attribute is set, which
     * makes little sense because the semantics are so different: the 'read-only'
     * flag is just a way for a user to protect against accidental deletion, and
     * serves no security purpose. Windows uses ACLs for that.
     *
     * Also people now use uv_fs_chmod() to take away the writable bit for good
     * reasons. Windows however just makes the file read-only, which makes it
     * impossible to delete the file afterwards, since read-only files can't be
     * deleted.
     *
     * IOW it's all just a clusterfuck and we should think of something that
     * makes slightly more sense.
     *
     * And uv_fs_chmod should probably just fail on windows or be a total no-op.
     * There's nothing sensible it can do anyway.
     */
    statbuf->st_mode = 0;

    /*
    * On Windows, FILE_ATTRIBUTE_REPARSE_POINT is a general purpose mechanism
    * by which filesystem drivers can intercept and alter file system requests.
    *
    * The only reparse points we care about are symlinks and mount points, both
    * of which are treated as POSIX symlinks. Further, we only care when
    * invoked via lstat, which seeks information about the link instead of its
    * target. Otherwise, reparse points must be treated as regular files.
    */
    if (do_lstat &&
        (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    {
        /*
         * If reading the link fails, the reparse point is not a symlink and needs
         * to be treated as a regular file. The higher level lstat function will
         * detect this failure and retry without do_lstat if appropriate.
         */
        if (_ev_fs_readlink_handle(handle, NULL, &statbuf->st_size) != 0)
            return -1;
        statbuf->st_mode |= S_IFLNK;
    }

    if (statbuf->st_mode == 0) {
        if (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            statbuf->st_mode |= _S_IFDIR;
            statbuf->st_size = 0;
        }
        else {
            statbuf->st_mode |= _S_IFREG;
            statbuf->st_size = file_info.StandardInformation.EndOfFile.QuadPart;
        }
    }

    if (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY)
        statbuf->st_mode |= _S_IREAD | (_S_IREAD >> 3) | (_S_IREAD >> 6);
    else
        statbuf->st_mode |= (_S_IREAD | _S_IWRITE) | ((_S_IREAD | _S_IWRITE) >> 3) |
        ((_S_IREAD | _S_IWRITE) >> 6);

    _ev_filetime_to_timespec(&statbuf->st_atim,
        file_info.BasicInformation.LastAccessTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_ctim,
        file_info.BasicInformation.ChangeTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_mtim,
        file_info.BasicInformation.LastWriteTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_birthtim,
        file_info.BasicInformation.CreationTime.QuadPart);

    statbuf->st_ino = file_info.InternalInformation.IndexNumber.QuadPart;

    /* st_blocks contains the on-disk allocation size in 512-byte units. */
    statbuf->st_blocks =
        (uint64_t)file_info.StandardInformation.AllocationSize.QuadPart >> 9;

    statbuf->st_nlink = file_info.StandardInformation.NumberOfLinks;

    /* The st_blksize is supposed to be the 'optimal' number of bytes for reading
     * and writing to the disk. That is, for any definition of 'optimal' - it's
     * supposed to at least avoid read-update-write behavior when writing to the
     * disk.
     *
     * However nobody knows this and even fewer people actually use this value,
     * and in order to fill it out we'd have to make another syscall to query the
     * volume for FILE_FS_SECTOR_SIZE_INFORMATION.
     *
     * Therefore we'll just report a sensible value that's quite commonly okay
     * on modern hardware.
     *
     * 4096 is the minimum required to be compatible with newer Advanced Format
     * drives (which have 4096 bytes per physical sector), and to be backwards
     * compatible with older drives (which have 512 bytes per physical sector).
     */
    statbuf->st_blksize = 4096;

    /* Todo: set st_flags to something meaningful. Also provide a wrapper for
     * chattr(2).
     */
    statbuf->st_flags = 0;

    /* Windows has nothing sensible to say about these values, so they'll just
     * remain empty.
     */
    statbuf->st_gid = 0;
    statbuf->st_uid = 0;
    statbuf->st_rdev = 0;
    statbuf->st_gen = 0;

    return 0;
}

static int _ev_file_fstat_win(HANDLE handle, ev_fs_stat_t* statbuf, int do_lstat)
{
    int errcode;
    if (_ev_file_wrap_fstat_win(handle, statbuf, do_lstat) != 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    return 0;
}

static ev_dirent_type_t _ev_fs_get_dirent_type_win(WIN32_FIND_DATAW* info)
{
    if (info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return EV_DIRENT_DIR;
    }
    if (info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        return EV_DIRENT_LINK;
    }
    if (info->dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
    {
        return EV_DIRENT_CHR;
    }
    return EV_DIRENT_FILE;
}

static int _ev_fs_wmkdir(WCHAR* path, int mode)
{
    (void)mode;

    wchar_t* p;
    wchar_t backup;
    DWORD errcode;
    const wchar_t* path_delim = L"\\/";

    for (p = wcspbrk(path, path_delim); p != NULL; p = wcspbrk(p + 1, path_delim))
    {
        backup = *p;
        *p = L'\0';
        if (!CreateDirectoryW(path, NULL))
        {
            errcode = GetLastError();
            if (errcode != ERROR_ALREADY_EXISTS)
            {
                *p = backup;
                return ev__translate_sys_error(errcode);
            }
        }
        *p = backup;
    }

    if (CreateDirectoryW(path, NULL))
    {
        return 0;
    }

    errcode = GetLastError();
    if (errcode == ERROR_INVALID_NAME || errcode == ERROR_DIRECTORY)
    {
        return EV_EINVAL;
    }

    return ev__translate_sys_error(errcode);
}

static int _ev_fs_readdir_w_on_dirent(ev_dirent_w_t* info, void* arg)
{
    fs_readdir_win_helper_t* helper = arg;

    ev_dirent_t wrap_info;
    wrap_info.type = info->type;
    ssize_t ret = ev__wide_to_utf8(&wrap_info.name, info->name);
    if (ret < 0)
    {
        helper->errcode = (int)ret;
        return -1;
    }

    ret = helper->cb(&wrap_info, helper->arg);
    ev_free(wrap_info.name);

    return (int)ret;
}

EV_LOCAL int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode)
{
    int ret;
    DWORD errcode;

    file_open_info_win_t info;
    ret = _ev_file_get_open_attributes(flags, mode, &info);
    if (ret != 0)
    {
        return ret;
    }

    HANDLE filehandle = CreateFile(path, info.access, info.share, NULL,
        info.disposition, info.attributes, NULL);
    if (filehandle == INVALID_HANDLE_VALUE)
    {
        errcode = GetLastError();
        if (errcode == ERROR_FILE_EXISTS && (flags & EV_FS_O_CREAT) && !(flags & EV_FS_O_EXCL))
        {
            return EV_EISDIR;
        }

        return ev__translate_sys_error(errcode);
    }

    *file = filehandle;
    return 0;
}

EV_LOCAL int ev__fs_seek(ev_os_file_t file, int whence, ssize_t offset)
{
    DWORD errcode;
    LONG DistanceToMove = offset & 0XFFFFFFFF;
    LONG DistanceToMoveHigh = offset >> 32;

    if (SetFilePointer(file, DistanceToMove, &DistanceToMoveHigh, whence) == INVALID_SET_FILE_POINTER)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    return 0;
}

EV_LOCAL ssize_t ev__fs_readv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    DWORD errcode;
    DWORD bytes = 0;
	if (file == INVALID_HANDLE_VALUE)
	{
		return ev__translate_sys_error(ERROR_INVALID_HANDLE);
	}

    size_t idx;
    for (idx = 0; idx < nbuf; idx++)
    {
        DWORD incremental_bytes;
        BOOL read_ret = ReadFile(file, bufs[idx].data, bufs[idx].size,
            &incremental_bytes, NULL);
        if (!read_ret)
        {
            goto error;
        }
        bytes += incremental_bytes;
    }

    return bytes;

error:
    errcode = GetLastError();
    if (errcode == ERROR_HANDLE_EOF || errcode == ERROR_BROKEN_PIPE)
    {
        return bytes;
    }

	if (errcode == ERROR_ACCESS_DENIED)
	{
        errcode = ERROR_INVALID_FLAGS;
	}
	return ev__translate_sys_error(errcode);
}

EV_LOCAL ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    if (file == INVALID_HANDLE_VALUE)
    {
        return ev__translate_sys_error(ERROR_INVALID_HANDLE);
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL read_ret = TRUE;
    for (idx = 0; read_ret && idx < nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        read_ret = ReadFile(file, bufs[idx].data, bufs[idx].size,
            &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    if (read_ret || bytes > 0)
    {
        return bytes;
    }

    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
    {
        return bytes;
    }

    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    return ev__translate_sys_error(err);
}

EV_LOCAL ssize_t ev__fs_writev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    DWORD errcode;
    DWORD bytes = 0;
	if (file == INVALID_HANDLE_VALUE)
	{
		return ev__translate_sys_error(ERROR_INVALID_HANDLE);
	}

    size_t idx;
    for (idx = 0; idx < nbuf; idx++)
    {
        DWORD incremental_bytes;
        BOOL write_ret = WriteFile(file, bufs[idx].data, bufs[idx].size,
            &incremental_bytes, NULL);
        if (!write_ret)
        {
            goto error;
        }

        bytes += incremental_bytes;
    }

    return bytes;

error:
    errcode = GetLastError();
	if (errcode == ERROR_ACCESS_DENIED)
	{
        errcode = ERROR_INVALID_FLAGS;
	}
	return ev__translate_sys_error(errcode);
}

EV_LOCAL ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    if (file == INVALID_HANDLE_VALUE)
    {
        return ev__translate_sys_error(ERROR_INVALID_HANDLE);
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL write_ret = TRUE;
    for (idx = 0; write_ret && idx < nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        write_ret = WriteFile(file, bufs[idx].data,
            bufs[idx].size, &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    if (write_ret || bytes > 0)
    {
        return bytes;
    }

    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    return ev__translate_sys_error(err);
}

EV_LOCAL int ev__fs_fstat(ev_os_file_t file, ev_fs_stat_t* statbuf)
{
    return _ev_file_fstat_win(file, statbuf, 0);
}

EV_LOCAL int ev__fs_close(ev_os_file_t file)
{
    DWORD errcode;
    if (!CloseHandle(file))
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

EV_LOCAL int ev__fs_readdir_w(const WCHAR* path, ev_readdir_w_cb cb, void* arg)
{
    WIN32_FIND_DATAW info;
    ev_dirent_w_t dirent_info;

    int ret = 0;
    HANDLE dir_handle = INVALID_HANDLE_VALUE;
    WCHAR* fixed_path = NULL;

    size_t path_len = wcslen(path);
    size_t fixed_path_len = path_len + 3;
    fixed_path = ev_malloc(sizeof(WCHAR) * fixed_path_len);
    if (fixed_path == NULL)
    {
        return EV_ENOMEM;
    }

    const WCHAR* fmt = L"%s\\*";
    if (path[path_len - 1] == L'/' || path[path_len - 1] == L'\\')
    {
        fmt = L"%s*";
    }
    _snwprintf_s(fixed_path, fixed_path_len, _TRUNCATE, fmt, path);

    dir_handle = FindFirstFileW(fixed_path, &info);
    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        ret = ev__translate_sys_error(GetLastError());
        goto cleanup;
    }

    do
    {
        if (wcscmp(info.cFileName, L".") == 0 || wcscmp(info.cFileName, L"..") == 0)
        {
            continue;
        }

        dirent_info.type = _ev_fs_get_dirent_type_win(&info);
        dirent_info.name = info.cFileName;

        if (cb(&dirent_info, arg))
        {
            break;
        }
    } while (FindNextFileW(dir_handle, &info));

cleanup:
    if (dir_handle != INVALID_HANDLE_VALUE)
    {
        FindClose(dir_handle);
    }
    if (fixed_path != NULL)
    {
        ev_free(fixed_path);
    }
    return ret;
}

EV_LOCAL int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg)
{
    WCHAR* wide_path = NULL;
    size_t wide_path_len = ev__utf8_to_wide(&wide_path, path);
    if (wide_path_len < 0)
    {
        return (int)wide_path_len;
    }

    fs_readdir_win_helper_t helper = { cb, arg, 0 };
    int ret = ev__fs_readdir_w(wide_path, _ev_fs_readdir_w_on_dirent, &helper);

    ev_free(wide_path);
    return helper.errcode != 0 ? helper.errcode : ret;
}

EV_LOCAL int ev__fs_mkdir(const char* path, int mode)
{
    WCHAR* copy_wpath;
    ssize_t ret = ev__utf8_to_wide(&copy_wpath, path);
    if (ret < 0)
    {
        return (int)ret;
    }

    ret = _ev_fs_wmkdir(copy_wpath, mode);
    ev_free(copy_wpath);

    return (int)ret;
}

#line 1 "src/win/loop_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "thread_win.h" */
/* AMALGAMATE: #include "threadpool_win.h" */
/* AMALGAMATE: #include "winsock.h" */
#include <assert.h>

ev_loop_win_ctx_t g_ev_loop_win_ctx;

static void _ev_time_init_win(void)
{
    DWORD errcode;
    LARGE_INTEGER perf_frequency;

    /* Retrieve high-resolution timer frequency
     * and precompute its reciprocal.
     */
    if (QueryPerformanceFrequency(&perf_frequency))
    {
        g_ev_loop_win_ctx.hrtime_frequency_ = perf_frequency.QuadPart;
    }
    else
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

static uint64_t _ev_hrtime_win(unsigned int scale)
{
    LARGE_INTEGER counter;
    double scaled_freq;
    double result;
    DWORD errcode;

    assert(scale != 0);
    if (!QueryPerformanceCounter(&counter))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
    assert(counter.QuadPart != 0);

    /* Because we have no guarantee about the order of magnitude of the
     * performance counter interval, integer math could cause this computation
     * to overflow. Therefore we resort to floating point math.
     */
    scaled_freq = (double)g_ev_loop_win_ctx.hrtime_frequency_ / scale;
    result = (double)counter.QuadPart / scaled_freq;
    return (uint64_t)result;
}

static void _ev_pool_win_handle_req(OVERLAPPED_ENTRY* overlappeds, ULONG count)
{
    ULONG i;
    for (i = 0; i < count; i++)
    {
        if (overlappeds[i].lpOverlapped)
        {
            ev_iocp_t* req = EV_CONTAINER_OF(overlappeds[i].lpOverlapped, ev_iocp_t, overlapped);
            req->cb(req, overlappeds[i].dwNumberOfBytesTransferred, req->arg);
        }
    }
}

static void _ev_check_layout_win(void)
{
    ENSURE_LAYOUT(ev_buf_t, size, data, WSABUF, len, buf);
}

static void _ev_init_once_win(void)
{
    g_ev_loop_win_ctx.net.zero_[0] = '\0';

    _ev_check_layout_win();
    ev__winsock_init();
    ev__winapi_init();
    _ev_time_init_win();
    ev__thread_init_win();
}

uint64_t ev_hrtime(void)
{
    return _ev_hrtime_win(1000000);
}

EV_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
    int repeat;
    BOOL success;
    ULONG count;
    DWORD errcode;
    OVERLAPPED_ENTRY overlappeds[128];

    uint64_t timeout_time = loop->hwtime + timeout;

    for (repeat = 0;; repeat++)
    {
        success = GetQueuedCompletionStatusEx(loop->backend.iocp, overlappeds,
            ARRAY_SIZE(overlappeds), &count, timeout, FALSE);

        /* If success, handle all IOCP request */
        if (success)
        {
            _ev_pool_win_handle_req(overlappeds, count);
            return;
        }

        /* Cannot handle any other error */
        errcode = GetLastError();
        if (errcode != WAIT_TIMEOUT)
        {
            EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
        }

        if (timeout == 0)
        {
            return;
        }

        /**
         * GetQueuedCompletionStatusEx() can occasionally return a little early.
         * Make sure that the desired timeout target time is reached.
         */
        ev__loop_update_time(loop);

        if (timeout_time <= loop->hwtime)
        {
            break;
        }

        timeout = (uint32_t)(timeout_time - loop->hwtime);
        timeout += repeat ? (1U << (repeat - 1)) : 0;
    }
}

EV_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg)
{
    req->cb = callback;
    req->arg = arg;
    memset(&req->overlapped, 0, sizeof(req->overlapped));
}

EV_LOCAL void ev__loop_exit_backend(ev_loop_t* loop)
{
    ev__threadpool_exit_win(loop);

    if (loop->backend.iocp != NULL)
    {
        CloseHandle(loop->backend.iocp);
        loop->backend.iocp = NULL;
    }
}

EV_LOCAL void ev__init_once_win(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_win);
}

EV_LOCAL int ev__loop_init_backend(ev_loop_t* loop)
{
    ev__init_once_win();

    loop->backend.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (loop->backend.iocp == NULL)
    {
        int err = GetLastError();
        return ev__translate_sys_error(err);
    }

    ev__threadpool_init_win(loop);

    return 0;
}

EV_LOCAL void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req)
{
    DWORD errcode;
    if (!PostQueuedCompletionStatus(loop->backend.iocp, 0, 0, &req->overlapped))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

EV_LOCAL int ev__reuse_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    int err;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen) != 0)
    {
        err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

EV_LOCAL int ev__ipv6only_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, optlen) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

#line 1 "src/win/misc_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "winapi.h" */
/* AMALGAMATE: #include "misc_win.h" */
#include <assert.h>

EV_LOCAL ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src)
{
    int errcode;
    int pathw_len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (pathw_len == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    size_t buf_sz = pathw_len * sizeof(WCHAR);
    WCHAR* buf = ev_malloc(buf_sz);
    if (buf == NULL)
    {
        return EV_ENOMEM;
    }

    int r = MultiByteToWideChar(CP_UTF8, 0, src, -1, buf, pathw_len);
    assert(r == pathw_len);

    *dst = buf;

    return r;
}

EV_LOCAL ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src)
{
    int errcode;
    int target_len = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0,
                                         NULL, NULL);
    if (target_len == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    char* buf = ev_malloc(target_len);
    if (buf == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = WideCharToMultiByte(CP_UTF8, 0, src, -1, buf, target_len, NULL,
                                  NULL);
    assert(ret == target_len);
    *dst = buf;

    return (ssize_t)ret;
}

EV_LOCAL int ev__translate_sys_error(int err)
{
    switch (err)
    {
    case 0:                                 return 0;
    case ERROR_NOACCESS:                    return EV_EACCES;
    case WSAEACCES:                         return EV_EACCES;
    case ERROR_ELEVATION_REQUIRED:          return EV_EACCES;
    case ERROR_CANT_ACCESS_FILE:            return EV_EACCES;
    case ERROR_ADDRESS_ALREADY_ASSOCIATED:  return EV_EADDRINUSE;
    case WSAEADDRINUSE:                     return EV_EADDRINUSE;
    case WSAEADDRNOTAVAIL:                  return EV_EADDRNOTAVAIL;
    case WSAEAFNOSUPPORT:                   return EV_EAFNOSUPPORT;
    case WSAEWOULDBLOCK:                    return EV_EAGAIN;
    case WSAEALREADY:                       return EV_EALREADY;
    case ERROR_INVALID_FLAGS:               return EV_EBADF;
    case ERROR_INVALID_HANDLE:              return EV_EBADF;
    case ERROR_LOCK_VIOLATION:              return EV_EBUSY;
    case ERROR_PIPE_BUSY:                   return EV_EBUSY;
    case ERROR_SHARING_VIOLATION:           return EV_EBUSY;
    case ERROR_OPERATION_ABORTED:           return EV_ECANCELED;
    case WSAEINTR:                          return EV_ECANCELED;
    case ERROR_CONNECTION_ABORTED:          return EV_ECONNABORTED;
    case WSAECONNABORTED:                   return EV_ECONNABORTED;
    case ERROR_CONNECTION_REFUSED:          return EV_ECONNREFUSED;
    case WSAECONNREFUSED:                   return EV_ECONNREFUSED;
    case ERROR_NETNAME_DELETED:             return EV_ECONNRESET;
    case WSAECONNRESET:                     return EV_ECONNRESET;
    case ERROR_ALREADY_EXISTS:              return EV_EEXIST;
    case ERROR_FILE_EXISTS:                 return EV_EEXIST;
    case ERROR_BUFFER_OVERFLOW:             return EV_EFAULT;
    case WSAEFAULT:                         return EV_EFAULT;
    case ERROR_HOST_UNREACHABLE:            return EV_EHOSTUNREACH;
    case WSAEHOSTUNREACH:                   return EV_EHOSTUNREACH;
    case ERROR_INSUFFICIENT_BUFFER:         return EV_EINVAL;
    case ERROR_INVALID_DATA:                return EV_EINVAL;
    case ERROR_INVALID_PARAMETER:           return EV_EINVAL;
    case ERROR_SYMLINK_NOT_SUPPORTED:       return EV_EINVAL;
    case WSAEINVAL:                         return EV_EINVAL;
    case WSAEPFNOSUPPORT:                   return EV_EINVAL;
    case WSAESOCKTNOSUPPORT:                return EV_EINVAL;
    case ERROR_BEGINNING_OF_MEDIA:          return EV_EIO;
    case ERROR_BUS_RESET:                   return EV_EIO;
    case ERROR_CRC:                         return EV_EIO;
    case ERROR_DEVICE_DOOR_OPEN:            return EV_EIO;
    case ERROR_DEVICE_REQUIRES_CLEANING:    return EV_EIO;
    case ERROR_DISK_CORRUPT:                return EV_EIO;
    case ERROR_EOM_OVERFLOW:                return EV_EIO;
    case ERROR_FILEMARK_DETECTED:           return EV_EIO;
    case ERROR_GEN_FAILURE:                 return EV_EIO;
    case ERROR_INVALID_BLOCK_LENGTH:        return EV_EIO;
    case ERROR_IO_DEVICE:                   return EV_EIO;
    case ERROR_NO_DATA_DETECTED:            return EV_EIO;
    case ERROR_NO_SIGNAL_SENT:              return EV_EIO;
    case ERROR_OPEN_FAILED:                 return EV_EIO;
    case ERROR_SETMARK_DETECTED:            return EV_EIO;
    case ERROR_SIGNAL_REFUSED:              return EV_EIO;
    case WSAEISCONN:                        return EV_EISCONN;
    case ERROR_CANT_RESOLVE_FILENAME:       return EV_ELOOP;
    case ERROR_TOO_MANY_OPEN_FILES:         return EV_EMFILE;
    case WSAEMFILE:                         return EV_EMFILE;
    case WSAEMSGSIZE:                       return EV_EMSGSIZE;
    case ERROR_FILENAME_EXCED_RANGE:        return EV_ENAMETOOLONG;
    case ERROR_NETWORK_UNREACHABLE:         return EV_ENETUNREACH;
    case WSAENETUNREACH:                    return EV_ENETUNREACH;
    case WSAENOBUFS:                        return EV_ENOBUFS;
    case ERROR_BAD_PATHNAME:                return EV_ENOENT;
    case ERROR_DIRECTORY:                   return EV_ENOENT;
    case ERROR_ENVVAR_NOT_FOUND:            return EV_ENOENT;
    case ERROR_FILE_NOT_FOUND:              return EV_ENOENT;
    case ERROR_INVALID_NAME:                return EV_ENOENT;
    case ERROR_INVALID_DRIVE:               return EV_ENOENT;
    case ERROR_INVALID_REPARSE_DATA:        return EV_ENOENT;
    case ERROR_MOD_NOT_FOUND:               return EV_ENOENT;
    case ERROR_PATH_NOT_FOUND:              return EV_ENOENT;
    case WSAHOST_NOT_FOUND:                 return EV_ENOENT;
    case WSANO_DATA:                        return EV_ENOENT;
    case ERROR_NOT_ENOUGH_MEMORY:           return EV_ENOMEM;
    case ERROR_OUTOFMEMORY:                 return EV_ENOMEM;
    case ERROR_CANNOT_MAKE:                 return EV_ENOSPC;
    case ERROR_DISK_FULL:                   return EV_ENOSPC;
    case ERROR_EA_TABLE_FULL:               return EV_ENOSPC;
    case ERROR_END_OF_MEDIA:                return EV_ENOSPC;
    case ERROR_HANDLE_DISK_FULL:            return EV_ENOSPC;
    case ERROR_NOT_CONNECTED:               return EV_ENOTCONN;
    case WSAENOTCONN:                       return EV_ENOTCONN;
    case ERROR_DIR_NOT_EMPTY:               return EV_ENOTEMPTY;
    case WSAENOTSOCK:                       return EV_ENOTSOCK;
    case ERROR_NOT_SUPPORTED:               return EV_ENOTSUP;
    case ERROR_BROKEN_PIPE:                 return EV_EOF;
    case ERROR_ACCESS_DENIED:               return EV_EPERM;
    case ERROR_PRIVILEGE_NOT_HELD:          return EV_EPERM;
    case ERROR_BAD_PIPE:                    return EV_EPIPE;
    case ERROR_NO_DATA:                     return EV_EPIPE;
    case ERROR_PIPE_NOT_CONNECTED:          return EV_EPIPE;
    case WSAESHUTDOWN:                      return EV_EPIPE;
    case WSAEPROTONOSUPPORT:                return EV_EPROTONOSUPPORT;
    case ERROR_WRITE_PROTECT:               return EV_EROFS;
    case ERROR_SEM_TIMEOUT:                 return EV_ETIMEDOUT;
    case WSAETIMEDOUT:                      return EV_ETIMEDOUT;
    case ERROR_NOT_SAME_DEVICE:             return EV_EXDEV;
    case ERROR_INVALID_FUNCTION:            return EV_EISDIR;
    case ERROR_META_EXPANSION_TOO_LONG:     return EV_E2BIG;
    default:                                return ev__translate_posix_sys_error(err);
    }
}

EV_LOCAL void ev__fatal_syscall(const char* file, int line,
    DWORD errcode, const char* syscall)
{
    const char* errmsg = "Unknown error";
    char* buf = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);
    if (buf)
    {
        errmsg = buf;
    }

    if (syscall != NULL)
    {
        fprintf(stderr, "%s:%d: [%s] %s(%d)\n", file, line, syscall, errmsg, (int)errcode);
    }
    else
    {
        fprintf(stderr, "%s:%d: %s(%d)\n", file, line, errmsg, (int)errcode);
    }

    if (buf)
    {
        LocalFree(buf);
        buf = NULL;
    }

    __debugbreak();
    abort();
}

void ev_library_shutdown(void)
{

}

#line 1 "src/win/mutex_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */

void ev_mutex_init(ev_mutex_t* handle, int recursive)
{
    (void)recursive;
    InitializeCriticalSection(&handle->u.r);
}

void ev_mutex_exit(ev_mutex_t* handle)
{
    DeleteCriticalSection(&handle->u.r);
}

void ev_mutex_enter(ev_mutex_t* handle)
{
    EnterCriticalSection(&handle->u.r);
}

void ev_mutex_leave(ev_mutex_t* handle)
{
    LeaveCriticalSection(&handle->u.r);
}

int ev_mutex_try_enter(ev_mutex_t* handle)
{
    if (TryEnterCriticalSection(&handle->u.r))
    {
        return 0;
    }

    return EV_EBUSY;
}

#line 1 "src/win/once_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_win.h" */

static BOOL WINAPI _ev_once_proxy(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context)
{
    (void)InitOnce; (void)Context;

    ((ev_once_cb)Parameter)();
    return TRUE;
}

void ev_once_execute(ev_once_t* guard, ev_once_cb cb)
{
    DWORD errcode;
    if (InitOnceExecuteOnce(&guard->guard, _ev_once_proxy, (PVOID)cb, NULL) == 0)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
    }
}

#line 1 "src/win/pipe_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "winsock.h" */
/* AMALGAMATE: #include "tcp_win.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "pipe_win.h" */
#include <stdio.h>
#include <assert.h>

static char s_ev_zero[] = "";

static int _ev_pipe_make_s(HANDLE* pip_handle, const char* name, int flags)
{
    DWORD r_open_mode = WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE;
    r_open_mode |= (flags & EV_PIPE_READABLE) ? PIPE_ACCESS_INBOUND : 0;
    r_open_mode |= (flags & EV_PIPE_WRITABLE) ? PIPE_ACCESS_OUTBOUND : 0;
    r_open_mode |= (flags & EV_PIPE_NONBLOCK) ? FILE_FLAG_OVERLAPPED : 0;

    HANDLE pip_r = CreateNamedPipeA(name, r_open_mode,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65535, 65535, 0, NULL);
    if (pip_r != INVALID_HANDLE_VALUE)
    {
        *pip_handle = pip_r;
        return 0;
    }

    DWORD errcode = GetLastError();
    return ev__translate_sys_error(errcode);
}

static int _ev_pipe_make_c(HANDLE* pipe_handle, const char* name, int flags)
{
    DWORD w_open_mode = WRITE_DAC;
    w_open_mode |= (flags & EV_PIPE_READABLE) ? GENERIC_READ : FILE_READ_ATTRIBUTES;
    w_open_mode |= (flags & EV_PIPE_WRITABLE) ? GENERIC_WRITE : FILE_WRITE_ATTRIBUTES;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = 0;

    DWORD dwFlagsAndAttributes = (flags & EV_PIPE_NONBLOCK) ? FILE_FLAG_OVERLAPPED : 0;
    HANDLE pip_w = CreateFile(name, w_open_mode, 0, &sa, OPEN_EXISTING, dwFlagsAndAttributes, NULL);

    if (pip_w != INVALID_HANDLE_VALUE)
    {
        *pipe_handle = pip_w;
        return 0;
    }

    DWORD errcode = GetLastError();
    return ev__translate_sys_error(errcode);
}

static void _ev_pipe_r_user_callback_win(ev_pipe_read_req_t* req, ssize_t size)
{
    ev_pipe_t* pipe = req->backend.owner;
    ev__handle_event_dec(&pipe->base);

    ev__read_exit(&req->base);
    req->ucb(req, size);
}

static void _ev_pipe_cancel_all_r_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_read_req_t* req;
    if ((req = pipe->backend.ipc_mode.rio.reading.reading) != NULL)
    {
        _ev_pipe_r_user_callback_win(req, stat);
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
    }
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_win(req, stat);
    }
}

static void _ev_pipe_cancel_all_r_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_read_req_t* req;
    if ((req = pipe->backend.data_mode.rio.r_doing) != NULL)
    {
        _ev_pipe_r_user_callback_win(req, stat);
        pipe->backend.data_mode.rio.r_doing = NULL;
    }

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_win(req, stat);
    }
}

static void _ev_pipe_cancel_all_r(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_cancel_all_r_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_r_data_mode(pipe, stat);
    }
}

static void _ev_pipe_w_user_callback_win(ev_pipe_write_req_t* req, ssize_t size)
{
    ev_pipe_t* pipe = req->backend.owner;
    ev__handle_event_dec(&pipe->base);

    ev__write_exit(&req->base);
    req->ucb(req, size);
}

static void _ev_pipe_cancel_all_w_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_write_req_t* req;
    if ((req = pipe->backend.data_mode.wio.w_half) != NULL)
    {
        _ev_pipe_w_user_callback_win(req, stat);
        pipe->backend.data_mode.wio.w_half = NULL;
    }
    pipe->backend.data_mode.wio.w_half_idx = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.wio.w_pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_win(req, stat);
    }
}

static void _ev_pipe_cancel_all_w_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_write_req_t* req;
    if ((req = pipe->backend.ipc_mode.wio.sending.w_req) != NULL)
    {
        _ev_pipe_w_user_callback_win(req, stat);
        pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    }
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_win(req, stat);
    }
}

static void _ev_pipe_cancel_all_w(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_cancel_all_w_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_w_data_mode(pipe, stat);
    }
}

static void _ev_pipe_close_pipe(ev_pipe_t* pipe)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        CloseHandle(pipe->pipfd);
        pipe->pipfd = EV_OS_PIPE_INVALID;
    }
}

/**
 * @brief Abort all pending task and close pipe.
 * The pipe is no longer usable.
 */
static void _ev_pipe_abort(ev_pipe_t* pipe, int stat)
{
    _ev_pipe_close_pipe(pipe);

    _ev_pipe_cancel_all_r(pipe, stat);
    _ev_pipe_cancel_all_w(pipe, stat);
}

static void _ev_pipe_on_close_win(ev_handle_t* handle)
{
    ev_pipe_t* pipe = EV_CONTAINER_OF(handle, ev_pipe_t, base);

    _ev_pipe_abort(pipe, EV_ECANCELED);

    if (pipe->close_cb != NULL)
    {
        pipe->close_cb(pipe);
    }
}

static int _ev_pipe_read_into_req(HANDLE file, ev_pipe_read_req_t* req, size_t minimum_size,
    size_t bufidx, size_t bufpos, size_t* dbufidx, size_t* dbufpos, size_t* total)
{
    int ret = 0;
    size_t total_size = 0;

    while (bufidx < req->base.data.nbuf && total_size < minimum_size)
    {
        ev_buf_t* buf = &req->base.data.bufs[bufidx];
        void* buffer = (uint8_t*)buf->data + bufpos;
        size_t buffersize = buf->size - bufpos;

        DWORD read_size;
        if (!ReadFile(file, buffer, (DWORD)buffersize, &read_size, NULL))
        {
            int err = GetLastError();
            ret = ev__translate_sys_error(err);
            break;
        }

        total_size += read_size;
        if (read_size < buffersize)
        {
            bufpos += read_size;
            continue;
        }

        bufidx++;
        bufpos = 0;
    }

    if (dbufidx != NULL)
    {
        *dbufidx = bufidx;
    }
    if (dbufpos != NULL)
    {
        *dbufpos = bufpos;
    }
    if (total != NULL)
    {
        *total = total_size;
    }
    return ret;
}

static int _ev_pipe_data_mode_want_read(ev_pipe_t* pipe)
{
    int ret = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL,
        &pipe->backend.data_mode.rio.io.overlapped);

    if (!ret)
    {
        DWORD err = GetLastError();
        if (err != ERROR_IO_PENDING)
        {
            return ev__translate_sys_error(err);
        }
    }

    return 0;
}

static void _ev_pipe_data_mode_callback_and_mount_next_win(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending);
    if (it == NULL)
    {
        pipe->backend.data_mode.rio.r_doing = NULL;
    }
    else
    {
        pipe->backend.data_mode.rio.r_doing = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    }

    _ev_pipe_r_user_callback_win(req, req->base.data.size);
}

static int _ev_pipe_on_data_mode_read_recv(ev_pipe_t* pipe)
{
    int ret = 0;

    ev_pipe_read_req_t* req;

    size_t bufidx = 0;
    size_t bufpos = 0;

    DWORD avail;
    while ((req = pipe->backend.data_mode.rio.r_doing) != NULL)
    {
        if (!PeekNamedPipe(pipe->pipfd, NULL, 0, NULL, &avail, NULL))
        {
            ret = GetLastError();
            return ev__translate_sys_error(ret);
        }

        /* no more data to read */
        if (avail == 0)
        {
            _ev_pipe_data_mode_callback_and_mount_next_win(pipe, req);
            break;
        }

        size_t total = 0;
        ret = _ev_pipe_read_into_req(pipe->pipfd, req, avail, bufidx, bufpos,
            &bufidx, &bufpos, &total);
        req->base.data.size += total;

        if (ret != 0)
        {
            return ret;
        }

        if (req->base.data.size < req->base.data.capacity)
        {
            continue;
        }

        _ev_pipe_data_mode_callback_and_mount_next_win(pipe, req);
    }

    return 0;
}

static int _ev_pipe_is_success_iocp_request(const ev_iocp_t* iocp)
{
    NTSTATUS status = (NTSTATUS)(iocp->overlapped.Internal);
    return NT_SUCCESS(status);
}

static DWORD _ev_pipe_get_iocp_error(const ev_iocp_t* iocp)
{
    NTSTATUS status = (NTSTATUS)(iocp->overlapped.Internal);
    return ev_winapi.RtlNtStatusToDosError(status);
}

static void _ev_pipe_on_data_mode_read_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;

    int ret;
    ev_pipe_t* pipe = arg;

    if (!_ev_pipe_is_success_iocp_request(iocp))
    {
        int err = _ev_pipe_get_iocp_error(iocp);
        ret = ev__translate_sys_error(err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    /* Do actual read */
    ret = _ev_pipe_on_data_mode_read_recv(pipe);
    if (ret != 0)
    {
        _ev_pipe_abort(pipe, ret);
        return;
    }

    /* If there are pending read request, we submit another IOCP request */
    if (pipe->backend.data_mode.rio.r_doing != NULL
        || ev_list_size(&pipe->backend.data_mode.rio.r_pending) != 0)
    {
        _ev_pipe_data_mode_want_read(pipe);
        return;
    }
}

static int _ev_pipe_write_file_iocp(HANDLE file, const void* buffer, size_t size, LPOVERLAPPED iocp)
{
    memset(iocp, 0, sizeof(*iocp));
    int result = WriteFile(file, buffer, (DWORD)size, NULL, iocp);
    if (result)
    {
        return 0;
    }

    int err = GetLastError();
    if (err == ERROR_IO_PENDING)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

static int _ev_pipe_io_wio_submit_half(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    ev_pipe_write_req_t* half_req = pipe->backend.data_mode.wio.w_half;
    size_t half_idx = pipe->backend.data_mode.wio.w_half_idx;

    wio->w_req = half_req;
    wio->w_buf_idx = half_idx;

    int result = _ev_pipe_write_file_iocp(pipe->pipfd, half_req->base.bufs[half_idx].data,
        half_req->base.bufs[half_idx].size, &wio->io.overlapped);
    pipe->backend.data_mode.wio.w_half_idx++;

    /* move half record to doing list */
    if (pipe->backend.data_mode.wio.w_half_idx == half_req->base.nbuf)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &half_req->base.node);
        pipe->backend.data_mode.wio.w_half = NULL;
        pipe->backend.data_mode.wio.w_half_idx = 0;
    }

    return result;
}

/**
 * @return 1: no more buffer need to send
 */
static int _ev_pipe_io_wio_submit_pending(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.data_mode.wio.w_pending);
    if (it == NULL)
    {
        return 1;
    }
    ev_pipe_write_req_t* wreq = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);

    wio->w_req = wreq;
    wio->w_buf_idx = 0;

    int result = _ev_pipe_write_file_iocp(pipe->pipfd, wreq->base.bufs[0].data,
        wreq->base.bufs[0].size, &wio->io.overlapped);

    if (wreq->base.nbuf == 1)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &wreq->base.node);
    }
    else
    {
        pipe->backend.data_mode.wio.w_half = wreq;
        pipe->backend.data_mode.wio.w_half_idx = 1;
    }

    return result;
}

/**
 * @return 0: success, 1: no more buffer need to send, any other value: failure
 */
static int _ev_pipe_io_wio_submit_next(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    if (pipe->backend.data_mode.wio.w_half != NULL)
    {
        return _ev_pipe_io_wio_submit_half(pipe, wio);
    }

    return _ev_pipe_io_wio_submit_pending(pipe, wio);
}

static void _ev_pipe_on_data_mode_write(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_pipe_t* pipe = arg;
    struct ev_pipe_backend_data_mode_wio* wio = EV_CONTAINER_OF(iocp, struct ev_pipe_backend_data_mode_wio, io);

    /* wio will be override, we need to backup value */
    ev_pipe_write_req_t* curr_req = wio->w_req;
    size_t curr_buf_idx = wio->w_buf_idx;

    /* update send size */
    curr_req->base.size += transferred;

    /* override wio with next write request */
    int submit_ret = _ev_pipe_io_wio_submit_next(pipe, wio);
    if (submit_ret != 0)
    {
        pipe->backend.data_mode.wio.w_io_cnt--;
    }

    /* The last buffer */
    if (curr_buf_idx == curr_req->base.nbuf - 1)
    {
        ssize_t stat = NT_SUCCESS(iocp->overlapped.Internal) ? 0 :
            ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
        ev_list_erase(&pipe->backend.data_mode.wio.w_doing, &curr_req->base.node);

        stat = stat < 0 ? stat : curr_req->base.size;
        _ev_pipe_w_user_callback_win(curr_req, stat);
    }

    /* If submit error, abort any pending actions */
    if (submit_ret != 0 && submit_ret != 1)
    {
        _ev_pipe_abort(pipe, submit_ret);
    }
}

static void _ev_pipe_init_data_mode_r(ev_pipe_t* pipe)
{
    ev__iocp_init(&pipe->backend.data_mode.rio.io, _ev_pipe_on_data_mode_read_win, pipe);
    ev_list_init(&pipe->backend.data_mode.rio.r_pending);
    pipe->backend.data_mode.rio.r_doing = NULL;
}

static void _ev_pipe_init_data_mode_w(ev_pipe_t* pipe)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pipe->backend.data_mode.wio.iocp); i++)
    {
        ev__iocp_init(&pipe->backend.data_mode.wio.iocp[i].io, _ev_pipe_on_data_mode_write, pipe);
        pipe->backend.data_mode.wio.iocp[i].idx = i;
        pipe->backend.data_mode.wio.iocp[i].w_req = NULL;
        pipe->backend.data_mode.wio.iocp[i].w_buf_idx = 0;
    }
    pipe->backend.data_mode.wio.w_io_idx = 0;
    pipe->backend.data_mode.wio.w_io_cnt = 0;

    ev_list_init(&pipe->backend.data_mode.wio.w_pending);
    ev_list_init(&pipe->backend.data_mode.wio.w_doing);
    pipe->backend.data_mode.wio.w_half = NULL;
    pipe->backend.data_mode.wio.w_half_idx = 0;
}

static int _ev_pipe_read_exactly(HANDLE file, void* buffer, size_t size)
{
    int err;
    DWORD bytes_read, bytes_read_now;

    bytes_read = 0;
    while (bytes_read < size)
    {
        if (!ReadFile(file, (char*)buffer + bytes_read, (DWORD)size - bytes_read, &bytes_read_now, NULL))
        {
            err = GetLastError();
            return ev__translate_sys_error(err);
        }

        bytes_read += bytes_read_now;
    }

    assert(bytes_read == size);
    return 0;
}

static ev_pipe_read_req_t* _ev_pipe_on_ipc_mode_read_mount_next(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending);
    if (it == NULL)
    {
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
    }
    else
    {
        pipe->backend.ipc_mode.rio.reading.reading =
            EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    }

    return pipe->backend.ipc_mode.rio.reading.reading;
}

static int _ev_pipe_on_ipc_mode_read_information(ev_pipe_t* pipe, ev_ipc_frame_hdr_t* hdr)
{
    if (hdr->hdr_exsz != sizeof(ev_pipe_win_ipc_info_t))
    {
        return EV_EPROTO;
    }

    void* buffer = (uint8_t*)pipe->backend.ipc_mode.rio.buffer + sizeof(ev_ipc_frame_hdr_t);
    size_t buffer_size = sizeof(pipe->backend.ipc_mode.rio.buffer) - sizeof(ev_ipc_frame_hdr_t);
    assert(buffer_size >= sizeof(ev_pipe_win_ipc_info_t));

    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, buffer_size);
    assert(ret == 0); (void)ret;

    ev_pipe_read_req_t* req;
    ev_pipe_win_ipc_info_t* ipc_info = buffer;

    switch (ipc_info->type)
    {
    case EV_PIPE_WIN_IPC_INFO_TYPE_STATUS:
        pipe->backend.ipc_mode.peer_pid = ipc_info->data.as_status.pid;
        break;

    case EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO:
        if ((req = pipe->backend.ipc_mode.rio.reading.reading) == NULL)
        {
            req = _ev_pipe_on_ipc_mode_read_mount_next(pipe);
        }
        assert(req != NULL);
        req->handle.os_socket = WSASocketW(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
            FROM_PROTOCOL_INFO, &ipc_info->data.as_protocol_info, 0, WSA_FLAG_OVERLAPPED);
        if (req->handle.os_socket == INVALID_SOCKET)
        {
            int errcode = WSAGetLastError();
            return ev__translate_sys_error(errcode);
        }
        break;

    default:
        abort();
        break;
    }

    return 0;
}

static int _ev_pipe_on_ipc_mode_read_remain(ev_pipe_t* pipe)
{
    DWORD avail, errcode;
    while (pipe->backend.ipc_mode.rio.remain_size > 0
        && PeekNamedPipe(pipe->pipfd, NULL, 0, NULL, &avail, NULL) && avail > 0)
    {
        ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.reading.reading;
        if (req == NULL)
        {
            req = _ev_pipe_on_ipc_mode_read_mount_next(pipe);
            if (req == NULL)
            {
                return 0;
            }
        }

        size_t buf_idx = pipe->backend.ipc_mode.rio.reading.buf_idx;
        ev_buf_t* buf = &req->base.data.bufs[buf_idx];
        void* buffer = (uint8_t*)buf->data + pipe->backend.ipc_mode.rio.reading.buf_pos;

        DWORD buffer_size = buf->size - pipe->backend.ipc_mode.rio.reading.buf_pos;
        buffer_size = EV_MIN(buffer_size, pipe->backend.ipc_mode.rio.remain_size);
        buffer_size = EV_MIN(buffer_size, avail);

        DWORD read_size;
        if (!ReadFile(pipe->pipfd, buffer, buffer_size, &read_size, NULL))
        {
            errcode = GetLastError();
            goto err;
        }
        pipe->backend.ipc_mode.rio.remain_size -= read_size;
        pipe->backend.ipc_mode.rio.reading.buf_pos += read_size;
        req->base.data.size += read_size;

        /* Read the whole frame */
        if (pipe->backend.ipc_mode.rio.remain_size == 0)
        {
            pipe->backend.ipc_mode.rio.reading.reading = NULL;
            _ev_pipe_r_user_callback_win(req, req->base.data.size);
            continue;
        }

        /* Remain data to read */
        if (pipe->backend.ipc_mode.rio.reading.buf_pos < buf->size)
        {
            continue;
        }

        /* Move to next buffer */
        pipe->backend.ipc_mode.rio.reading.buf_pos = 0;
        pipe->backend.ipc_mode.rio.reading.buf_idx++;
        if (pipe->backend.ipc_mode.rio.reading.buf_idx < req->base.data.nbuf)
        {
            continue;
        }

        /* Buffer is full, need to notify user */
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
        _ev_pipe_r_user_callback_win(req, req->base.data.size);
    }

    return 0;

err:
    return ev__translate_sys_error(errcode);
}

static int _ev_pipe_on_ipc_mode_read_first(ev_pipe_t* pipe)
{
    /* Read */
    void* buffer = pipe->backend.ipc_mode.rio.buffer;
    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, sizeof(ev_ipc_frame_hdr_t));
    if (ret != 0)
    {
        return ret;
    }

    if (!ev__ipc_check_frame_hdr(buffer, sizeof(ev_ipc_frame_hdr_t)))
    {
        return EV_EPROTO;
    }

    ev_ipc_frame_hdr_t* hdr = buffer;
    if (hdr->hdr_flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        if ((ret = _ev_pipe_on_ipc_mode_read_information(pipe, hdr)) != 0)
        {
            return ret;
        }
    }

    pipe->backend.ipc_mode.rio.remain_size = hdr->hdr_dtsz;
    return _ev_pipe_on_ipc_mode_read_remain(pipe);
}

static int _ev_pipe_ipc_mode_want_read(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.rio.mask.rio_pending)
    {
        return 0;
    }

    int result = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL, &pipe->backend.ipc_mode.rio.io.overlapped);
    if (!result)
    {
        int err = GetLastError();
        if (err != ERROR_IO_PENDING)
        {
            return ev__translate_sys_error(err);
        }
    }

    pipe->backend.ipc_mode.rio.mask.rio_pending = 1;
    return 0;
}

static void _ev_pipe_on_ipc_mode_read(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_pipe_t* pipe = arg;
    /* Clear IOCP pending flag */
    pipe->backend.ipc_mode.rio.mask.rio_pending = 0;

    /* Check error */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        int ret = ev__translate_sys_error(winsock_err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    int ret;
    if (pipe->backend.ipc_mode.rio.remain_size != 0)
    {
        ret = _ev_pipe_on_ipc_mode_read_remain(pipe);
    }
    else
    {
        ret = _ev_pipe_on_ipc_mode_read_first(pipe);
    }

    if (ret != 0)
    {
        _ev_pipe_abort(pipe, ret);
        return;
    }

    if (pipe->backend.ipc_mode.rio.reading.reading != NULL
        || ev_list_size(&pipe->backend.ipc_mode.rio.pending))
    {
        _ev_pipe_ipc_mode_want_read(pipe);
        return;
    }
}

/**
 * @breif Initialize buffer as #ev_ipc_frame_hdr_t
 *
 * Write sizeof(ev_ipc_frame_hdr_t) bytes
 */
static void _ev_pipe_init_ipc_frame_hdr(uint8_t* buffer, size_t bufsize, uint8_t flags, uint32_t dtsz)
{
    assert(bufsize >= sizeof(ev_ipc_frame_hdr_t)); (void)bufsize;

    uint16_t exsz = 0;
    if (flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        exsz = sizeof(ev_pipe_win_ipc_info_t);
    }

    ev__ipc_init_frame_hdr((ev_ipc_frame_hdr_t*)buffer, flags, exsz, dtsz);
}

/**
 * @brief Send IPC data
 */
static int _ev_pipe_ipc_mode_write_data(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    uint8_t flags = 0;
    ev_pipe_win_ipc_info_t ipc_info;
    size_t hdr_size = 0;

    assert(pipe->backend.ipc_mode.wio.sending.w_req == NULL);

    if (req->handle.role != EV_ROLE_UNKNOWN)
    {
        flags |= EV_IPC_FRAME_FLAG_INFORMATION;

        DWORD target_pid = pipe->backend.ipc_mode.peer_pid;
        if (target_pid == EV_INVALID_PID_WIN)
        {
            return EV_EAGAIN;
        }

        memset(&ipc_info, 0, sizeof(ipc_info));
        ipc_info.type = EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO;

        if (WSADuplicateSocketW(req->handle.u.os_socket, target_pid, &ipc_info.data.as_protocol_info))
        {
            int err = WSAGetLastError();
            return ev__translate_sys_error(err);
        }

        void* buffer = (uint8_t*)pipe->backend.ipc_mode.wio.buffer + sizeof(ev_ipc_frame_hdr_t);
        memcpy(buffer, &ipc_info, sizeof(ipc_info));
        hdr_size += sizeof(ipc_info);
    }

    _ev_pipe_init_ipc_frame_hdr(pipe->backend.ipc_mode.wio.buffer,
        sizeof(pipe->backend.ipc_mode.wio.buffer), flags, (uint32_t)req->base.capacity);
    hdr_size += sizeof(ev_ipc_frame_hdr_t);

    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
        hdr_size, &pipe->backend.ipc_mode.wio.io.overlapped);
    if (ret != 0)
    {
        return ret;
    }

    pipe->backend.ipc_mode.wio.sending.w_req = req;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    return 0;
}

static int _ev_pipe_ipc_mode_send_next(ev_pipe_t* pipe)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending);
    if (it == NULL)
    {
        return 0;
    }

    ev_pipe_write_req_t* next_req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
    int ret = _ev_pipe_ipc_mode_write_data(pipe, next_req);
    if (ret != 0)
    {
        ev_list_push_front(&pipe->backend.ipc_mode.wio.pending, it);
    }
    return ret;
}

static int _ev_pipe_on_ipc_mode_write_process(ev_pipe_t* pipe, size_t transferred)
{
    if (pipe->backend.ipc_mode.wio.sending.w_req == NULL)
    {/* This is a builtin status notify */
        return _ev_pipe_ipc_mode_send_next(pipe);
    }

    pipe->backend.ipc_mode.wio.sending.donecnt++;

    if (pipe->backend.ipc_mode.wio.sending.donecnt == 1)
    {/* Frame header send success */
        /* Do nothing */
    }
    else
    {
        pipe->backend.ipc_mode.wio.sending.w_req->base.size += transferred;

        if (pipe->backend.ipc_mode.wio.sending.donecnt > pipe->backend.ipc_mode.wio.sending.w_req->base.nbuf)
        {
            goto finish_request;
        }
    }

    size_t send_buf_idx = pipe->backend.ipc_mode.wio.sending.donecnt - 1;
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.sending.w_req;
    ev_buf_t* buf = &req->base.bufs[send_buf_idx];

    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, buf->data, buf->size,
        &pipe->backend.ipc_mode.wio.io.overlapped);
    return ret;

finish_request:
    req = pipe->backend.ipc_mode.wio.sending.w_req;
    pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    _ev_pipe_w_user_callback_win(req, req->base.size);

    if ((ret = _ev_pipe_ipc_mode_send_next(pipe)) != 0)
    {
        return ret;
    }

    return 0;
}

static void _ev_pipe_on_ipc_mode_write(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    int ret = 0;
    ev_pipe_t* pipe = arg;
    pipe->backend.ipc_mode.wio.mask.iocp_pending = 0;

    /* Check status */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        ret = ev__translate_sys_error(winsock_err);
        goto err;
    }

    if ((ret = _ev_pipe_on_ipc_mode_write_process(pipe, transferred)) != 0)
    {
        goto err;
    }
    return;

err:
    if (ret != 0)
    {
        pipe->backend.ipc_mode.wio.w_err = ret;
    }
    _ev_pipe_abort(pipe, ret);
}

static void _ev_pipe_init_as_ipc(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.iner_err = 0;
    pipe->backend.ipc_mode.peer_pid = 0;

    /* rio */
    memset(&pipe->backend.ipc_mode.rio.mask, 0, sizeof(pipe->backend.ipc_mode.rio.mask));
    pipe->backend.ipc_mode.rio.reading.reading = NULL;
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;
    ev_list_init(&pipe->backend.ipc_mode.rio.pending);
    pipe->backend.ipc_mode.rio.r_err = 0;
    pipe->backend.ipc_mode.rio.remain_size = 0;
    ev__iocp_init(&pipe->backend.ipc_mode.rio.io, _ev_pipe_on_ipc_mode_read, pipe);

    /* wio */
    memset(&pipe->backend.ipc_mode.wio.mask, 0, sizeof(pipe->backend.ipc_mode.wio.mask));
    pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;
    ev_list_init(&pipe->backend.ipc_mode.wio.pending);
    pipe->backend.ipc_mode.wio.w_err = 0;
    ev__iocp_init(&pipe->backend.ipc_mode.wio.io, _ev_pipe_on_ipc_mode_write, pipe);
}

static void _ev_pipe_init_as_data(ev_pipe_t* pipe)
{
    _ev_pipe_init_data_mode_r(pipe);
    _ev_pipe_init_data_mode_w(pipe);
}

static int _ev_pipe_notify_status(ev_pipe_t* pipe)
{
    _ev_pipe_init_ipc_frame_hdr(pipe->backend.ipc_mode.wio.buffer,
        sizeof(pipe->backend.ipc_mode.wio.buffer), EV_IPC_FRAME_FLAG_INFORMATION, 0);

    ev_pipe_win_ipc_info_t ipc_info;
    memset(&ipc_info, 0, sizeof(ev_pipe_win_ipc_info_t));
    ipc_info.type = EV_PIPE_WIN_IPC_INFO_TYPE_STATUS;
    ipc_info.data.as_status.pid = GetCurrentProcessId();
    memcpy(pipe->backend.ipc_mode.wio.buffer + sizeof(ev_ipc_frame_hdr_t), &ipc_info, sizeof(ipc_info));

    DWORD send_size = sizeof(ev_ipc_frame_hdr_t) + sizeof(ev_pipe_win_ipc_info_t);

    pipe->backend.ipc_mode.wio.mask.iocp_pending = 1;
    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
        send_size, &pipe->backend.ipc_mode.wio.io.overlapped);
    return ret;
}

static size_t _ev_pipe_get_and_forward_w_idx(ev_pipe_t* pipe)
{
    size_t ret = pipe->backend.data_mode.wio.w_io_idx;
    if (pipe->backend.data_mode.wio.w_io_idx == ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) - 1)
    {
        pipe->backend.data_mode.wio.w_io_idx = 0;
    }
    else
    {
        pipe->backend.data_mode.wio.w_io_idx++;
    }
    pipe->backend.data_mode.wio.w_io_cnt++;

    return ret;
}

static size_t _ev_pipe_revert_w_idx(ev_pipe_t* pipe)
{
    if (pipe->backend.data_mode.wio.w_io_idx == 0)
    {
        pipe->backend.data_mode.wio.w_io_idx = ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) - 1;
    }
    else
    {
        pipe->backend.data_mode.wio.w_io_idx--;
    }
    pipe->backend.data_mode.wio.w_io_cnt--;

    return pipe->backend.data_mode.wio.w_io_idx;
}

/**
 * @brief Write in DATA mode.
 * 
 * In DATA mode, every #ev_pipe_backend_t::w_io::iocp can be used to provide maximum
 * performance.
 */
static int _ev_pipe_data_mode_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    int result;
    int flag_failure = 0;
    DWORD err = 0;

    req->backend.owner = pipe;
    req->backend.stat = EV_EINPROGRESS;

    size_t available_iocp_cnt = ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) -
        pipe->backend.data_mode.wio.w_io_cnt;
    if (available_iocp_cnt == 0)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_pending, &req->base.node);
        return 0;
    }

    size_t idx;
    size_t nbuf = EV_MIN(available_iocp_cnt, req->base.nbuf);
    for (idx = 0; idx < nbuf; idx++)
    {
        size_t pos = _ev_pipe_get_and_forward_w_idx(pipe);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_req == NULL);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_buf_idx == 0);

        pipe->backend.data_mode.wio.iocp[pos].w_req = req;
        pipe->backend.data_mode.wio.iocp[pos].w_buf_idx = idx;

        result = _ev_pipe_write_file_iocp(pipe->pipfd, req->base.bufs[idx].data,
            req->base.bufs[idx].size, &pipe->backend.data_mode.wio.iocp[pos].io.overlapped);
        /* write success */
        if (result == 0)
        {
            continue;
        }

        flag_failure = 1;
        break;
    }

    if (flag_failure)
    {
        size_t i;
        for (i = 0; i <= idx; i++)
        {
            size_t pos = _ev_pipe_revert_w_idx(pipe);
            CancelIoEx(pipe->pipfd, &pipe->backend.data_mode.wio.iocp[pos].io.overlapped);
            pipe->backend.data_mode.wio.iocp[pos].w_req = NULL;
            pipe->backend.data_mode.wio.iocp[pos].w_buf_idx = 0;
        }
        return ev__translate_sys_error(err);
    }

    if (nbuf < req->base.nbuf)
    {
        pipe->backend.data_mode.wio.w_half = req;
        pipe->backend.data_mode.wio.w_half_idx = nbuf;
    }
    else
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &req->base.node);
    }

    ev__handle_event_add(&pipe->base);
    return 0;
}

/**
 * @brief Write in IPC mode.
 *
 * In IPC mode, we only use #ev_pipe_backend_t::w_io::iocp[0] for simplify implementation.
 */
static int _ev_pipe_ipc_mode_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    if (pipe->backend.ipc_mode.iner_err != 0)
    {
        return pipe->backend.ipc_mode.iner_err;
    }

    /* Check total send size, limited by IPC protocol */
    if (req->base.capacity > UINT32_MAX)
    {
        return EV_E2BIG;
    }

    /* If we have pending IOCP request, add it to queue */
    if (pipe->backend.ipc_mode.wio.mask.iocp_pending
        || pipe->backend.ipc_mode.wio.sending.w_req != NULL)
    {
        ev_list_push_back(&pipe->backend.ipc_mode.wio.pending, &req->base.node);
        return 0;
    }

    return _ev_pipe_ipc_mode_write_data(pipe, req);
}

static int _ev_pipe_read_ipc_mode(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    int ret;
    if (pipe->backend.ipc_mode.iner_err != 0)
    {
        return pipe->backend.ipc_mode.iner_err;
    }

    ev_list_push_back(&pipe->backend.ipc_mode.rio.pending, &req->base.node);

    if ((ret = _ev_pipe_ipc_mode_want_read(pipe)) != 0)
    {
        ev_list_erase(&pipe->backend.ipc_mode.rio.pending, &req->base.node);
    }

    return ret;
}

static int _ev_pipe_read_data_mode(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    if (pipe->backend.data_mode.rio.r_doing != NULL)
    {
        ev_list_push_back(&pipe->backend.data_mode.rio.r_pending, &req->base.node);
        return 0;
    }

    pipe->backend.data_mode.rio.r_doing = req;
    return _ev_pipe_data_mode_want_read(pipe);
}

static int _ev_pipe_init_read_token_win(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_pipe_read_cb cb)
{
    int ret;

    if ((ret = ev__pipe_read_init(req, bufs, nbuf, cb)) != 0)
    {
        return ret;
    }

    req->backend.owner = pipe;
    req->backend.stat = EV_EINPROGRESS;

    return 0;
}

static int _ev_pipe_make_win(ev_os_pipe_t fds[2], int rflags, int wflags,
        const char *name)
{
    int err;
    HANDLE pip_r = INVALID_HANDLE_VALUE;
    HANDLE pip_w = INVALID_HANDLE_VALUE;

    err = _ev_pipe_make_s(&pip_r, name, rflags);
    if (err != 0)
    {
        goto err_close_rw;
    }

    err = _ev_pipe_make_c(&pip_w, name, wflags);
    if (pip_w == INVALID_HANDLE_VALUE)
    {
        goto err_close_rw;
    }

    if (!ConnectNamedPipe(pip_r, NULL))
    {
        err = GetLastError();
        if (err != ERROR_PIPE_CONNECTED)
        {
            err = ev__translate_sys_error(err);
            goto err_close_rw;
        }
    }

    fds[0] = pip_r;
    fds[1] = pip_w;

    return 0;

err_close_rw:
    if (pip_r != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pip_r);
    }
    if (pip_w != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pip_w);
    }
    return err;
}

static int _ev_pipe_open_check_win(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        return EV_EEXIST;
    }
    if (handle == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    IO_STATUS_BLOCK io_status;
    FILE_ACCESS_INFORMATION access;
    NTSTATUS nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status, &access, sizeof(access), FileAccessInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return EV_EINVAL;
    }

    DWORD mode = PIPE_READMODE_BYTE | PIPE_WAIT;
    if (!SetNamedPipeHandleState(handle, &mode, NULL, NULL))
    {
        DWORD err = GetLastError();
        if (err != ERROR_ACCESS_DENIED)
        {
            return EV_EBADF;
        }

        DWORD current_mode = 0;
        if (!GetNamedPipeHandleState(handle, &current_mode, NULL, NULL, NULL, NULL, 0))
        {
            return ev__translate_sys_error(GetLastError());
        }
        if (current_mode & PIPE_NOWAIT)
        {
            return ev__translate_sys_error(ERROR_ACCESS_DENIED);
        }
    }

    FILE_MODE_INFORMATION mode_info;
    nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status, &mode_info, sizeof(mode_info), FileModeInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return ev__translate_sys_error(GetLastError());
    }

    return 0;
}

int ev_pipe_make(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    static long volatile s_pipe_serial_no = 0;
    char buffer[128];

    fds[0] = EV_OS_PIPE_INVALID;
    fds[1] = EV_OS_PIPE_INVALID;
    if ((rflags & EV_PIPE_IPC) != (wflags & EV_PIPE_IPC))
    {
        return EV_EINVAL;
    }

    snprintf(buffer, sizeof(buffer), "\\\\.\\pipe\\LOCAL\\libev\\RemoteExeAnon.%08lx.%08lx",
        (long)GetCurrentProcessId(), InterlockedIncrement(&s_pipe_serial_no));

    rflags |= EV_PIPE_READABLE;
    wflags |= EV_PIPE_WRITABLE;

    int is_ipc = rflags & EV_PIPE_IPC;
    if (is_ipc)
    {
        rflags |= EV_PIPE_WRITABLE;
        wflags |= EV_PIPE_READABLE;
    }
    else
    {
        rflags &= ~EV_PIPE_WRITABLE;
        wflags &= ~EV_PIPE_READABLE;
    }

    return _ev_pipe_make_win(fds, rflags, wflags, buffer);
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;
    pipe->base.data.flags |= ipc ? EV_HANDLE_PIPE_IPC : 0;

    if (ipc)
    {
        _ev_pipe_init_as_ipc(pipe);
    }
    else
    {
        _ev_pipe_init_as_data(pipe);
    }

    return 0;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    _ev_pipe_close_pipe(pipe);

    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base, _ev_pipe_on_close_win);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    int ret;

    if ((ret = _ev_pipe_open_check_win(pipe, handle)) != 0)
    {
        return ret;
    }

    if (CreateIoCompletionPort(handle, pipe->base.loop->backend.iocp, (ULONG_PTR)pipe, 0) == NULL)
    {
        return ev__translate_sys_error(GetLastError());
    }
    pipe->pipfd = handle;

    if (!(pipe->base.data.flags & EV_HANDLE_PIPE_IPC))
    {
        return 0;
    }

    /**
     * TODO:
     * In IPC mode, we need to setup communication.
     * 
     * Here we may have problem that if the pipe is read-only / write-only, we
     * cannot unbind IOCP beacuse windows not support that.
     * 
     * There are may ways to avoid it:
     * 1. Avoid handeshake procedure. We need handeshake because we need child
     *   process information to call DuplicateHandle(). But accroding to
     *   https://stackoverflow.com/questions/46348163/how-to-transfer-the-duplicated-handle-to-the-child-process
     *   we can call DuplicateHandle() in child process, as long as we wait for
     *   peer response.
     * 2. If handle is not readable, we return success but mark it as error, and
     *   notify user error in future operation.
     */
    if ((ret = _ev_pipe_notify_status(pipe)) != 0)
    {
        pipe->backend.ipc_mode.iner_err = ret;
        return 0;
    }

    if ((ret = _ev_pipe_ipc_mode_want_read(pipe)) != 0)
    {
        pipe->backend.ipc_mode.iner_err = ret;
        return 0;
    }

    return 0;
}

int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size,
    ev_pipe_write_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_write_init_ext(req, cb, bufs, nbuf, handle_role, handle_addr, handle_size);
    if (ret != 0)
    {
        return ret;
    }

    req->backend.owner = pipe;
    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_ipc_mode_write(pipe, req);
    }
    else
    {
        ret = _ev_pipe_data_mode_write(pipe, req);
    }

    if (ret != 0)
    {
        ev__write_exit(&req->base);
        ev__handle_event_dec(&pipe->base);
    }

    return ret;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_read_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = _ev_pipe_init_read_token_win(pipe, req, bufs, nbuf, cb);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_read_ipc_mode(pipe, req);
    }
    else
    {
        ret = _ev_pipe_read_data_mode(pipe, req);
    }

    if (ret != 0)
    {
        ev__handle_event_dec(&pipe->base);
        ev__read_exit(&req->base);
    }

    return ret;
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    (void)pipe;
    if (req->handle.os_socket == EV_OS_SOCKET_INVALID)
    {
        return EV_ENOENT;
    }

    if (handle_role != EV_ROLE_EV_TCP
        || handle_addr == NULL
        || handle_size != sizeof(ev_tcp_t))
    {
        return EV_EINVAL;
    }

    int ret = ev__tcp_open_win((ev_tcp_t*)handle_addr, req->handle.os_socket);
    req->handle.os_socket = EV_OS_SOCKET_INVALID;

    return ret;
}

void ev_pipe_close(ev_os_pipe_t fd)
{
    CloseHandle(fd);
}

#line 1 "src/win/process_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "async.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "defs.h" */
/* AMALGAMATE: #include "pipe_win.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "process_win.h" */
#include <assert.h>

typedef struct ev_startup_info
{
    STARTUPINFO start_info;

    char*       cmdline;
    char*       envline;
}ev_startup_info_t;

typedef struct stdio_pair_s
{
    HANDLE*     dst;
    DWORD       type;
} stdio_pair_t;

static int _dup_cmd(char** buf, char* const argv[])
{
    char* cmdline = ev_malloc(MAX_PATH + 1);
    if (cmdline == NULL)
    {
        return EV_ENOMEM;
    }

    cmdline[0] = '\0';

    strcat_s(cmdline, MAX_PATH, argv[0]);
    for (int i = 1; argv[i] != NULL; i++)
    {
        strcat_s(cmdline, MAX_PATH, " ");
        strcat_s(cmdline, MAX_PATH, argv[i]);
    }

    *buf = cmdline;
    return 0;
}

static int _dup_envp(char**buf, char* const envp[])
{
    if (envp == NULL)
    {
        *buf = NULL;
        return 0;
    }

    size_t malloc_size = 1;
    size_t idx = 0;

    for (idx = 0; envp[idx] != NULL; idx++)
    {
        malloc_size += strlen(envp[idx]) + 1;
    }

    char* envline = ev_malloc(malloc_size);
    if (envline == NULL)
    {
        return EV_ENOMEM;
    }

    envline[malloc_size - 1] = '\0';

    size_t pos = 0;
    for (idx = 0; envp[idx] != NULL; idx++)
    {
        size_t cplen = strlen(envp[idx]) + 1;
        memcpy(envline + pos, envp[idx], cplen);
        pos += cplen;
    }

    *buf = envline;
    return 0;
}

static int _ev_process_setup_stdio_as_null(HANDLE* handle, DWORD dwDesiredAccess)
{
    DWORD errcode;
    HANDLE nul_file = CreateFileW(L"NUL:", dwDesiredAccess,
        FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (nul_file == INVALID_HANDLE_VALUE)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    *handle = nul_file;
    return 0;
}

static int _ev_process_setup_stdio_as_fd(HANDLE* duph, HANDLE handle)
{
    HANDLE current_process = GetCurrentProcess();
    BOOL ret = DuplicateHandle(current_process, handle, current_process, duph, 0, TRUE, DUPLICATE_SAME_ACCESS);
    if (!ret)
    {
        *duph = INVALID_HANDLE_VALUE;
        DWORD errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    return 0;
}

static int _ev_process_setup_stdio_as_pipe_win(ev_pipe_t* pipe, HANDLE* handle, int is_pipe_read)
{
    int ret;
    ev_os_pipe_t pipfd[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };

    /* fd for #ev_pipe_t should open in nonblock mode */
    int rflags = is_pipe_read ? EV_PIPE_NONBLOCK : 0;
    int wflags = is_pipe_read ? 0 : EV_PIPE_NONBLOCK;

    if ((ret = ev_pipe_make(pipfd, rflags, wflags)) != 0)
    {
        return ret;
    }

    if ((ret = ev_pipe_open(pipe, is_pipe_read ? pipfd[0] : pipfd[1])) != 0)
    {
        goto err;
    }

    *handle = is_pipe_read ? pipfd[1] : pipfd[0];

    return 0;

err:
    ev_pipe_close(pipfd[0]);
    ev_pipe_close(pipfd[1]);
    return ret;
}

static int _ev_process_dup_stdin_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdInput, GENERIC_READ);
    }
    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdInput, container->data.fd);
    }
    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdInput, 0);
    }

    return 0;
}

static int _ev_process_dup_stdout_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdOutput, GENERIC_WRITE);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdOutput, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdOutput, 1);
    }

    return 0;
}

static int _ev_process_dup_stderr_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdError, GENERIC_WRITE);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdOutput, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdError, 1);
    }

    return 0;
}

static void _ev_process_close_stdin_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdInput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdInput);
        info->start_info.hStdInput = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_close_stdout_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdOutput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdOutput);
        info->start_info.hStdOutput = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_close_stderr_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdError != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdError);
        info->start_info.hStdError = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_cleanup_cmdline(ev_startup_info_t* start_info)
{
    if (start_info->cmdline != NULL)
    {
        ev_free(start_info->cmdline);
        start_info->cmdline = NULL;
    }
}

static void _ev_process_cleanup_envp(ev_startup_info_t* start_info)
{
    if (start_info->envline != NULL)
    {
        ev_free(start_info->envline);
        start_info->envline = NULL;
    }
}

static void _ev_process_cleanup_start_info(ev_startup_info_t* start_info)
{
    _ev_process_close_stdin_win(start_info);
    _ev_process_close_stdout_win(start_info);
    _ev_process_close_stderr_win(start_info);
    _ev_process_cleanup_cmdline(start_info);
    _ev_process_cleanup_envp(start_info);
}

static int _ev_process_inherit_stdio(ev_startup_info_t* info)
{
    stdio_pair_t stdio_pair_list[] = {
        { &info->start_info.hStdInput, STD_INPUT_HANDLE },
        { &info->start_info.hStdOutput, STD_OUTPUT_HANDLE },
        { &info->start_info.hStdError, STD_ERROR_HANDLE },
    };

    BOOL dupret;
    DWORD errcode;
    HANDLE current_process = GetCurrentProcess();

    size_t i;
    for (i = 0; i < ARRAY_SIZE(stdio_pair_list); i++)
    {
        if (*(stdio_pair_list[i].dst) != INVALID_HANDLE_VALUE)
        {
            /* The stdio handle must be inherited */
            if (!SetHandleInformation(*(stdio_pair_list[i].dst), HANDLE_FLAG_INHERIT, 1))
            {
                errcode = GetLastError();
                return ev__translate_sys_error(errcode);
            }
            continue;
        }

        dupret = DuplicateHandle(current_process, GetStdHandle(stdio_pair_list[i].type),
            current_process, stdio_pair_list[i].dst, 0, TRUE, DUPLICATE_SAME_ACCESS);
        if (!dupret)
        {
            errcode = GetLastError();
            return ev__translate_sys_error(errcode);
        }
    }

    return 0;
}

static int _ev_process_dup_stdio_win(ev_startup_info_t* info, const ev_process_options_t* opt)
{
    int ret;

    if ((ret = _ev_process_dup_stdin_win(info, &opt->stdios[0])) != 0)
    {
        return ret;
    }

    if ((ret = _ev_process_dup_stdout_win(info, &opt->stdios[1])) != 0)
    {
        goto err;
    }

    if ((ret = _ev_process_dup_stderr_win(info, &opt->stdios[2])) != 0)
    {
        goto err;
    }

    if (info->start_info.hStdInput == INVALID_HANDLE_VALUE
        && info->start_info.hStdOutput == INVALID_HANDLE_VALUE
        && info->start_info.hStdError == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    info->start_info.dwFlags |= STARTF_USESTDHANDLES;
    if ((ret = _ev_process_inherit_stdio(info)) != 0)
    {
        goto err;
    }

    return 0;

err:
    _ev_process_cleanup_start_info(info);
    return ret;
}

static void _ev_process_on_async_exit(ev_async_t* async)
{
    ev_process_t* process = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    if (process->exit_cb != NULL)
    {
        process->exit_cb(process);
    }
}

static void _ev_process_unregister_wait_handle(ev_process_t* process)
{
    DWORD status;
    if (process->backend.wait_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (!UnregisterWait(process->backend.wait_handle))
    {
        status = GetLastError();

        /**
         * https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-unregisterwait
         * According to remarks, ERROR_IO_PENDING does not means error.
         */
        if (status == ERROR_IO_PENDING)
        {
            goto fin;
        }
        process->exit_code = ev__translate_sys_error(status);
        EV_ABORT("GetLastError:%lu", (unsigned long)status);
    }

fin:
    process->backend.wait_handle = INVALID_HANDLE_VALUE;
}

static void _ev_process_on_sigchild_win(ev_async_t* async)
{
    DWORD status;
    ev_process_t* process = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    _ev_process_unregister_wait_handle(process);

    process->exit_status = EV_PROCESS_EXIT_NORMAL;
    if (GetExitCodeProcess(process->pid, &status))
    {
        process->exit_code = status;
    }
    else
    {
        status = GetLastError();
        process->exit_code = ev__translate_sys_error(status);
    }

	if (process->sigchild_cb != NULL)
	{
		process->sigchild_cb(process, process->exit_status, process->exit_code);
	}
}

static int _ev_process_setup_start_info(ev_startup_info_t* start_info,
    const ev_process_options_t* opt)
{
    int ret;
    ZeroMemory(start_info, sizeof(*start_info));
    start_info->start_info.cb = sizeof(start_info->start_info);
    start_info->start_info.hStdError = INVALID_HANDLE_VALUE;
    start_info->start_info.hStdOutput = INVALID_HANDLE_VALUE;
    start_info->start_info.hStdInput = INVALID_HANDLE_VALUE;

    ret = _dup_cmd(&start_info->cmdline, opt->argv);
    if (ret != 0)
    {
        return ret;
    }

    ret = _dup_envp(&start_info->envline, opt->envp);
    if (ret != 0)
    {
        goto err_free_cmdline;
    }

    ret = _ev_process_dup_stdio_win(start_info, opt);
    if (ret != 0)
    {
        goto err_free_envp;
    }

    return 0;

err_free_envp:
    _ev_process_cleanup_envp(start_info);
err_free_cmdline:
    _ev_process_cleanup_cmdline(start_info);
    return ret;
}

static VOID NTAPI _ev_process_on_object_exit(PVOID data, BOOLEAN didTimeout)
{
    ev_process_t* process = data;

    assert(didTimeout == FALSE); (void) didTimeout;
    assert(process != NULL);

    ev_async_wakeup(&process->sigchld);
}

static void _ev_process_init_win(ev_process_t* handle, const ev_process_options_t* opt)
{
    handle->sigchild_cb = opt->on_exit;
    handle->exit_cb = NULL;
    handle->pid = EV_OS_PID_INVALID;
    handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
    handle->exit_code = 0;
    handle->backend.wait_handle = INVALID_HANDLE_VALUE;
}

int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;
    DWORD errcode;
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ev_startup_info_t start_info;
    ret = _ev_process_setup_start_info(&start_info, opt);
    if (ret != 0)
    {
        return ret;
    }

    _ev_process_init_win(handle, opt);

    ret = ev_async_init(loop, &handle->sigchld, _ev_process_on_sigchild_win);
    if (ret != 0)
    {
        _ev_process_cleanup_start_info(&start_info);
        return ret;
    }

    ret = CreateProcessA(opt->file, start_info.cmdline, NULL, NULL, TRUE, 0,
        start_info.envline, opt->cwd, &start_info.start_info, &piProcInfo);

    handle->pid = piProcInfo.hProcess;
    _ev_process_cleanup_start_info(&start_info);

    if (!ret)
    {
        errcode = GetLastError();
        ev__async_exit_force(&handle->sigchld);
        return ev__translate_sys_error(errcode);
    }

    ret = RegisterWaitForSingleObject(&handle->backend.wait_handle, handle->pid,
        _ev_process_on_object_exit, handle, INFINITE, WT_EXECUTEINWAITTHREAD | WT_EXECUTEONLYONCE);
    if (!ret)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
    }

    CloseHandle(piProcInfo.hThread);
    piProcInfo.hThread = INVALID_HANDLE_VALUE;

    return 0;
}

void ev_process_exit(ev_process_t* handle, ev_process_exit_cb cb)
{
    DWORD errcode;
    _ev_process_unregister_wait_handle(handle);

    if (handle->pid != EV_OS_PID_INVALID)
    {
        if (CloseHandle(handle->pid) == 0)
        {
            errcode = GetLastError();
            EV_ABORT("errcode: %d", (int)errcode);
        }

        handle->pid = EV_OS_PID_INVALID;
    }

    handle->sigchild_cb = NULL;
    handle->exit_cb = cb;
    ev_async_exit(&handle->sigchld, _ev_process_on_async_exit);
}

ssize_t ev_getcwd(char* buffer, size_t size)
{
    DWORD errcode;
    DWORD wide_size = GetCurrentDirectoryW(0, NULL);
    if (wide_size == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    WCHAR* tmp_buf = ev_malloc(wide_size * sizeof(WCHAR));
    if (tmp_buf == NULL)
    {
        return EV_ENOMEM;
    }

    wide_size = GetCurrentDirectoryW(wide_size, tmp_buf);
    if (wide_size == 0)
    {
        errcode = GetLastError();
        ev_free(tmp_buf);
        return ev__translate_sys_error(errcode);
    }

    /* remove trailing slash */
    if (wide_size == 3 && tmp_buf[2] == L':' && tmp_buf[3] == L'\\')
    {
        wide_size = 2;
        tmp_buf[3] = L'\0';
    }

    /* check how many  */
    int required_size = WideCharToMultiByte(CP_UTF8, 0, tmp_buf, -1, NULL, 0,
        NULL, NULL);
    if (required_size == 0)
    {
        errcode = GetLastError();
        ev_free(tmp_buf);
        return ev__translate_sys_error(errcode);
    }

    int write_size = WideCharToMultiByte(CP_UTF8, 0, tmp_buf, -1, buffer,
        (int)size, NULL, NULL);
    ev_free(tmp_buf);

    if (write_size == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    if (write_size < required_size)
    {
        buffer[write_size - 1] = '\0';
    }

    return (ssize_t)write_size - 1;
}

ssize_t ev_exepath(char* buffer, size_t size)
{
    int utf8_len;
    int err;

    DWORD utf16_buffer_len = WIN32_UNICODE_PATH_MAX;
    WCHAR* utf16_buffer = (WCHAR*) ev_malloc(sizeof(WCHAR) * utf16_buffer_len);
    if (!utf16_buffer)
    {
        return EV_ENOMEM;
    }

    /* Get the path as UTF-16. */
    DWORD utf16_len = GetModuleFileNameW(NULL, utf16_buffer, utf16_buffer_len);
    if (utf16_len == 0)
    {
        err = GetLastError();
        goto error;
    }

    /* utf16_len contains the length, *not* including the terminating null. */
    utf16_buffer[utf16_len] = L'\0';

    /* Convert to UTF-8 */
    utf8_len = WideCharToMultiByte(CP_UTF8,
                                   0,
                                   utf16_buffer,
                                   -1,
                                   buffer,
                                   (int) size,
                                   NULL,
                                   NULL);
    if (utf8_len == 0)
    {
        err = GetLastError();
        goto error;
    }

    ev_free(utf16_buffer);

    /* utf8_len *does* include the terminating null at this point, but the
     * returned size shouldn't. */
    return utf8_len - 1;

error:
    if (buffer != NULL && size > 0)
    {
        *buffer = '\0';
    }
    ev_free(utf16_buffer);
    return ev__translate_sys_error(err);
}

#line 1 "src/win/sem_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */

void ev_sem_init(ev_sem_t* sem, unsigned value)
{
    DWORD errcode;
    sem->u.r = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (sem->u.r == NULL)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_exit(ev_sem_t* sem)
{
    DWORD errcode;
    if (!CloseHandle(sem->u.r))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    DWORD errcode;
    if (!ReleaseSemaphore(sem->u.r, 1, NULL))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    DWORD errcode;
    if (WaitForSingleObject(sem->u.r, INFINITE) != WAIT_OBJECT_0)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    DWORD ret = WaitForSingleObject(sem->u.r, 0);

    if (ret == WAIT_OBJECT_0)
    {
        return 0;
    }

    if (ret == WAIT_TIMEOUT)
    {
        return EV_EAGAIN;
    }

    DWORD errcode = GetLastError();
    EV_ABORT("ret:%lu, GetLastError:%lu", ret, errcode);
}

#line 1 "src/win/shmem_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "shmem.h" */

int ev_shm_init(ev_shm_t* shm, const char* key, size_t size)
{
    int err;

    shm->size = size;

    DWORD d_high = 0;
    DWORD d_low = (DWORD)size;
#if defined(_WIN64)
    if (d_low != size)
    {
        d_high = size >> 32;
    }
#endif

    shm->backend.map_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, d_high, d_low, key);
    if (shm->backend.map_file == NULL)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    shm->addr = MapViewOfFile(shm->backend.map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (shm->addr == NULL)
    {
        err = GetLastError();
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }

    return 0;
}

int ev_shm_open(ev_shm_t* shm, const char* key)
{
    int err;

    shm->backend.map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, key);
    if (shm->backend.map_file == NULL)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    shm->addr = MapViewOfFile(shm->backend.map_file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (shm->addr == NULL)
    {
        err = GetLastError();
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }

    MEMORY_BASIC_INFORMATION info;
    if (VirtualQuery(shm->addr, &info, sizeof(info)) == 0)
    {
        err = GetLastError();
        UnmapViewOfFile(shm->addr);
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }
    shm->size = info.RegionSize;

    return 0;
}

void ev_shm_exit(ev_shm_t* shm)
{
    if (!UnmapViewOfFile(shm->addr))
    {
        EV_ABORT("GetLastError:%lu", (unsigned long)GetLastError());
    }

    if (!CloseHandle(shm->backend.map_file))
    {
        EV_ABORT("GetLastError:%lu", (unsigned long)GetLastError());
    }
}

#line 1 "src/win/tcp_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "winsock.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "tcp_win.h" */
#include <WinSock2.h>
#include <assert.h>

static void _ev_tcp_close_socket(ev_tcp_t* sock)
{
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(sock->sock);
        sock->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_tcp_finialize_accept(ev_tcp_t* conn)
{
    ev_tcp_t* lisn = conn->backend.u.accept.listen;
    conn->backend.u.accept.listen = NULL;

    if (conn->backend.u.accept.stat == EV_EINPROGRESS)
    {
        conn->backend.u.accept.stat = EV_ECANCELED;
        ev_list_erase(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);
    }
    else
    {
        ev_list_erase(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);
    }

    ev__handle_event_dec(&lisn->base);
    ev__handle_event_dec(&conn->base);

    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb(lisn, conn, conn->backend.u.accept.stat);
}

static void _ev_tcp_cleanup_connection_in_listen(ev_tcp_t* conn)
{
    ev_tcp_t* lisn = conn->backend.u.accept.listen;

    ev__handle_event_dec(&lisn->base);
    ev__handle_event_dec(&conn->base);

    _ev_tcp_close_socket(conn);
    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb(lisn, conn, EV_ECANCELED);
}

static void _ev_tcp_cleanup_listen(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue)) != NULL)
    {
        ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_cleanup_connection_in_listen(conn);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue_done)) != NULL)
    {
        ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_cleanup_connection_in_listen(conn);
    }
}

static void _ev_tcp_w_user_callback_win(ev_tcp_t* sock, ev_tcp_write_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&sock->base);
    ev__write_exit(&req->base);
    req->user_callback(req, size);
}

static void _ev_tcp_r_user_callbak_win(ev_tcp_t* sock, ev_tcp_read_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&sock->base);
    ev__read_exit(&req->base);
    req->user_callback(req, size);
}

static void _ev_tcp_cleanup_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        _ev_tcp_r_user_callbak_win(sock, req, req->base.data.size);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        _ev_tcp_r_user_callbak_win(sock, req, EV_ECANCELED);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        _ev_tcp_w_user_callback_win(sock, req, req->base.size);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        _ev_tcp_w_user_callback_win(sock, req, EV_ECANCELED);
    }
}

static void _ev_tcp_cleanup_connect(ev_tcp_t* sock)
{
    if (sock->backend.u.client.stat == EV_EINPROGRESS)
    {
        sock->backend.u.client.stat = EV_ECANCELED;
    }
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_close_win(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);

    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_LISTING;
        _ev_tcp_cleanup_listen(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_finialize_accept(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_STREAMING;
        _ev_tcp_cleanup_stream(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        _ev_tcp_cleanup_connect(sock);
    }

    if (sock->close_cb != NULL)
    {
        sock->close_cb(sock);
    }
}

static int _ev_tcp_get_connectex(ev_tcp_t* sock, LPFN_CONNECTEX* fn)
{
    int ret;
    DWORD bytes;
    GUID wsaid = WSAID_CONNECTEX;

    ret = WSAIoctl(sock->sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &wsaid, sizeof(wsaid), fn, sizeof(*fn), &bytes, NULL, NULL);
    if (ret == 0)
    {
        return 0;
    }

    ret = WSAGetLastError();
    return ev__translate_sys_error(ret);
}

static int _ev_tcp_setup_sock(ev_tcp_t* sock, int af, int with_iocp)
{
    int ret;

    SOCKET os_sock = socket(af, SOCK_STREAM, 0);
    if (os_sock == INVALID_SOCKET)
    {
        goto err;
    }

    u_long yes = 1;
    if (ioctlsocket(os_sock, FIONBIO, &yes) == SOCKET_ERROR)
    {
        goto err;
    }

    if (!SetHandleInformation((HANDLE)os_sock, HANDLE_FLAG_INHERIT, 0))
    {
        goto err;
    }

    HANDLE iocp = sock->base.loop->backend.iocp;
    if (with_iocp && CreateIoCompletionPort((HANDLE)os_sock, iocp, os_sock, 0) == 0)
    {
        goto err;
    }

    sock->sock = os_sock;
    sock->backend.af = af;

    return 0;

err:
    ret = ev__translate_sys_error(WSAGetLastError());
    if (os_sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(os_sock);
    }
    return ret;
}

static void _ev_tcp_setup_listen_win(ev_tcp_t* sock)
{
    ev_list_init(&sock->backend.u.listen.a_queue);
    ev_list_init(&sock->backend.u.listen.a_queue_done);
    sock->base.data.flags |= EV_HANDLE_TCP_LISTING;
}

static void _ev_tcp_setup_accept_win(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    conn->backend.u.accept.cb = cb;
    conn->backend.u.accept.listen = lisn;
    conn->backend.u.accept.stat = EV_EINPROGRESS;
    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
}

static int _ev_tcp_setup_client_win(ev_tcp_t* sock, ev_tcp_connect_cb cb)
{
    int ret;
    if ((ret = _ev_tcp_get_connectex(sock, &sock->backend.u.client.fn_connectex)) != 0)
    {
        return ret;
    }

    sock->backend.u.client.stat = EV_EINPROGRESS;
    sock->backend.u.client.cb = cb;
    sock->base.data.flags |= EV_HANDLE_TCP_CONNECTING;

    return 0;
}

static void _ev_tcp_setup_stream_win(ev_tcp_t* sock)
{
    ev_list_init(&sock->backend.u.stream.r_queue);
    ev_list_init(&sock->backend.u.stream.r_queue_done);
    ev_list_init(&sock->backend.u.stream.w_queue);
    ev_list_init(&sock->backend.u.stream.w_queue_done);
    sock->base.data.flags |= EV_HANDLE_TCP_STREAMING;
}

static void _ev_tcp_process_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;

    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        size_t write_size = req->base.size;
        int write_stat = req->backend.stat;

        ssize_t ret = write_stat < 0 ? write_stat : write_size;
        _ev_tcp_w_user_callback_win(sock, req, ret);
    }

    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        size_t read_size = req->base.data.size;
        int read_stat = req->backend.stat;

        ssize_t ret = read_stat < 0 ? read_stat : read_size;
        _ev_tcp_r_user_callbak_win(sock, req, ret);
    }
}

static void _ev_tcp_process_connect(ev_tcp_t* sock)
{
    int ret;
    if (sock->backend.u.client.stat == 0)
    {
        if ((ret = setsockopt(sock->sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0)) == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            sock->backend.u.client.stat = ev__translate_sys_error(ret);
        }
    }

    ev__handle_event_dec(&sock->base);

    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_task_done(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);
    sock->backend.mask.todo_pending = 0;

    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        _ev_tcp_process_stream(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_finialize_accept(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        _ev_tcp_process_connect(sock);
    }
}

static void _ev_tcp_submit_stream_todo(ev_tcp_t* sock)
{
    if (sock->backend.mask.todo_pending)
    {
        return;
    }

    ev__backlog_submit(&sock->base, _ev_tcp_on_task_done);
    sock->backend.mask.todo_pending = 1;
}

static void _ev_tcp_on_accept_win(ev_tcp_t* conn, size_t transferred)
{
    (void)transferred;
    ev_tcp_t* lisn = conn->backend.u.accept.listen;

    ev_list_erase(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);
    ev_list_push_back(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);

    conn->backend.u.accept.stat = NT_SUCCESS(conn->backend.io.overlapped.Internal) ?
        0 : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)conn->backend.io.overlapped.Internal));
    _ev_tcp_submit_stream_todo(conn);
}

static void _ev_tcp_on_connect_win(ev_tcp_t* sock, size_t transferred)
{
    (void)transferred;

    sock->backend.u.client.stat = NT_SUCCESS(sock->backend.io.overlapped.Internal) ?
        0 : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)sock->backend.io.overlapped.Internal));
    _ev_tcp_submit_stream_todo(sock);
}

static int _ev_tcp_bind_any_addr(ev_tcp_t* sock, int af)
{
    size_t name_len;
    const struct sockaddr* bind_addr;

    switch (af)
    {
    case AF_INET:
        bind_addr = (struct sockaddr*)&ev_addr_ip4_any_;
        name_len = sizeof(ev_addr_ip4_any_);
        break;

    case AF_INET6:
        bind_addr = (struct sockaddr*)&ev_addr_ip6_any_;
        name_len = sizeof(ev_addr_ip6_any_);
        break;

    default:
        return EV_EINVAL;
    }

    return ev_tcp_bind(sock, bind_addr, name_len);
}

static void _ev_tcp_on_stream_write_done(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_tcp_write_req_t* req = arg;
    ev_tcp_t* sock = req->backend.owner;

    req->base.size = transferred;
    req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
        0 : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

    ev_list_erase(&sock->backend.u.stream.w_queue, &req->base.node);
    ev_list_push_back(&sock->backend.u.stream.w_queue_done, &req->base.node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_stream_read_done(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_tcp_read_req_t* req = arg;
    ev_tcp_t* sock = req->backend.owner;

    req->base.data.size = transferred;
    if (transferred == 0)
    {/* Zero recv means peer close */
        req->backend.stat = EV_EOF;
    }
    else
    {
        req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
            0 : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
    }

    ev_list_erase(&sock->backend.u.stream.r_queue, &req->base.node);
    ev_list_push_back(&sock->backend.u.stream.r_queue_done, &req->base.node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_iocp(ev_iocp_t* req, size_t transferred, void* arg)
{
    (void)req;
    ev_tcp_t* sock = arg;

    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_on_accept_win(sock, transferred);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        _ev_tcp_on_connect_win(sock, transferred);
    }
}

static int _ev_tcp_init_write_req_win(ev_tcp_t* sock, ev_tcp_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb)
{
    int ret;
    if ((ret = ev__write_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }

    req->user_callback = cb;
    req->backend.owner = sock;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_write_done, req);

    return 0;
}

static int _ev_tcp_init_read_req_win(ev_tcp_t* sock, ev_tcp_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    int ret;

    if ((ret = ev__read_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }

    req->user_callback = cb;
    req->backend.owner = sock;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_read_done, req);

    return 0;
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp)
{
    ev__handle_init(loop, &tcp->base, EV_ROLE_EV_TCP);
    tcp->close_cb = NULL;
    tcp->sock = EV_OS_SOCKET_INVALID;

    tcp->backend.af = AF_INET6;
    ev__iocp_init(&tcp->backend.io, _ev_tcp_on_iocp, tcp);
    memset(&tcp->backend.mask, 0, sizeof(tcp->backend.mask));

    return 0;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    sock->close_cb = cb;

    /* Close socket to avoid IOCP conflict with exiting process */
    _ev_tcp_close_socket(sock);

    /**
     * From this point, any complete IOCP operations should be in done_queue,
     * and any pending IOCP operations is canceled.
     */

    /* Ready to close socket */
    ev__handle_exit(&sock->base, _ev_tcp_on_close_win);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_socket = 0;

    if (tcp->base.data.flags & EV_HABDLE_TCP_BOUND)
    {
        return EV_EALREADY;
    }

    if (tcp->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(tcp, addr->sa_family, 1)) != 0)
        {
            return ret;
        }
        flag_new_socket = 1;
    }

    if ((ret = bind(tcp->sock, addr, (int)addrlen)) == SOCKET_ERROR)
    {
        ret = ev__translate_sys_error(WSAGetLastError());
        goto err;
    }
    tcp->base.data.flags |= EV_HABDLE_TCP_BOUND;

    return 0;

err:
    if (flag_new_socket)
    {
        _ev_tcp_close_socket(tcp);
    }
    return ret;
}

int ev_tcp_listen(ev_tcp_t* sock, int backlog)
{
    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        return EV_EADDRINUSE;
    }

    int ret;
    if ((ret = listen(sock->sock, backlog)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }
    _ev_tcp_setup_listen_win(sock);

    return 0;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (conn->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        return EV_EINPROGRESS;
    }

    if (conn->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(conn, lisn->backend.af, 1)) != 0)
        {
            goto err;
        }
        flag_new_sock = 1;
    }
    _ev_tcp_setup_accept_win(lisn, conn, cb);

    ev__handle_event_add(&lisn->base);
    ev__handle_event_add(&conn->base);

    DWORD bytes = 0;
    ret = AcceptEx(lisn->sock, conn->sock,
        conn->backend.u.accept.buffer, 0, sizeof(struct sockaddr_storage), sizeof(struct sockaddr_storage),
        &bytes, &conn->backend.io.overlapped);

    /* Accept success */
    if (ret)
    {
        conn->backend.u.accept.stat = 0;
        ev_list_push_back(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);
        _ev_tcp_submit_stream_todo(conn);
        return 0;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        ev__handle_event_dec(&lisn->base);
        ev__handle_event_dec(&conn->base);
        conn->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        return ev__translate_sys_error(ret);
    }
    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
    ev_list_push_back(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);

    return 0;

err:
    if (flag_new_sock)
    {
        _ev_tcp_close_socket(conn);
    }
    return ret;
}

int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    int socklen = (int)*len;

    if ((ret = getsockname(sock->sock, name, &socklen)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }

    *len = socklen;
    return 0;
}

int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    int socklen = (int)*len;

    if ((ret = getpeername(sock->sock, name, &socklen)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }

    *len = socklen;
    return 0;
}

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_tcp_connect_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        return EV_EINPROGRESS;
    }

    if (sock->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(sock, addr->sa_family, 1)) != 0)
        {
            goto err;
        }
        flag_new_sock = 1;
    }

    if (!(sock->base.data.flags & EV_HABDLE_TCP_BOUND))
    {
        if ((ret = _ev_tcp_bind_any_addr(sock, addr->sa_family)) != 0)
        {
            goto err;
        }
    }

    if ((ret = _ev_tcp_setup_client_win(sock, cb)) != 0)
    {
        goto err;
    }
    ev__handle_event_add(&sock->base);

    DWORD bytes;
    ret = sock->backend.u.client.fn_connectex(sock->sock, addr, (int)size,
        NULL, 0, &bytes, &sock->backend.io.overlapped);
    if (ret)
    {
        sock->backend.u.client.stat = 0;
        _ev_tcp_submit_stream_todo(sock);
        return 0;
    }

    ret = WSAGetLastError();
    if (ret != WSA_IO_PENDING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        ev__handle_event_dec(&sock->base);
        ret = ev__translate_sys_error(ret);
        goto err;
    }

    return 0;

err:
    if (flag_new_sock)
    {
        _ev_tcp_close_socket(sock);
    }
    return ret;
}

int ev_tcp_write(ev_tcp_t* sock, ev_tcp_write_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_tcp_write_cb cb)
{
    int ret;
    if ((ret = _ev_tcp_init_write_req_win(sock, req, bufs, nbuf, cb)) != 0)
    {
        return ret;
    }

    if (!(sock->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    ev_list_push_back(&sock->backend.u.stream.w_queue, &req->base.node);
    ev__handle_event_add(&sock->base);

    ret = WSASend(sock->sock, (WSABUF*)req->base.bufs, (DWORD)req->base.nbuf,
        NULL, 0, &req->backend.io.overlapped, NULL);
    if (ret == 0)
    {
        /*
         * A result of zero means send successful, but the result will still go
         * through IOCP callback, so it is necessary to return directly.
         */
        return 0;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        ev__handle_event_dec(&sock->base);
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_tcp_read(ev_tcp_t* sock, ev_tcp_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    int ret;

    if ((ret = _ev_tcp_init_read_req_win(sock, req, bufs, nbuf, cb)) != 0)
    {
        return ret;
    }

    if (!(sock->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    ev_list_push_back(&sock->backend.u.stream.r_queue, &req->base.node);
    ev__handle_event_add(&sock->base);

    DWORD flags = 0;
    ret = WSARecv(sock->sock, (WSABUF*)req->base.data.bufs, (DWORD)req->base.data.nbuf,
        NULL, &flags, &req->backend.io.overlapped, NULL);
    if (ret == 0)
    {
        /*
         * A result of zero means recv successful, but the result will still go
         * through IOCP callback, so it is necessary to return directly.
         */
        return 0;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        ev__handle_event_dec(&sock->base);
        return ev__translate_sys_error(ret);
    }

    return 0;
}

EV_LOCAL int ev__tcp_open_win(ev_tcp_t* tcp, SOCKET fd)
{
    tcp->sock = fd;
    if (!(tcp->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(tcp);
    }

    return 0;
}

#line 1 "src/win/thread_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "thread_win.h" */
#include <process.h>

typedef struct ev_thread_helper_win
{
    ev_thread_cb    cb;         /**< User thread body */
    void*           arg;        /**< User thread argument */
    HANDLE          start_sem;  /**< Start semaphore */
    HANDLE          thread_id;  /**< Thread handle */
}ev_thread_helper_win_t;

static size_t _ev_thread_calculate_stack_size_win(const ev_thread_opt_t* opt)
{
    if (opt == NULL || !opt->flags.have_stack_size)
    {
        return 0;
    }

    return opt->stack_size;
}

static unsigned __stdcall _ev_thread_proxy_proc_win(void* lpThreadParameter)
{
    DWORD errcode;
    ev_thread_helper_win_t* p_helper = lpThreadParameter;
    ev_thread_helper_win_t helper = *p_helper;

    ev_tls_set(&g_ev_loop_win_ctx.thread.thread_key, (void*)p_helper->thread_id);
    if (!ReleaseSemaphore(p_helper->start_sem, 1, NULL))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
    }

    helper.cb(helper.arg);
    return 0;
}

EV_LOCAL void ev__thread_init_win(void)
{
    int ret = ev_tls_init(&g_ev_loop_win_ctx.thread.thread_key);
    if (ret != 0)
    {
        EV_ABORT("ret:%d", ret);
    }
}

int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg)
{
    DWORD err = 0;
    ev__init_once_win();

    ev_thread_helper_win_t helper;
    helper.cb = cb;
    helper.arg = arg;
    if ((helper.start_sem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL)
    {
        err = GetLastError();
        goto err_fin;
    }

    size_t stack_size = _ev_thread_calculate_stack_size_win(opt);
    helper.thread_id = (HANDLE)_beginthreadex(NULL, (unsigned)stack_size,
        _ev_thread_proxy_proc_win, &helper, CREATE_SUSPENDED, NULL);
    if (helper.thread_id == NULL)
    {
        err = GetLastError();
        goto err_create_thread;
    }

    if (ResumeThread(helper.thread_id) == -1)
    {
        err = GetLastError();
        EV_ABORT("GetLastError:%lu", err);
    }

    int ret = WaitForSingleObject(helper.start_sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        err = (ret != WAIT_FAILED) ? ERROR_INVALID_PARAMETER : GetLastError();
        goto err_create_thread;
    }

    *thr = helper.thread_id;

err_create_thread:
    CloseHandle(helper.start_sem);
err_fin:
    return ev__translate_sys_error(err);
}

int ev_thread_exit(ev_os_thread_t* thr, unsigned long timeout)
{
    int ret = WaitForSingleObject(*thr, timeout);
    switch (ret)
    {
    case WAIT_TIMEOUT:
        return EV_ETIMEDOUT;
    case WAIT_ABANDONED:
        EV_ABORT("WAIT_ABANDONED"); // should not happen
        break;
    case WAIT_FAILED:
        ret = GetLastError();
        return ret == WAIT_TIMEOUT ? EV_ETIMEDOUT : ev__translate_sys_error(ret);
    default:
        break;
    }

    CloseHandle(*thr);
    *thr = NULL;

    return 0;
}

ev_os_thread_t ev_thread_self(void)
{
    ev__init_once_win();
    return ev_tls_get(&g_ev_loop_win_ctx.thread.thread_key);
}

ev_os_tid_t ev_thread_id(void)
{
    return GetCurrentThreadId();
}

int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2)
{
    return *t1 == *t2;
}

void ev_thread_sleep(uint32_t timeout)
{
    Sleep(timeout);
}

int ev_tls_init(ev_tls_t* tls)
{
    int err;
    if ((tls->tls = TlsAlloc()) == TLS_OUT_OF_INDEXES)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

void ev_tls_exit(ev_tls_t* tls)
{
    DWORD errcode;
    if (TlsFree(tls->tls) == FALSE)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
    tls->tls = TLS_OUT_OF_INDEXES;
}

void ev_tls_set(ev_tls_t* tls, void* val)
{
    DWORD errcode;
    if (TlsSetValue(tls->tls, val) == FALSE)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void* ev_tls_get(ev_tls_t* tls)
{
    DWORD errcode;
    void* val = TlsGetValue(tls->tls);
    if (val == NULL)
    {
        if ((errcode = GetLastError()) != ERROR_SUCCESS)
        {
            EV_ABORT("GetLastError:%lu", errcode);
        }
    }
    return val;
}

#line 1 "src/win/threadpool_win.c"
/* AMALGAMATE: #include "threadpool_win.h" */
/* AMALGAMATE: #include "loop_win.h" */

static void _on_work_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;

    ev_loop_t* loop = EV_CONTAINER_OF(iocp, ev_loop_t, backend.threadpool.io);

    ev__threadpool_process(loop);
}

EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop)
{
    ev__iocp_post(loop, &loop->backend.threadpool.io);
}

EV_LOCAL void ev__threadpool_init_win(ev_loop_t* loop)
{
    ev__iocp_init(&loop->backend.threadpool.io, _on_work_win, NULL);
}

EV_LOCAL void ev__threadpool_exit_win(ev_loop_t* loop)
{
    (void)loop;
}

#line 1 "src/win/udp_win.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc.h" */
/* AMALGAMATE: #include "loop_win.h" */
/* AMALGAMATE: #include "winsock.h" */
/* AMALGAMATE: #include "misc_win.h" */
/* AMALGAMATE: #include "udp_win.h" */
/* AMALGAMATE: #include "handle.h" */
#include <assert.h>

static int _ev_udp_setup_socket_attribute_win(ev_loop_t* loop, ev_udp_t* udp, int family)
{
    DWORD yes = 1;
    WSAPROTOCOL_INFOW info;
    int opt_len;
    int ret;

    assert(udp->sock != EV_OS_SOCKET_INVALID);

    /* Set the socket to nonblocking mode */
    if (ioctlsocket(udp->sock, FIONBIO, &yes) == SOCKET_ERROR)
    {
        ret = WSAGetLastError();
        goto err;
    }

    /* Make the socket non-inheritable */
    if (!SetHandleInformation((HANDLE)udp->sock, HANDLE_FLAG_INHERIT, 0))
    {
        ret = GetLastError();
        goto err;
    }

    /**
     * Associate it with the I/O completion port. Use uv_handle_t pointer as
     * completion key.
     */
    if (CreateIoCompletionPort((HANDLE)udp->sock, loop->backend.iocp, (ULONG_PTR)udp->sock, 0) == NULL)
    {
        ret = GetLastError();
        goto err;
    }

    /*
     * All known Windows that support SetFileCompletionNotificationModes have a
     * bug that makes it impossible to use this function in conjunction with
     * datagram sockets. We can work around that but only if the user is using
     * the default UDP driver (AFD) and has no other. LSPs stacked on top. Here
     * we check whether that is the case.
     */
    opt_len = sizeof(info);
    if (getsockopt(udp->sock, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&info, &opt_len) == SOCKET_ERROR)
    {
        ret = GetLastError();
        goto err;
    }
    if (info.ProtocolChain.ChainLen == 1)
    {
        if (SetFileCompletionNotificationModes((HANDLE)udp->sock,
            FILE_SKIP_SET_EVENT_ON_HANDLE | FILE_SKIP_COMPLETION_PORT_ON_SUCCESS))
        {
            udp->base.data.flags |= EV_HANDLE_UDP_BYPASS_IOCP;
            udp->backend.fn_wsarecv = ev__wsa_recv_workaround;
            udp->backend.fn_wsarecvfrom = ev__wsa_recvfrom_workaround;
        }
        else if ((ret = GetLastError()) != ERROR_INVALID_FUNCTION)
        {
            goto err;
        }
    }

    if (family == AF_INET6)
    {
        udp->base.data.flags |= EV_HANDLE_UDP_IPV6;
    }
    else
    {
        assert(!(udp->base.data.flags & EV_HANDLE_UDP_IPV6));
    }

    return 0;

err:
    return ev__translate_sys_error(ret);
}

static void _ev_udp_on_close_win(ev_handle_t* handle)
{
    ev_udp_t* udp = EV_CONTAINER_OF(handle, ev_udp_t, base);
    if (udp->close_cb != NULL)
    {
        udp->close_cb(udp);
    }
}

static int _ev_udp_is_bound_win(ev_udp_t* udp)
{
    struct sockaddr_storage addr;
    size_t addrlen = sizeof(addr);

    int ret = ev_udp_getsockname(udp, (struct sockaddr*)&addr, &addrlen);
    return ret == 0 && addrlen > 0;
}

static int _ev_udp_is_connected_win(ev_udp_t* udp)
{
    struct sockaddr_storage addr;
    size_t addrlen = sizeof(addr);

    int ret = ev_udp_getpeername(udp, (struct sockaddr*)&addr, &addrlen);
    return ret == 0 && addrlen > 0;
}

static int _ev_udp_maybe_deferred_socket_win(ev_udp_t* udp, int domain)
{
    int ret;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return 0;
    }

    if ((udp->sock = socket(domain, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    if ((ret = _ev_udp_setup_socket_attribute_win(udp->base.loop, udp, domain)) != 0)
    {
        closesocket(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
        return ret;
    }

    return 0;
}

static void _ev_udp_close_socket_win(ev_udp_t* udp)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
    }
}

static int _ev_udp_disconnect_win(ev_udp_t* udp)
{
    struct sockaddr addr;
    memset(&addr, 0, sizeof(addr));

    int ret = connect(udp->sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    udp->base.data.flags &= ~EV_HANDLE_UDP_CONNECTED;
    return 0;
}

static int _ev_udp_maybe_deferred_bind_win(ev_udp_t* udp, int domain)
{
    if (udp->base.data.flags & EV_HANDLE_UDP_BOUND)
    {
        return 0;
    }

    struct sockaddr* bind_addr;
    if (domain == AF_INET)
    {
        bind_addr = (struct sockaddr*)&ev_addr_ip4_any_;
    }
    else if (domain == AF_INET6)
    {
        bind_addr = (struct sockaddr*)&ev_addr_ip6_any_;
    }
    else
    {
        return EV_EINVAL;
    }

    return ev_udp_bind(udp, bind_addr, 0);
}

static int _ev_udp_set_membership_ipv4_win(ev_udp_t* udp,
    const struct sockaddr_in* multicast_addr, const char* interface_addr,
    ev_udp_membership_t membership)
{
    int ret;
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr)
    {
        if (inet_pton(AF_INET, interface_addr, &mreq.imr_interface.s_addr) != 1)
        {
            ret = WSAGetLastError();
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }

    mreq.imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static int _ev_udp_set_membership_ipv6_win(ev_udp_t* udp,
    const struct sockaddr_in6* multicast_addr, const char* interface_addr,
    ev_udp_membership_t membership)
{
    int ret;
    struct ipv6_mreq mreq;
    struct sockaddr_in6 addr6;

    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr)
    {
        if (ev_ipv6_addr(interface_addr, 0, &addr6))
        {
            return EV_EINVAL;
        }
        mreq.ipv6mr_interface = addr6.sin6_scope_id;
    }
    else
    {
        mreq.ipv6mr_interface = 0;
    }
    mreq.ipv6mr_multiaddr = multicast_addr->sin6_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static int _ev_udp_set_source_membership_ipv4(ev_udp_t* udp,
    const struct sockaddr_in* multicast_addr, const char* interface_addr,
    const struct sockaddr_in* source_addr, ev_udp_membership_t membership)
{
    int err;
    struct ip_mreq_source mreq;
    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr != NULL)
    {
        if (inet_pton(AF_INET, interface_addr, &mreq.imr_interface.s_addr) != 1)
        {
            err = WSAGetLastError();
            return ev__translate_sys_error(err);
        }
    }
    else
    {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }

    mreq.imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr->sin_addr.s_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;

    if (setsockopt(udp->sock, IPPROTO_IP, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

static int _ev_udp_set_source_membership_ipv6(ev_udp_t* udp,
    const struct sockaddr_in6* multicast_addr, const char* interface_addr,
    const struct sockaddr_in6* source_addr, ev_udp_membership_t membership)
{
    int ret;
    struct group_source_req mreq;
    struct sockaddr_in6 addr6;

    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr != NULL)
    {
        if ((ret = ev_ipv6_addr(interface_addr, 0, &addr6)) != 0)
        {
            return ret;
        }
        mreq.gsr_interface = addr6.sin6_scope_id;
    }
    else
    {
        mreq.gsr_interface = 0;
    }

    memcpy(&mreq.gsr_group, multicast_addr, sizeof(*multicast_addr));
    memcpy(&mreq.gsr_source, source_addr, sizeof(*source_addr));

    int optname = membership == EV_UDP_ENTER_GROUP ? MCAST_JOIN_SOURCE_GROUP : MCAST_LEAVE_SOURCE_GROUP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static void _ev_udp_w_user_callback_win(ev_udp_write_t* req, ssize_t size)
{
    ev_udp_t* udp = req->backend.owner;
    ev__handle_event_dec(&udp->base);

    ev__write_exit(&req->base);
    ev__handle_exit(&req->handle, NULL);

    req->usr_cb(req, size);
}

static void _ev_udp_r_user_callback_win(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    ev_udp_t* udp = req->backend.owner;
    ev__handle_event_dec(&udp->base);

    ev__read_exit(&req->base);
    ev__handle_exit(&req->handle, NULL);

    req->usr_cb(req, addr, size);
}

static void _ev_udp_on_send_complete_win(ev_udp_t* udp, ev_udp_write_t* req)
{
    ev_list_erase(&udp->send_list, &req->base.node);

    ssize_t result = req->backend.stat;
    if (result >= 0)
    {
        result = req->base.size;
    }
    _ev_udp_w_user_callback_win(req, result);
}

static void _ev_udp_on_send_bypass_iocp(ev_handle_t* handle)
{
    ev_udp_write_t* req = EV_CONTAINER_OF(handle, ev_udp_write_t, handle);
    ev_udp_t* udp = req->backend.owner;

    _ev_udp_on_send_complete_win(udp, req);
}

static void _ev_udp_on_send_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_udp_t* udp = arg;
    ev_udp_write_t* req = EV_CONTAINER_OF(iocp, ev_udp_write_t, backend.io);

    req->base.size = transferred;
    req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
        0 : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

    _ev_udp_on_send_complete_win(udp, req);
}

static void _ev_udp_do_recv_win(ev_udp_t* udp, ev_udp_read_t* req)
{
    DWORD recv_bytes;
    DWORD flags = 0;
    socklen_t peer_addr_len = sizeof(req->addr);

    int ret = WSARecvFrom(udp->sock, (WSABUF*)req->base.data.bufs, (DWORD)req->base.data.nbuf,
        &recv_bytes, &flags, (struct sockaddr*)&req->addr, &peer_addr_len, NULL, NULL);
    if (ret != SOCKET_ERROR)
    {
        req->base.data.size = recv_bytes;
        req->backend.stat = 0;
    }
    else
    {
        ret = WSAGetLastError();
        req->backend.stat = ev__translate_sys_error(ret);
    }

    ev_list_erase(&udp->recv_list, &req->base.node);

    ssize_t recv_ret = req->backend.stat;
    struct sockaddr* peer_addr = NULL;

    if (recv_ret >= 0)
    {
        recv_ret = req->base.data.size;
        peer_addr = (struct sockaddr*)&req->addr;
    }

    _ev_udp_r_user_callback_win(req, peer_addr, recv_ret);
}

static void _ev_udp_on_recv_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_udp_t* udp = arg;
    ev_udp_read_t* req = EV_CONTAINER_OF(iocp, ev_udp_read_t, backend.io);

    _ev_udp_do_recv_win(udp, req);
}

static void _ev_udp_on_recv_bypass_iocp_win(ev_handle_t* handle)
{
    ev_udp_read_t* req = EV_CONTAINER_OF(handle, ev_udp_read_t, handle);
    ev_udp_t* udp = req->backend.owner;

    _ev_udp_do_recv_win(udp, req);
}

static int _ev_udp_maybe_bind_win(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags)
{
    int ret;
    if (udp->base.data.flags & EV_HANDLE_UDP_BOUND)
    {
        return EV_EALREADY;
    }

    if ((flags & EV_UDP_IPV6_ONLY) && addr->sa_family != AF_INET6)
    {
        return EV_EINVAL;
    }

    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        if ((udp->sock = socket(addr->sa_family, SOCK_DGRAM, 0)) == EV_OS_SOCKET_INVALID)
        {
            ret = WSAGetLastError();
            return ev__translate_sys_error(ret);
        }

        ret = _ev_udp_setup_socket_attribute_win(udp->base.loop, udp, addr->sa_family);
        if (ret != 0)
        {
            _ev_udp_close_socket_win(udp);
            return ret;
        }
    }

    if (flags & EV_UDP_REUSEADDR)
    {
        if ((ret = ev__reuse_win(udp->sock, 1)) != 0)
        {
            return ret;
        }
    }

    if (addr->sa_family == AF_INET6)
    {
        udp->base.data.flags |= EV_HANDLE_UDP_IPV6;
        if (flags & EV_UDP_IPV6_ONLY)
        {
            if ((ret = ev__ipv6only_win(udp->sock, 1)) != 0)
            {
                _ev_udp_close_socket_win(udp);
                return ret;
            }
        }
    }

    socklen_t addrlen = ev__get_addr_len(addr);
    if ((ret = bind(udp->sock, addr, addrlen)) == SOCKET_ERROR)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    udp->base.data.flags |= EV_HANDLE_UDP_BOUND;
    return 0;
}

EV_LOCAL int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req)
{
    WSABUF buf;
    buf.buf = g_ev_loop_win_ctx.net.zero_;
    buf.len = 0;

    DWORD bytes = 0;
    DWORD flags = MSG_PEEK;

    req->backend.owner = udp;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_udp_on_recv_iocp_win, udp);

    ev__handle_event_add(&udp->base);

    int ret = WSARecv(udp->sock, &buf, 1, &bytes, &flags, &req->backend.io.overlapped, NULL);
    if (ret == 0 && (udp->base.data.flags & EV_HANDLE_UDP_BYPASS_IOCP))
    {
        ev__backlog_submit(&req->handle, _ev_udp_on_recv_bypass_iocp_win);
        return 0;
    }

    int err;
    if (ret == 0 || (err = WSAGetLastError()) == ERROR_IO_PENDING)
    {
        return 0;
    }

    ev__handle_event_dec(&udp->base);
    return ev__translate_sys_error(err);
}

EV_LOCAL int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req,
    const struct sockaddr* addr, socklen_t addrlen)
{
    int ret, err;

    if (!(udp->base.data.flags & EV_HANDLE_UDP_BOUND))
    {
        if (addr == NULL)
        {
            return EV_EINVAL;
        }

        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, addr->sa_family)) != 0)
        {
            return ret;
        }
    }

    req->backend.owner = udp;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_udp_on_send_iocp_win, udp);

    DWORD send_bytes;

    ev__handle_event_add(&udp->base);
    ret = WSASendTo(udp->sock, (WSABUF*)req->base.bufs, (DWORD)req->base.nbuf,
        &send_bytes, 0, addr, addrlen, &req->backend.io.overlapped, NULL);

    if (ret == 0 && (udp->base.data.flags & EV_HANDLE_UDP_BYPASS_IOCP))
    {
        req->base.size += req->base.capacity;
        req->backend.stat = 0;
        ev__backlog_submit(&req->handle, _ev_udp_on_send_bypass_iocp);
        return 0;
    }

    if (ret == 0 || (err = GetLastError()) == ERROR_IO_PENDING)
    {
        req->backend.stat = EV_EINPROGRESS;
        return 0;
    }

    ev__handle_event_dec(&udp->base);
    return ev__translate_sys_error(err);
}

int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain)
{
    int err;

    udp->sock = EV_OS_SOCKET_INVALID;
    udp->close_cb = NULL;
    ev_list_init(&udp->send_list);
    ev_list_init(&udp->recv_list);
    ev__handle_init(loop, &udp->base, EV_ROLE_EV_UDP);

    udp->backend.fn_wsarecv = WSARecv;
    udp->backend.fn_wsarecvfrom = WSARecvFrom;

    if (domain != AF_UNSPEC)
    {
        if ((err = _ev_udp_maybe_deferred_socket_win(udp, domain)) != 0)
        {
            ev__handle_exit(&udp->base, NULL);
            return err;
        }
    }

    return 0;
}

void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb)
{
    udp->close_cb = close_cb;

    _ev_udp_close_socket_win(udp);
    ev__handle_exit(&udp->base, _ev_udp_on_close_win);
}

int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock)
{
    int ret;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_EBUSY;
    }

    WSAPROTOCOL_INFOW protocol_info;
    int opt_len = sizeof(protocol_info);
    if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&protocol_info, &opt_len) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    udp->sock = sock;
    if ((ret = _ev_udp_setup_socket_attribute_win(udp->base.loop, udp,
        protocol_info.iAddressFamily)) != 0)
    {
        udp->sock = EV_OS_SOCKET_INVALID;
        return ret;
    }

    if (_ev_udp_is_bound_win(udp))
    {
        udp->base.data.flags |= EV_HANDLE_UDP_BOUND;
    }

    if (_ev_udp_is_connected_win(udp))
    {
        udp->base.data.flags |= EV_HANDLE_UDP_CONNECTED;
    }

    return 0;
}

int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags)
{
    return _ev_udp_maybe_bind_win(udp, addr, flags);
}

int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr)
{
    int ret;
    if (addr == NULL)
    {
        if (!(udp->base.data.flags & EV_HANDLE_UDP_CONNECTED))
        {
            return EV_ENOTCONN;
        }

        return _ev_udp_disconnect_win(udp);
    }

    if (!(udp->base.data.flags & EV_HANDLE_UDP_BOUND))
    {
        struct sockaddr* bind_addr = addr->sa_family == AF_INET ?
            (struct sockaddr*)&ev_addr_ip4_any_ : (struct sockaddr*)&ev_addr_ip6_any_;
        if ((ret = _ev_udp_maybe_bind_win(udp, bind_addr, 0)) != 0)
        {
            return ret;
        }
    }

    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    if ((ret = connect(udp->sock, addr, addrlen)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    int wrap_len = (int)*len;
    if (getsockname(udp->sock, name, &wrap_len) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return 0;
}

int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    int wrap_len = (int)*len;
    if (getpeername(udp->sock, name, &wrap_len) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return 0;
}

int ev_udp_set_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    if ((ret = ev_ipv4_addr(multicast_addr, 0, (struct sockaddr_in*)&addr)) == 0)
    {
        if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET)) != 0)
        {
            return ret;
        }
        return _ev_udp_set_membership_ipv4_win(udp, (struct sockaddr_in*)&addr,
            interface_addr, membership);
    }

    if ((ret = ev_ipv6_addr(multicast_addr, 0, (struct sockaddr_in6*)&addr)) == 0)
    {
        if ((udp->base.data.flags & EV_HANDLE_UDP_BOUND) && !(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET6)) != 0)
        {
            return ret;
        }
        return _ev_udp_set_membership_ipv6_win(udp, (struct sockaddr_in6*)&addr,
            interface_addr, membership);
    }

    return ret;
}

int ev_udp_set_source_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, const char* source_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage mcast_addr;
    struct sockaddr_storage src_addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    if ((ret = ev_ipv4_addr(multicast_addr, 0, (struct sockaddr_in*)&mcast_addr)) == 0)
    {
        if ((ret = ev_ipv4_addr(source_addr, 0, (struct sockaddr_in*)&src_addr)) != 0)
        {
            return ret;
        }
        if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET)) != 0)
        {
            return ret;
        }
        return _ev_udp_set_source_membership_ipv4(udp, (struct sockaddr_in*)&mcast_addr,
            interface_addr, (struct sockaddr_in*)&src_addr, membership);
    }

    if ((ret = ev_ipv6_addr(multicast_addr, 0, (struct sockaddr_in6*)&mcast_addr)) == 0)
    {
        if ((ret = ev_ipv6_addr(source_addr, 0, (struct sockaddr_in6*)&src_addr)) != 0)
        {
            return ret;
        }
        if ((udp->base.data.flags & EV_HANDLE_UDP_BOUND) && !(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET6)) != 0)
        {
            return ret;
        }
        return _ev_udp_set_source_membership_ipv6(udp, (struct sockaddr_in6*)&mcast_addr,
            interface_addr, (struct sockaddr_in6*)&src_addr, membership);
    }

    return ret;
}

int ev_udp_set_multicast_loop(ev_udp_t* udp, int on)
{
    DWORD optval = on;
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_LOOP;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_LOOP;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl)
{
    DWORD optval = (DWORD)ttl;
    if (ttl < -1 || ttl > 255)
    {
        return EV_EINVAL;
    }
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_TTL;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_HOPS;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr)
{
    int ret;
    struct sockaddr_storage addr_st;
    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr_st;
    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr_st;

    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int is_ipv6 = udp->base.data.flags & EV_HANDLE_UDP_IPV6;
    if ((ret = ev__udp_interface_addr_to_sockaddr(&addr_st, interface_addr, is_ipv6)) != 0)
    {
        return ret;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_IF;
    char* optval = (char*)&addr_4->sin_addr;
    int optlen = sizeof(addr_4->sin_addr);
    if (addr_st.ss_family == AF_INET6)
    {
        optval = (char*)&addr_6->sin6_scope_id;
        optlen = sizeof(addr_6->sin6_scope_id);
    }

    if (setsockopt(udp->sock, level, optname, optval, optlen) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_udp_set_broadcast(ev_udp_t* udp, int on)
{
    BOOL optval = !!on;

    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    if (setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

int ev_udp_set_ttl(ev_udp_t* udp, int ttl)
{
    DWORD optval = ttl;
    if (optval < 1 || optval > 255)
    {
        return EV_EINVAL;
    }
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_TTL;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_HOPLIMIT;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

#line 1 "src/win/winapi.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "winapi.h" */
#include <assert.h>

ev_winapi_t ev_winapi = {
    NULL,
    NULL,
    NULL,
};

EV_LOCAL void ev__winapi_init(void)
{
#define GET_NTDLL_FUNC(name)  \
    do {\
        ev_winapi.name = (fn_##name)GetProcAddress(ntdll_modeule, #name);\
        assert(ev_winapi.name != NULL);\
    } while (0)

    HMODULE ntdll_modeule = GetModuleHandle("ntdll.dll");
    assert(ntdll_modeule != NULL);

    GET_NTDLL_FUNC(NtQueryInformationFile);
    GET_NTDLL_FUNC(RtlNtStatusToDosError);
    GET_NTDLL_FUNC(NtQueryVolumeInformationFile);
    GET_NTDLL_FUNC(NtDeviceIoControlFile);

#undef GET_NTDLL_FUNC
}

#line 1 "src/win/winsock.c"
/* AMALGAMATE: #include "winsock.h" */
/* AMALGAMATE: #include "misc_win.h" */

int ev_tcp_non_ifs_lsp_ipv4;
int ev_tcp_non_ifs_lsp_ipv6;

struct sockaddr_in ev_addr_ip4_any_;
struct sockaddr_in6 ev_addr_ip6_any_;

EV_LOCAL void ev__winsock_init(void)
{
#define DETECT_IFS_LSP(AF, flag)    \
    do {\
        flag = 1;\
        SOCKET dummy = socket(AF, SOCK_STREAM, IPPROTO_IP);\
        if (dummy != INVALID_SOCKET) {\
            break;\
        }\
        WSAPROTOCOL_INFOW protocol_info;\
        int opt_len = sizeof(protocol_info);\
        if (getsockopt(dummy, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&protocol_info, &opt_len) == 0) {\
            if (protocol_info.dwServiceFlags1 & XP1_IFS_HANDLES) {\
                flag = 0;\
            }\
        }\
        closesocket(dummy);\
    } while (0)

    int ret; (void)ret;

    /* Set implicit binding address used by connectEx */
    if ((ret = ev_ipv4_addr("0.0.0.0", 0, &ev_addr_ip4_any_)) != 0)
    {
        abort();
    }
    if ((ret = ev_ipv6_addr("::", 0, &ev_addr_ip6_any_)) != 0)
    {
        abort();
    }

    /* Skip initialization in safe mode without network support */
    if (GetSystemMetrics(SM_CLEANBOOT) == 1)
    {
        return;
    }

    /* Initialize winsock */
    {
        WSADATA wsa_data;
        if ((ret = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0)
        {
            EV_FATAL_SYSCALL(ret, "WSAStartup");
        }
    }
    
    /* Try to detect non-IFS LSPs */
    DETECT_IFS_LSP(AF_INET, ev_tcp_non_ifs_lsp_ipv4);
    /* Try to detect IPV6 support and non-IFS LSPs */
    DETECT_IFS_LSP(AF_INET6, ev_tcp_non_ifs_lsp_ipv6);

#undef DETECT_IFS_LSP
}

EV_LOCAL int WSAAPI ev__wsa_recv_workaround(SOCKET socket, WSABUF* buffers,
    DWORD buffer_count, DWORD* bytes, DWORD* flags, WSAOVERLAPPED* overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine)
{
	NTSTATUS status;
	void* apc_context;
	IO_STATUS_BLOCK* iosb = (IO_STATUS_BLOCK*)&overlapped->Internal;
	AFD_RECV_INFO info;
	DWORD error;

	if (overlapped == NULL || completion_routine != NULL) {
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}

	info.BufferArray = buffers;
	info.BufferCount = buffer_count;
	info.AfdFlags = AFD_OVERLAPPED;
	info.TdiFlags = TDI_RECEIVE_NORMAL;

	if (*flags & MSG_PEEK) {
		info.TdiFlags |= TDI_RECEIVE_PEEK;
	}

	if (*flags & MSG_PARTIAL) {
		info.TdiFlags |= TDI_RECEIVE_PARTIAL;
	}

	if (!((intptr_t)overlapped->hEvent & 1)) {
		apc_context = (void*)overlapped;
	}
	else {
		apc_context = NULL;
	}

	iosb->Status = STATUS_PENDING;
	iosb->Pointer = 0;

	status = ev_winapi.NtDeviceIoControlFile((HANDLE)socket,
		overlapped->hEvent,
		NULL,
		apc_context,
		iosb,
		IOCTL_AFD_RECEIVE,
		&info,
		sizeof(info),
		NULL,
		0);

	*flags = 0;
	*bytes = (DWORD)iosb->Information;

	switch (status) {
	case STATUS_SUCCESS:
		error = ERROR_SUCCESS;
		break;

	case STATUS_PENDING:
		error = WSA_IO_PENDING;
		break;

	case STATUS_BUFFER_OVERFLOW:
		error = WSAEMSGSIZE;
		break;

	case STATUS_RECEIVE_EXPEDITED:
		error = ERROR_SUCCESS;
		*flags = MSG_OOB;
		break;

	case STATUS_RECEIVE_PARTIAL_EXPEDITED:
		error = ERROR_SUCCESS;
		*flags = MSG_PARTIAL | MSG_OOB;
		break;

	case STATUS_RECEIVE_PARTIAL:
		error = ERROR_SUCCESS;
		*flags = MSG_PARTIAL;
		break;

	default:
		error = ev__ntstatus_to_winsock_error(status);
		break;
	}

	WSASetLastError(error);

	if (error == ERROR_SUCCESS)
    {
		return 0;
	}

	return SOCKET_ERROR;
}

EV_LOCAL int WSAAPI ev__wsa_recvfrom_workaround(SOCKET socket, WSABUF* buffers,
    DWORD buffer_count, DWORD* bytes, DWORD* flags, struct sockaddr* addr,
    int* addr_len, WSAOVERLAPPED* overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine)
{
    NTSTATUS status;
    void* apc_context;
    IO_STATUS_BLOCK* iosb = (IO_STATUS_BLOCK*)&overlapped->Internal;
    AFD_RECV_DATAGRAM_INFO info;
    DWORD error;

    if (overlapped == NULL || addr == NULL || addr_len == NULL ||
        completion_routine != NULL)
    {
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }

    info.BufferArray = buffers;
    info.BufferCount = buffer_count;
    info.AfdFlags = AFD_OVERLAPPED;
    info.TdiFlags = TDI_RECEIVE_NORMAL;
    info.Address = addr;
    info.AddressLength = addr_len;

    if (*flags & MSG_PEEK) {
        info.TdiFlags |= TDI_RECEIVE_PEEK;
    }

    if (*flags & MSG_PARTIAL) {
        info.TdiFlags |= TDI_RECEIVE_PARTIAL;
    }

    if (!((intptr_t)overlapped->hEvent & 1)) {
        apc_context = (void*)overlapped;
    }
    else {
        apc_context = NULL;
    }

    iosb->Status = STATUS_PENDING;
    iosb->Pointer = 0;

    status = ev_winapi.NtDeviceIoControlFile((HANDLE)socket,
        overlapped->hEvent,
        NULL,
        apc_context,
        iosb,
        IOCTL_AFD_RECEIVE_DATAGRAM,
        &info,
        sizeof(info),
        NULL,
        0);

    *flags = 0;
    *bytes = (DWORD)iosb->Information;

    switch (status)
    {
    case STATUS_SUCCESS:
        error = ERROR_SUCCESS;
        break;

    case STATUS_PENDING:
        error = WSA_IO_PENDING;
        break;

    case STATUS_BUFFER_OVERFLOW:
        error = WSAEMSGSIZE;
        break;

    case STATUS_RECEIVE_EXPEDITED:
        error = ERROR_SUCCESS;
        *flags = MSG_OOB;
        break;

    case STATUS_RECEIVE_PARTIAL_EXPEDITED:
        error = ERROR_SUCCESS;
        *flags = MSG_PARTIAL | MSG_OOB;
        break;

    case STATUS_RECEIVE_PARTIAL:
        error = ERROR_SUCCESS;
        *flags = MSG_PARTIAL;
        break;

    default:
        error = ev__ntstatus_to_winsock_error(status);
        break;
    }

    WSASetLastError(error);

    if (error == ERROR_SUCCESS) {
        return 0;
    }

    return SOCKET_ERROR;
}

EV_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status)
{
    switch (status)
    {
    case STATUS_SUCCESS:
        return ERROR_SUCCESS;

    case STATUS_PENDING:
        return ERROR_IO_PENDING;

    case STATUS_INVALID_HANDLE:
    case STATUS_OBJECT_TYPE_MISMATCH:
        return WSAENOTSOCK;

    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_PAGEFILE_QUOTA:
    case STATUS_COMMITMENT_LIMIT:
    case STATUS_WORKING_SET_QUOTA:
    case STATUS_NO_MEMORY:
    case STATUS_QUOTA_EXCEEDED:
    case STATUS_TOO_MANY_PAGING_FILES:
    case STATUS_REMOTE_RESOURCES:
        return WSAENOBUFS;

    case STATUS_TOO_MANY_ADDRESSES:
    case STATUS_SHARING_VIOLATION:
    case STATUS_ADDRESS_ALREADY_EXISTS:
        return WSAEADDRINUSE;

    case STATUS_LINK_TIMEOUT:
    case STATUS_IO_TIMEOUT:
    case STATUS_TIMEOUT:
        return WSAETIMEDOUT;

    case STATUS_GRACEFUL_DISCONNECT:
        return WSAEDISCON;

    case STATUS_REMOTE_DISCONNECT:
    case STATUS_CONNECTION_RESET:
    case STATUS_LINK_FAILED:
    case STATUS_CONNECTION_DISCONNECTED:
    case STATUS_PORT_UNREACHABLE:
    case STATUS_HOPLIMIT_EXCEEDED:
        return WSAECONNRESET;

    case STATUS_LOCAL_DISCONNECT:
    case STATUS_TRANSACTION_ABORTED:
    case STATUS_CONNECTION_ABORTED:
        return WSAECONNABORTED;

    case STATUS_BAD_NETWORK_PATH:
    case STATUS_NETWORK_UNREACHABLE:
    case STATUS_PROTOCOL_UNREACHABLE:
        return WSAENETUNREACH;

    case STATUS_HOST_UNREACHABLE:
        return WSAEHOSTUNREACH;

    case STATUS_CANCELLED:
    case STATUS_REQUEST_ABORTED:
        return WSAEINTR;

    case STATUS_BUFFER_OVERFLOW:
    case STATUS_INVALID_BUFFER_SIZE:
        return WSAEMSGSIZE;

    case STATUS_BUFFER_TOO_SMALL:
    case STATUS_ACCESS_VIOLATION:
        return WSAEFAULT;

    case STATUS_DEVICE_NOT_READY:
    case STATUS_REQUEST_NOT_ACCEPTED:
        return WSAEWOULDBLOCK;

    case STATUS_INVALID_NETWORK_RESPONSE:
    case STATUS_NETWORK_BUSY:
    case STATUS_NO_SUCH_DEVICE:
    case STATUS_NO_SUCH_FILE:
    case STATUS_OBJECT_PATH_NOT_FOUND:
    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_UNEXPECTED_NETWORK_ERROR:
        return WSAENETDOWN;

    case STATUS_INVALID_CONNECTION:
        return WSAENOTCONN;

    case STATUS_REMOTE_NOT_LISTENING:
    case STATUS_CONNECTION_REFUSED:
        return WSAECONNREFUSED;

    case STATUS_PIPE_DISCONNECTED:
        return WSAESHUTDOWN;

    case STATUS_CONFLICTING_ADDRESSES:
    case STATUS_INVALID_ADDRESS:
    case STATUS_INVALID_ADDRESS_COMPONENT:
        return WSAEADDRNOTAVAIL;

    case STATUS_NOT_SUPPORTED:
    case STATUS_NOT_IMPLEMENTED:
        return WSAEOPNOTSUPP;

    case STATUS_ACCESS_DENIED:
        return WSAEACCES;

    default:
        if ((status & (FACILITY_NTWIN32 << 16)) == (FACILITY_NTWIN32 << 16) &&
            (status & (ERROR_SEVERITY_ERROR | ERROR_SEVERITY_WARNING)))
        {
            /*
             * It's a windows error that has been previously mapped to an ntstatus
             * code.
             */
            return (DWORD)(status & 0xffff);
        }
        else
        {
            /* The default fallback for unmappable ntstatus codes. */
            return WSAEINVAL;
        }
    }
}

#else               /* AMALGAMATE: ev.c (2/3) */
#line 1 "src/unix/async_unix.h"
#ifndef __EV_ASYNC_UNIX_INTERNAL_H__
#define __EV_ASYNC_UNIX_INTERNAL_H__

/* AMALGAMATE: #include "async.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a pair of eventfd.
 * Index 0 for read, index 1 for write.
 */
EV_LOCAL int ev__asyc_eventfd(int evtfd[2]);

EV_LOCAL void ev__async_eventfd_close(int fd);

/**
 * @brief Post event to eventfd.
 */
EV_LOCAL void ev__async_post(int wfd);

/**
 * @brief Pend event from eventfd.
 */
EV_LOCAL void ev__async_pend(int rfd);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/unix/io_unix.h"
#ifndef __EV_IO_UNIX_H__
#define __EV_IO_UNIX_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_unix.h" */
#include <sys/epoll.h>
#define EV_IO_IN            EPOLLIN     /**< The associated file is available for read(2) operations. */
#define EV_IO_OUT           EPOLLOUT    /**< The associated file is available for write(2) operations. */

#ifdef __cplusplus
extern "C" {
#endif

EV_LOCAL void ev__init_io(ev_loop_t* loop);

EV_LOCAL void ev__exit_io(ev_loop_t* loop);

/**
 * @brief Initialize io structure
 * @param[out] io   A pointer to the structure
 * @param[in] fd    File descriptor
 * @param[in] cb    IO active callback
 * @param[in] arg   User data
 */
EV_LOCAL void ev__nonblock_io_init(ev_nonblock_io_t* io, int fd, ev_nonblock_io_cb cb, void* arg);

/**
 * @brief Add events to IO structure
 * @param[in] loop  Event loop
 * @param[in] io    IO structure
 * @param[in] evts  #EV_IO_IN or #EV_IO_OUT
 */
EV_LOCAL void ev__nonblock_io_add(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts);

/**
 * @brief Delete events from IO structure
 * @param[in] loop  Event loop
 * @param[in] io    IO structure
 * @param[in] evts  #EV_IO_IN or #EV_IO_OUT
 */
EV_LOCAL void ev__nonblock_io_del(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts);

/**
 * @brief Add or remove FD_CLOEXEC
 * @param[in] fd    File descriptor
 * @param[in] set   bool
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__cloexec(int fd, int set);

/**
 * @brief Add or remove O_NONBLOCK
 * @param[in] fd    File descriptor
 * @param[in] set   bool
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__nonblock(int fd, int set);

/**
 * @brief Set reuse
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__reuse_unix(int fd);

/**
 * @brief Return the file access mode and the file status flags
 */
EV_LOCAL int ev__fcntl_getfl_unix(int fd);

/**
 * @brief Return the file descriptor flags.
 */
EV_LOCAL int ev__fcntl_getfd_unix(int fd);

/**
 * @brief readv wrap
 * @return 0: try again; >0: read size; <0 errno
 */
EV_LOCAL ssize_t ev__readv_unix(int fd, ev_buf_t* iov, int iovcnt);

/**
 * @brief readv wrap
 * @return 0: try again; >0: write size; <0 errno
 */
EV_LOCAL ssize_t ev__writev_unix(int fd, ev_buf_t* iov, int iovcnt);

/**
 * @brief write
 * @return 0: try again; >0: write size; <0 errno
 */
EV_LOCAL ssize_t ev__write_unix(int fd, void* buffer, size_t size);

/**
 * @brief Write \p req to \p fd
 * @param[in] fd    File to write
 * @param[in] req   Write request
 * @param[in] do_write  Write function
 * @param[in] arg       User defined data
 * @return              + #EV_SUCCESS: \p req send finish
 *                      + #EV_EAGAIN: \p req not send finish, need to try again
 *                      + other value: error
 */
EV_LOCAL int ev__send_unix(int fd, ev_write_t* req,
    ssize_t(*do_write)(int fd, struct iovec* iov, int iovcnt, void* arg), void* arg);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/unix/process_unix.h"
#ifndef __EV_PROCESS_UNIX_H__
#define __EV_PROCESS_UNIX_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_process_ctx_s
{
    ev_list_t       wait_queue;         /**< #ev_process_t::node */
    ev_mutex_t      wait_queue_mutex;   /**< Mutex for wait_queue */
} ev_process_ctx_t;

/**
 * @brief Initialize process context.
 */
EV_LOCAL void ev__init_process_unix(void);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/unix/tcp_unix.h"
#ifndef __EV_TCP_UNIX_H__
#define __EV_TCP_UNIX_H__

/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open fd for read/write.
 * @param[in] tcp   TCP handle
 * @param[in] fd    fd
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__tcp_open(ev_tcp_t* tcp, int fd);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/unix/loop_unix.h"
#ifndef __EV_LOOP_UNIX_H__
#define __EV_LOOP_UNIX_H__

/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "process_unix.h" */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_loop_unix_ctx
{
    clockid_t           hwtime_clock_id;    /**< Clock id */
    int                 iovmax;             /**< The limits instead of readv/writev */
    ev_process_ctx_t    process;            /**< Process context */
}ev_loop_unix_ctx_t;

/**
 * @brief Global runtime
 */
extern ev_loop_unix_ctx_t g_ev_loop_unix_ctx;

/**
 * @brief Initialize windows context.
 */
EV_LOCAL void ev__init_once_unix(void);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/unix/misc_unix.h"
#ifndef __EV_MISC_UNIX_INTERNAL_H__
#define __EV_MISC_UNIX_INTERNAL_H__

/* AMALGAMATE: #include "misc.h" */

#endif

#line 1 "src/unix/stream_unix.h"
#ifndef __EV_STREAM_UNIX_H__
#define __EV_STREAM_UNIX_H__

/* AMALGAMATE: #include "io_unix.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize stream.
 * @param[in] loop      Event loop
 * @param[out] stream   Stream handler
 * @param[in] fd        File descriptor
 * @param[in] wcb       Write callback
 * @param[in] rcb       Read callback
 */
EV_LOCAL void ev__nonblock_stream_init(ev_loop_t* loop, ev_nonblock_stream_t* stream,
    int fd, ev_stream_write_cb wcb, ev_stream_read_cb rcb);

/**
 * @brief Cleanup and exit stream
 * @param[in] stream    Stream handler
 */
EV_LOCAL void ev__nonblock_stream_exit(ev_nonblock_stream_t* stream);

/**
 * @brief Do stream write
 * @param[in] stream    Stream handle
 * @param[in] req       Write request
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__nonblock_stream_write(ev_nonblock_stream_t* stream, ev_write_t* req);

/**
 * @brief Do stream read
 * @param[in] stream    Stream handle
 * @param[in] req       Read request
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__nonblock_stream_read(ev_nonblock_stream_t* stream, ev_read_t* req);

/**
 * @brief Get pending action count.
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 * @return              Action count
 */
EV_LOCAL size_t ev__nonblock_stream_size(ev_nonblock_stream_t* stream, unsigned evts);

/**
 * @brief Abort pending requests
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 */
EV_LOCAL void ev__nonblock_stream_abort(ev_nonblock_stream_t* stream);

/**
 * @brief Cleanup pending requests
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 */
EV_LOCAL void ev__nonblock_stream_cleanup(ev_nonblock_stream_t* stream, unsigned evts);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/unix/work.h"
#ifndef __EV_WORK_INTERNAL_H__
#define __EV_WORK_INTERNAL_H__

/* AMALGAMATE: #include "defs.h" */

#ifdef __cplusplus
extern "C" {
#endif

EV_LOCAL void ev__init_work(ev_loop_t* loop);

EV_LOCAL void ev__exit_work(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/unix/async_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "io_unix.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "async_unix.h" */
#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

static void _async_on_wakeup_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(io, ev_async_t, backend.io);

    ev__async_pend(handle->backend.pipfd[0]);
    handle->active_cb(handle);
}

static void _ev_async_on_close(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);

    if (async->close_cb != NULL)
    {
        async->close_cb(async);
    }
}

static void _async_close_pipe(ev_async_t* handle)
{
    if (handle->backend.pipfd[0] != -1)
    {
        close(handle->backend.pipfd[0]);
        handle->backend.pipfd[0] = -1;
    }
    if (handle->backend.pipfd[1] != -1)
    {
        close(handle->backend.pipfd[1]);
        handle->backend.pipfd[1] = -1;
    }
}

static void _ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    assert(!ev__handle_is_closing(&handle->base));

    handle->close_cb = close_cb;
    _async_close_pipe(handle);

    ev__handle_event_dec(&handle->base);
    ev__handle_exit(&handle->base, close_cb != NULL ? _ev_async_on_close : NULL);
}

EV_LOCAL void ev__async_exit_force(ev_async_t* handle)
{
    _ev_async_exit(handle, NULL);
}

EV_LOCAL int ev__asyc_eventfd(int evtfd[2])
{
    int errcode;

#if defined(__linux__)
    if ((evtfd[0] = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) < 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    if ((evtfd[1] = dup(evtfd[0])) < 0)
    {
        errcode = errno;
        close(evtfd[0]);
        return ev__translate_sys_error(errcode);
    }
#else
    errcode = ev_pipe_make(evtfd, EV_PIPE_NONBLOCK, EV_PIPE_NONBLOCK);
    if (errcode != 0)
    {
        return errcode;
    }
#endif

    return 0;
}

EV_LOCAL void ev__async_eventfd_close(int fd)
{
    close(fd);
}

EV_LOCAL void ev__async_post(int wfd)
{
    uint64_t val = 1;

    ssize_t write_size;
    int errcode;

    do
    {
        write_size = write(wfd, &val, sizeof(val));
    }while(write_size == -1 && (errcode = errno) == EINTR);

    if (write_size < 0)
    {
        EV_ABORT();
    }
}

EV_LOCAL void ev__async_pend(int rfd)
{
    uint64_t val;
    int errcode;
    ssize_t read_size;

    do
    {
        read_size = read(rfd, &val, sizeof(val));
    }while(read_size == -1 && (errcode = errno) == EINTR);

    if (read_size < 0)
    {
        EV_ABORT();
    }
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    int errcode;

    handle->active_cb = cb;
    handle->close_cb = NULL;
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC);

    errcode = ev__asyc_eventfd(handle->backend.pipfd);
    if (errcode != 0)
    {
        goto err_close_handle;
    }

    ev__nonblock_io_init(&handle->backend.io, handle->backend.pipfd[0],
        _async_on_wakeup_unix, NULL);
    ev__nonblock_io_add(loop, &handle->backend.io, EV_IO_IN);
    ev__handle_event_add(&handle->base);

    return 0;

err_close_handle:
    _async_close_pipe(handle);
    ev__handle_exit(&handle->base, NULL);
    return errcode;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    _ev_async_exit(handle, close_cb);
}

void ev_async_wakeup(ev_async_t* handle)
{
    ev__async_post(handle->backend.pipfd[1]);
}

#line 1 "src/unix/fs_unix.c"
#define _GNU_SOURCE
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "fs.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "io_unix.h" */
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

static ev_dirent_type_t _ev_fs_get_dirent_type(struct dirent* dent)
{
    ev_dirent_type_t type;

    switch (dent->d_type)
    {
    case DT_DIR:
        type = EV_DIRENT_DIR;
        break;
    case DT_REG:
        type = EV_DIRENT_FILE;
        break;
    case DT_LNK:
        type = EV_DIRENT_LINK;
        break;
    case DT_FIFO:
        type = EV_DIRENT_FIFO;
        break;
    case DT_SOCK:
        type = EV_DIRENT_SOCKET;
        break;
    case DT_CHR:
        type = EV_DIRENT_CHR;
        break;
    case DT_BLK:
        type = EV_DIRENT_BLOCK;
        break;
    default:
        type = EV_DIRENT_UNKNOWN;
        break;
    }

    return type;
}

static int _ev_fs_mkpath(char* file_path, int mode)
{
    char* p;
    int errcode;
    assert(file_path && *file_path);

    for (p = strchr(file_path + 1, '/'); p != NULL; p = strchr(p + 1, '/'))
    {
        *p = '\0';
        if (mkdir(file_path, mode) == -1)
        {
            errcode = errno;
            if (errcode != EEXIST)
            {
                *p = '/';
                return ev__translate_sys_error(errcode);
            }
        }
        *p = '/';
    }

    if (mkdir(file_path, mode) == -1)
    {
        errcode = errno;
        if (errcode != EEXIST)
        {
            return ev__translate_sys_error(errcode);
        }
    }

    return 0;
}

EV_LOCAL int ev__fs_fstat(ev_os_file_t file, ev_fs_stat_t* statbuf)
{
    int ret;
    int errcode;

#if defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 28)
    struct statx statxbuf;
    ret = statx(file, "", AT_EMPTY_PATH, STATX_ALL, &statxbuf);
    if (ret != 0)
    {
        goto err_errno;
    }

    statbuf->st_dev                 = makedev(statxbuf.stx_dev_major, statxbuf.stx_dev_minor);
    statbuf->st_mode                = statxbuf.stx_mode;
    statbuf->st_nlink               = statxbuf.stx_nlink;
    statbuf->st_uid                 = statxbuf.stx_uid;
    statbuf->st_gid                 = statxbuf.stx_gid;
    statbuf->st_rdev                = makedev(statxbuf.stx_rdev_major, statxbuf.stx_rdev_minor);
    statbuf->st_ino                 = statxbuf.stx_ino;
    statbuf->st_size                = statxbuf.stx_size;
    statbuf->st_blksize             = statxbuf.stx_blksize;
    statbuf->st_blocks              = statxbuf.stx_blocks;
    statbuf->st_atim.tv_sec         = statxbuf.stx_atime.tv_sec;
    statbuf->st_atim.tv_nsec        = statxbuf.stx_atime.tv_nsec;
    statbuf->st_mtim.tv_sec         = statxbuf.stx_mtime.tv_sec;
    statbuf->st_mtim.tv_nsec        = statxbuf.stx_mtime.tv_nsec;
    statbuf->st_ctim.tv_sec         = statxbuf.stx_ctime.tv_sec;
    statbuf->st_ctim.tv_nsec        = statxbuf.stx_ctime.tv_nsec;
    statbuf->st_birthtim.tv_sec     = statxbuf.stx_btime.tv_sec;
    statbuf->st_birthtim.tv_nsec    = statxbuf.stx_btime.tv_nsec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#else
    struct stat pbuf;
    ret = fstat(file, &pbuf);
    if (ret != 0)
    {
        goto err_errno;
    }
    statbuf->st_dev                 = pbuf.st_dev;
    statbuf->st_mode                = pbuf.st_mode;
    statbuf->st_nlink               = pbuf.st_nlink;
    statbuf->st_uid                 = pbuf.st_uid;
    statbuf->st_gid                 = pbuf.st_gid;
    statbuf->st_rdev                = pbuf.st_rdev;
    statbuf->st_ino                 = pbuf.st_ino;
    statbuf->st_size                = pbuf.st_size;
    statbuf->st_blksize             = pbuf.st_blksize;
    statbuf->st_blocks              = pbuf.st_blocks;

#   if defined(__APPLE__)
    statbuf->st_atim.tv_sec         = pbuf.st_atimespec.tv_sec;
    statbuf->st_atim.tv_nsec        = pbuf.st_atimespec.tv_nsec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtimespec.tv_sec;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtimespec.tv_nsec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctimespec.tv_sec;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctimespec.tv_nsec;
    statbuf->st_birthtim.tv_sec     = pbuf.st_birthtimespec.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_birthtimespec.tv_nsec;
    statbuf->st_flags               = pbuf.st_flags;
    statbuf->st_gen                 = pbuf.st_gen;
#   elif defined(__ANDROID__)
    statbuf->st_atim.tv_sec         = pbuf.st_atime;
    statbuf->st_atim.tv_nsec        = pbuf.st_atimensec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtime;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtimensec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctime;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctimensec;
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctime;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_ctimensec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#   elif !defined(_AIX) && !defined(__MVS__) && \
        (\
            defined(__DragonFly__)   || \
            defined(__FreeBSD__)     || \
            defined(__OpenBSD__)     || \
            defined(__NetBSD__)      || \
            defined(_GNU_SOURCE)     || \
            defined(_BSD_SOURCE)     || \
            defined(_SVID_SOURCE)    || \
            defined(_XOPEN_SOURCE)   || \
            defined(_DEFAULT_SOURCE)\
        )
    statbuf->st_atim.tv_sec         = pbuf.st_atim.tv_sec;
    statbuf->st_atim.tv_nsec        = pbuf.st_atim.tv_nsec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtim.tv_sec;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtim.tv_nsec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctim.tv_sec;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctim.tv_nsec;
#       if defined(__FreeBSD__) || defined(__NetBSD__)
    statbuf->st_birthtim.tv_sec     = pbuf.st_birthtim.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_birthtim.tv_nsec;
    statbuf->st_flags               = pbuf.st_flags;
    statbuf->st_gen                 = pbuf.st_gen;
#       else
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctim.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_ctim.tv_nsec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#       endif
#   else
    statbuf->st_atim.tv_sec         = pbuf.st_atime;
    statbuf->st_atim.tv_nsec        = 0;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtime;
    statbuf->st_mtim.tv_nsec        = 0;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctime;
    statbuf->st_ctim.tv_nsec        = 0;
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctime;
    statbuf->st_birthtim.tv_nsec    = 0;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#   endif
#endif

    return 0;

err_errno:
    errcode = errno;
    return ev__translate_sys_error(errcode);
}

EV_LOCAL int ev__fs_close(ev_os_file_t file)
{
    int errcode;
    if (close(file) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

EV_LOCAL int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode)
{
    int errcode;

#if defined(O_CLOEXEC)
    flags |= O_CLOEXEC;
#endif

    int fd = open(path, flags, mode);
    if (fd < 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

#if defined(O_CLOEXEC)
    if ((errcode = ev__cloexec(fd, 1)) != 0)
    {
        close(fd);
        return errcode;
    }
#endif

    *file = fd;
    return 0;
}

EV_LOCAL int ev__fs_seek(ev_os_file_t file, int whence, ssize_t offset)
{
    int errcode;
    if (lseek(file, offset, whence) == (off_t)-1)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

EV_LOCAL ssize_t ev__fs_readv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    ssize_t read_size = readv(file, (struct iovec*)bufs, nbuf);
    if (read_size >= 0)
    {
        return read_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

EV_LOCAL ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t read_size = preadv(file, (struct iovec*)bufs, nbuf, offset);
    if (read_size >= 0)
    {
        return read_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

EV_LOCAL ssize_t ev__fs_writev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    ssize_t write_size = writev(file, (struct iovec*)bufs, nbuf);
    if (write_size >= 0)
    {
        return write_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

EV_LOCAL ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t write_size = pwritev(file, (struct iovec*)bufs, nbuf, offset);
    if (write_size >= 0)
    {
        return write_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

EV_LOCAL int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg)
{
    int ret = 0;
    DIR* dir = opendir(path);

    if (dir == NULL)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    struct dirent* res;
    ev_dirent_t info;

    while ((res = readdir(dir)) != NULL)
    {
        if (strcmp(res->d_name, ".") == 0 || strcmp(res->d_name, "..") == 0)
        {
            continue;
        }

        info.name = res->d_name;
        info.type = _ev_fs_get_dirent_type(res);

        if (cb(&info, arg) != 0)
        {
            break;
        }
    }

    closedir(dir);

    return ret;
}

EV_LOCAL int ev__fs_mkdir(const char* path, int mode)
{
    char* dup_path = ev__strdup(path);
    if (dup_path == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = _ev_fs_mkpath(dup_path, mode);
    ev_free(dup_path);

    return ret;
}

#line 1 "src/unix/io_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "io_unix.h" */
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

static int _ev_io_finalize_send_req_unix(ev_write_t* req, size_t write_size)
{
    req->size += write_size;

    /* All data is sent */
    if (req->size == req->capacity)
    {
        req->nbuf = 0;
        return 0;
    }
    assert(req->size < req->capacity);

    /* maintenance iovec */
    size_t idx;
    for (idx = 0; write_size > 0 && idx < req->nbuf; idx++)
    {
        if (write_size < req->bufs[idx].size)
        {
            req->bufs[idx].size -= write_size;
            req->bufs[idx].data = (uint8_t*)req->bufs[idx].data + write_size;
            break;
        }
        else
        {
            write_size -= req->bufs[idx].size;
        }
    }

    assert(idx < req->nbuf);

    memmove(&req->bufs[0], &req->bufs[idx], sizeof(req->bufs[0]) * (req->nbuf - idx));
    req->nbuf -= idx;

    return EV_EAGAIN;
}

static int _ev_cmp_io_unix(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_nonblock_io_t* io1 = EV_CONTAINER_OF(key1, ev_nonblock_io_t, node);
    ev_nonblock_io_t* io2 = EV_CONTAINER_OF(key2, ev_nonblock_io_t, node);
    return io1->data.fd - io2->data.fd;
}

EV_LOCAL void ev__init_io(ev_loop_t* loop)
{
    int err;
    ev_map_init(&loop->backend.io, _ev_cmp_io_unix, NULL);

    if ((loop->backend.pollfd = epoll_create(256)) == -1)
    {
        err = errno;
        EV_ABORT("errno:%d", err);
    }
    if ((err = ev__cloexec(loop->backend.pollfd, 1)) != 0)
    {
        err = errno;
        EV_ABORT("errno:%d", err);
    }
}

EV_LOCAL void ev__exit_io(ev_loop_t* loop)
{
    if (loop->backend.pollfd != -1)
    {
        close(loop->backend.pollfd);
        loop->backend.pollfd = -1;
    }
}

EV_LOCAL void ev__nonblock_io_init(ev_nonblock_io_t* io, int fd, ev_nonblock_io_cb cb, void* arg)
{
    io->data.fd = fd;
    io->data.c_events = 0;
    io->data.n_events = 0;
    io->data.cb = cb;
    io->data.arg = arg;
}

EV_LOCAL void ev__nonblock_io_add(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts)
{
    int errcode;
    struct epoll_event poll_event;

    io->data.n_events |= evts;
    if (io->data.n_events == io->data.c_events)
    {
        return;
    }

    memset(&poll_event, 0, sizeof(poll_event));
    poll_event.events = io->data.n_events;
    poll_event.data.fd = io->data.fd;

    int op = io->data.c_events == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
    {
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_ADD)
    {
        ev_map_insert(&loop->backend.io, &io->node);
    }
}

EV_LOCAL void ev__nonblock_io_del(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts)
{
    int errcode;
    struct epoll_event poll_event;
    io->data.n_events &= ~evts;
    if (io->data.n_events == io->data.c_events)
    {
        return;
    }

    memset(&poll_event, 0, sizeof(poll_event));
    poll_event.events = io->data.n_events;
    poll_event.data.fd = io->data.fd;

    int op = io->data.n_events == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
    {
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_DEL)
    {
        ev_map_erase(&loop->backend.io, &io->node);
    }
}

EV_LOCAL int ev__cloexec(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
    int r;

    do
    {
        r = ioctl(fd, set ? FIOCLEX : FIONCLEX);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return 0;
#else
    int flags;

    int r = ev__fcntl_getfd_unix(fd);
    if (r == -1)
    {
        return errno;
    }

    /* Bail out now if already set/clear. */
    if (!!(r & FD_CLOEXEC) == !!set)
    {
        return 0;
    }

    if (set)
    {
        flags = r | FD_CLOEXEC;
    }
    else
    {
        flags = r & ~FD_CLOEXEC;
    }

    do
    {
        r = fcntl(fd, F_SETFD, flags);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return 0;
#endif
}

EV_LOCAL int ev__nonblock(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
    int r;

    do
    {
        r = ioctl(fd, FIONBIO, &set);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return 0;
#else
    int flags;

    int r = ev__fcntl_getfl_unix(fd);
    if (r == -1)
    {
        return ev__translate_sys_error(errno);
    }

    /* Bail out now if already set/clear. */
    if (!!(r & O_NONBLOCK) == !!set)
    {
        return 0;
    }

    if (set)
    {
        flags = r | O_NONBLOCK;
    }
    else
    {
        flags = r & ~O_NONBLOCK;
    }

    do
    {
        r = fcntl(fd, F_SETFL, flags);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return 0;
#endif
}

EV_LOCAL int ev__reuse_unix(int fd)
{
    int yes;
    yes = 1;

#if defined(SO_REUSEPORT) && defined(__MVS__)
    struct sockaddr_in sockfd;
    unsigned int sockfd_len = sizeof(sockfd);
    if (getsockname(fd, (struct sockaddr*)&sockfd, &sockfd_len) == -1)
    {
        goto err;
    }
    if (sockfd.sin_family == AF_UNIX)
    {
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
        {
            goto err;
        }
    }
    else
    {
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)))
        {
            goto err;
        }
    }
#elif defined(SO_REUSEPORT) && !defined(__linux__)
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)))
    {
        goto err;
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
    {
        goto err;
    }
#endif

    return 0;

err:
    yes = errno;
    return ev__translate_sys_error(yes);
}

EV_LOCAL int ev__fcntl_getfl_unix(int fd)
{
    int mode;
    do
    {
        mode = fcntl(fd, F_GETFL);
    } while (mode == -1 && errno == EINTR);
    return mode;
}

EV_LOCAL int ev__fcntl_getfd_unix(int fd)
{
    int flags;

    do
    {
        flags = fcntl(fd, F_GETFD);
    } while (flags == -1 && errno == EINTR);

    return flags;
}

EV_LOCAL ssize_t ev__readv_unix(int fd, ev_buf_t* iov, int iovcnt)
{
    ssize_t read_size;
    do
    {
        read_size = readv(fd, (struct iovec*)iov, iovcnt);
    } while (read_size == -1 && errno == EINTR);

    if (read_size > 0)
    {
        return read_size;
    }
    else if (read_size == 0)
    {
        return EV_EOF;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

EV_LOCAL ssize_t ev__writev_unix(int fd, ev_buf_t* iov, int iovcnt)
{
    ssize_t write_size;
    do
    {
        write_size = writev(fd, (struct iovec*)iov, iovcnt);
    } while (write_size == -1 && errno == EINTR);

    if (write_size >= 0)
    {
        return write_size;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

EV_LOCAL ssize_t ev__write_unix(int fd, void* buffer, size_t size)
{
    ssize_t send_size;
    do
    {
        send_size = write(fd, buffer, size);
    } while (send_size == -1 && errno == EINTR);

    if (send_size >= 0)
    {
        return send_size;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

EV_LOCAL int ev__send_unix(int fd, ev_write_t* req,
    ssize_t(*do_write)(int fd, struct iovec* iov, int iovcnt, void* arg), void* arg)
{
    ev_buf_t* iov = req->bufs;
    int iovcnt = req->nbuf;
    if (iovcnt > g_ev_loop_unix_ctx.iovmax)
    {
        iovcnt = g_ev_loop_unix_ctx.iovmax;
    }

    ssize_t write_size = do_write(fd, (struct iovec*)iov, iovcnt, arg);

    /* Check send result */
    if (write_size < 0)
    {
        if (write_size == EV_ENOBUFS)
        {
            write_size = EV_EAGAIN;
        }
        return write_size;
    }

    return _ev_io_finalize_send_req_unix(req, (size_t)write_size);
}

#line 1 "src/unix/loop_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_unix.h" */
/* AMALGAMATE: #include "io_unix.h" */
/* AMALGAMATE: #include "work.h" */
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/eventfd.h>

#if defined(__PASE__)
/* on IBMi PASE the control message length can not exceed 256. */
#   define EV__CMSG_FD_COUNT 60
#else
#   define EV__CMSG_FD_COUNT 64
#endif
#define EV__CMSG_FD_SIZE (EV__CMSG_FD_COUNT * sizeof(int))

ev_loop_unix_ctx_t g_ev_loop_unix_ctx;

static void _ev_init_hwtime(void)
{
    struct timespec t;
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &t) != 0)
    {
        goto err;
    }
    if (t.tv_nsec > 1 * 1000 * 1000)
    {
        goto err;
    }
    g_ev_loop_unix_ctx.hwtime_clock_id = CLOCK_MONOTONIC_COARSE;
    return;

err:
    g_ev_loop_unix_ctx.hwtime_clock_id = CLOCK_MONOTONIC;
}

static ev_nonblock_io_t* _ev_find_io(ev_loop_t* loop, int fd)
{
    ev_nonblock_io_t tmp;
    tmp.data.fd = fd;

    ev_map_node_t* it = ev_map_find(&loop->backend.io, &tmp.node);
    return it != NULL ? EV_CONTAINER_OF(it, ev_nonblock_io_t, node) : NULL;
}

static int _ev_poll_once(ev_loop_t* loop, struct epoll_event* events, int maxevents, int timeout)
{
    int nfds = epoll_wait(loop->backend.pollfd, events, maxevents, timeout);
    if (nfds < 0)
    {
        return nfds;
    }

    int i;
    for (i = 0; i < nfds; i++)
    {
        ev_nonblock_io_t* io = _ev_find_io(loop, events[i].data.fd);
        io->data.cb(io, events[i].events, io->data.arg);
    }

    return nfds;
}

static void _ev_init_iovmax(void)
{
#if defined(IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = IOV_MAX;
#elif defined(__IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = __IOV_MAX;
#elif defined(_SC_IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = sysconf(_SC_IOV_MAX);
    if (g_ev_loop_unix_ctx.iovmax == -1)
    {
        g_ev_loop_unix_ctx.iovmax = 1;
    }
#else
    g_ev_loop_unix_ctx.iovmax = EV_IOV_MAX;
#endif
}

static void _ev_check_layout_unix(void)
{
    ENSURE_LAYOUT(ev_buf_t, data, size, struct iovec, iov_base, iov_len);
}

static void _ev_init_once_unix(void)
{
    _ev_check_layout_unix();
    _ev_init_hwtime();
    _ev_init_iovmax();
    ev__init_process_unix();
}

uint64_t ev_hrtime(void)
{
    int errcode;
    struct timespec t;

    if (clock_gettime(g_ev_loop_unix_ctx.hwtime_clock_id, &t) != 0)
    {
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    return t.tv_sec * 1000 * 1000 + t.tv_nsec / 1000;
}

EV_LOCAL void ev__init_once_unix(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_unix);
}

EV_LOCAL int ev__loop_init_backend(ev_loop_t* loop)
{
    ev__init_once_unix();
    ev__init_io(loop);
    ev__init_work(loop);

    return 0;
}

EV_LOCAL void ev__loop_exit_backend(ev_loop_t* loop)
{
    ev__exit_work(loop);
    ev__exit_io(loop);
}

EV_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
    int nevts;
    int errcode;
    struct epoll_event events[128];

    /**
     * A bug in kernels < 2.6.37 makes timeouts larger than ~30 minutes
     * effectively infinite on 32 bits architectures.  To avoid blocking
     * indefinitely, we cap the timeout and poll again if necessary.
     *
     * Note that "30 minutes" is a simplification because it depends on
     * the value of CONFIG_HZ.  The magic constant assumes CONFIG_HZ=1200,
     * that being the largest value I have seen in the wild (and only once.)
     */
    const uint32_t max_safe_timeout = 1789569;

    /**
     * from libuv, this value gives the best throughput.
     */
    int max_performance_events = 49152;

    const uint64_t base_time = loop->hwtime;
    const uint32_t user_timeout = timeout;
    for (; max_performance_events != 0; max_performance_events--)
    {
        if (timeout > max_safe_timeout)
        {
            timeout = max_safe_timeout;
        }

        nevts = _ev_poll_once(loop, events, ARRAY_SIZE(events), timeout);

        if (nevts == ARRAY_SIZE(events))
        {/* Poll for more events but don't block this time. */
            timeout = 0;
            continue;
        }

        if (nevts >= 0)
        {
            break;
        }

        /* If errno is not EINTR, something must wrong in the program */
        if ((errcode = errno) != EINTR)
        {
            EV_ABORT("errno:%d", errcode);
        }

        ev__loop_update_time(loop);
        uint64_t pass_time = loop->hwtime - base_time;
        if (pass_time >= user_timeout)
        {
            break;
        }

        timeout = user_timeout - pass_time;
    }
}

#line 1 "src/unix/misc_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc_unix.h" */
#include <errno.h>

EV_LOCAL int ev__translate_sys_error(int syserr)
{
    return ev__translate_posix_sys_error(syserr);
}

void ev_library_shutdown(void)
{
    // Do nothing
}

#line 1 "src/unix/mutex_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */

static void _ev_mutex_init_unix(ev_os_mutex_t* handle)
{
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
    if (pthread_mutex_init(handle, NULL) != 0)
    {
        EV_ABORT();
    }
#else
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        EV_ABORT();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        EV_ABORT();
    }
#endif
}

static void _ev_mutex_init_recursive_unix(ev_os_mutex_t* handle)
{
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    {
        EV_ABORT();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        EV_ABORT();
    }
}

void ev_mutex_init(ev_mutex_t* handle, int recursive)
{
    if (recursive)
    {
        _ev_mutex_init_recursive_unix(&handle->u.r);
    }
    else
    {
        _ev_mutex_init_unix(&handle->u.r);
    }
}

void ev_mutex_exit(ev_mutex_t* handle)
{
    if (pthread_mutex_destroy(&handle->u.r))
    {
        EV_ABORT();
    }
}

void ev_mutex_enter(ev_mutex_t* handle)
{
    if (pthread_mutex_lock(&handle->u.r))
    {
        EV_ABORT();
    }
}

void ev_mutex_leave(ev_mutex_t* handle)
{
    if (pthread_mutex_unlock(&handle->u.r))
    {
        EV_ABORT();
    }
}

int ev_mutex_try_enter(ev_mutex_t* handle)
{
    int err = pthread_mutex_trylock(&handle->u.r);
    if (!err)
    {
        return 0;
    }

    if (err != EBUSY && err != EAGAIN)
    {
        EV_ABORT();
    }

    return EV_EBUSY;
}

#line 1 "src/unix/once_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */
#include <stdlib.h>

void ev_once_execute(ev_once_t* guard, ev_once_cb cb)
{
    if (pthread_once(&guard->guard, cb) != 0)
    {
        EV_ABORT();
    }
}

#line 1 "src/unix/pipe_unix.c"
#define _GNU_SOURCE
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_unix.h" */
/* AMALGAMATE: #include "tcp_unix.h" */
/* AMALGAMATE: #include "stream_unix.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "pipe.h" */
/* AMALGAMATE: #include "handle.h" */
/* AMALGAMATE: #include "log.h" */
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef char ev_ipc_msghdr[CMSG_SPACE(sizeof(int))];

static void _ev_pipe_close_unix(ev_pipe_t* pipe)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        close(pipe->pipfd);
        pipe->pipfd = EV_OS_PIPE_INVALID;
    }
}

static void _ev_pipe_on_close_unix(ev_handle_t* handle)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(handle, ev_pipe_t, base);

    if (pipe_handle->close_cb != NULL)
    {
        pipe_handle->close_cb(pipe_handle);
    }
}

static void _ev_pipe_w_user_callback_unix(ev_pipe_t* pipe,
    ev_pipe_write_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&pipe->base);
    ev__write_exit(&req->base);
    req->ucb(req, size);
}

static void _ev_pipe_r_user_callback_unix(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&pipe->base);
    ev__read_exit(&req->base);
    req->ucb(req, size);
}

static void _ev_pipe_on_data_mode_write_unix(ev_nonblock_stream_t* stream,
    ev_write_t* req, ssize_t size)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(stream, ev_pipe_t, backend.data_mode.stream);
    ev_pipe_write_req_t* w_req = EV_CONTAINER_OF(req, ev_pipe_write_req_t, base);
    _ev_pipe_w_user_callback_unix(pipe_handle, w_req, size);
}

static void _ev_pipe_on_data_mode_read_unix(ev_nonblock_stream_t* stream,
    ev_read_t* req, ssize_t size)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(stream, ev_pipe_t, backend.data_mode.stream);

    ev_pipe_read_req_t* r_req = EV_CONTAINER_OF(req, ev_pipe_read_req_t, base);
    _ev_pipe_r_user_callback_unix(pipe_handle, r_req, size);
}

static int _ev_pipe_on_ipc_mode_io_read_remain(ev_pipe_t* pipe)
{
    assert(pipe->backend.ipc_mode.rio.curr.reading != NULL);

    ev_buf_t bufs[EV_IOV_MAX];
    size_t target_size = pipe->backend.ipc_mode.rio.curr.data_remain_size;
    ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.curr.reading;

    size_t buf_idx = pipe->backend.ipc_mode.rio.curr.buf_idx;
    size_t buf_pos = pipe->backend.ipc_mode.rio.curr.buf_pos;

    size_t idx;
    for (idx = 0;
        idx < ARRAY_SIZE(bufs) && target_size > 0 && buf_idx < req->base.data.nbuf;
        idx++, buf_idx++)
    {
        bufs[idx].data = (uint8_t*)req->base.data.bufs[buf_idx].data + buf_pos;
        bufs[idx].size = req->base.data.bufs[buf_idx].size - buf_pos;

        bufs[idx].size = EV_MIN(bufs[idx].size, target_size);
        target_size -= bufs[idx].size;

        buf_pos = 0;
    }

    ssize_t read_size = ev__readv_unix(pipe->pipfd, bufs, idx);
    if (read_size < 0)
    {
        return read_size;
    }

    pipe->backend.ipc_mode.rio.curr.data_remain_size -= read_size;
    req->base.data.size += read_size;

    /* no data remain */
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size == 0)
    {
        goto callback;
    }

    /* move cursor */
    while (read_size > 0 && pipe->backend.ipc_mode.rio.curr.buf_idx < req->base.data.nbuf)
    {
        size_t left_size = req->base.data.bufs[pipe->backend.ipc_mode.rio.curr.buf_idx].size
            - pipe->backend.ipc_mode.rio.curr.buf_pos;

        if (left_size > (size_t)read_size)
        {
            pipe->backend.ipc_mode.rio.curr.buf_pos += read_size;
            break;
        }

        read_size -= left_size;
        pipe->backend.ipc_mode.rio.curr.buf_idx++;
        pipe->backend.ipc_mode.rio.curr.buf_pos = 0;
        continue;
    }

    /* Buffer is full */
    if (pipe->backend.ipc_mode.rio.curr.buf_idx >= req->base.data.nbuf)
    {
        goto callback;
    }

    return 0;

callback:
    pipe->backend.ipc_mode.rio.curr.reading = NULL;
    _ev_pipe_r_user_callback_unix(pipe, req, req->base.data.size);
    return 0;
}

static ssize_t _ev_pipe_recvmsg_unix(ev_pipe_t* pipe, struct msghdr* msg)
{
    struct cmsghdr* cmsg;
    ssize_t rc;
    int* pfd;
    int* end;
    int fd = pipe->pipfd;
#if defined(__linux__)
    if (!pipe->backend.ipc_mode.mask.no_cmsg_cloexec)
    {
        rc = recvmsg(fd, msg, MSG_CMSG_CLOEXEC);
        if (rc != -1)
        {
            return rc;
        }
        if ((rc = errno) != EINVAL)
        {
            return ev__translate_sys_error(errno);
        }
        rc = recvmsg(fd, msg, 0);
        if (rc == -1)
        {
            return ev__translate_sys_error(errno);
        }
        pipe->backend.ipc_mode.mask.no_cmsg_cloexec = 1;
    }
    else
    {
        rc = recvmsg(fd, msg, 0);
    }
#else
    rc = recvmsg(fd, msg, 0);
#endif
    if (rc == -1)
    {
        return ev__translate_sys_error(errno);
    }
    if (msg->msg_controllen == 0)
    {
        return rc;
    }
    for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg))
    {
        if (cmsg->cmsg_type == SCM_RIGHTS)
        {
            for (pfd = (int*)CMSG_DATA(cmsg),
                end = (int*)((char*)cmsg + cmsg->cmsg_len);
                pfd < end;
                pfd += 1)
            {
                ev__cloexec(*pfd, 1);
            }
        }
    }
    return rc;
}

static void _ev_stream_do_read_parser_msghdr(ev_pipe_read_req_t* req, struct msghdr* msg)
{
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(msg);
    if (cmsg == NULL)
    {
        return;
    }

    void* pv = CMSG_DATA(cmsg);
    int* pi = pv;
    req->handle.os_socket = *pi;

    assert(CMSG_NXTHDR(msg, cmsg) == NULL);
}

static int _ev_pipe_on_ipc_mode_io_read_first(ev_pipe_t* pipe)
{
    void* buffer = (uint8_t*)pipe->backend.ipc_mode.rio.buffer + pipe->backend.ipc_mode.rio.curr.head_read_size;
    size_t buffer_size = sizeof(pipe->backend.ipc_mode.rio.buffer) - pipe->backend.ipc_mode.rio.curr.head_read_size;

    if (pipe->backend.ipc_mode.rio.curr.head_read_size == 0)
    {
        struct msghdr msg;
        ev_ipc_msghdr cmsg_space;
        struct iovec iov = { buffer, buffer_size };

        /* ipc uses recvmsg */
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        /* Set up to receive a descriptor even if one isn't in the message */
        msg.msg_controllen = sizeof(cmsg_space);
        msg.msg_control = cmsg_space;

        ssize_t read_size = _ev_pipe_recvmsg_unix(pipe, &msg);
        if (read_size <= 0)
        {/* Error or EOF */
            return read_size;
        }
        assert(read_size <= (ssize_t)sizeof(pipe->backend.ipc_mode.rio.buffer));

        _ev_stream_do_read_parser_msghdr(pipe->backend.ipc_mode.rio.curr.reading, &msg);
        pipe->backend.ipc_mode.rio.curr.head_read_size += read_size;
    }
    else
    {
        ssize_t read_size = read(pipe->pipfd, buffer, buffer_size);
        if (read_size == 0)
        {
            return EV_EOF;
        }
        if (read_size < 0)
        {
            int err = errno;
            if (err == EAGAIN)
            {/* try again */
                return 0;
            }
            return ev__translate_sys_error(err);
        }
        pipe->backend.ipc_mode.rio.curr.head_read_size += read_size;
    }

    /* If frame header not read complete, try again */
    if (pipe->backend.ipc_mode.rio.curr.head_read_size < sizeof(ev_ipc_frame_hdr_t))
    {
        return 0;
    }

    /* A invalid frame header means something wrong in the transmission link */
    if (!ev__ipc_check_frame_hdr(pipe->backend.ipc_mode.rio.buffer, sizeof(ev_ipc_frame_hdr_t)))
    {
        return EV_EPIPE;
    }

    ev_ipc_frame_hdr_t* hdr = (ev_ipc_frame_hdr_t*)pipe->backend.ipc_mode.rio.buffer;
    pipe->backend.ipc_mode.rio.curr.data_remain_size = hdr->hdr_dtsz;

    /* No data remain to read means peer send a empty package */
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size == 0)
    {
        ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.curr.reading;
        pipe->backend.ipc_mode.rio.curr.reading = NULL;
        _ev_pipe_r_user_callback_unix(pipe, req, req->base.data.size);
    }

    /* Process to read body */
    return _ev_pipe_on_ipc_mode_io_read_remain(pipe);
}

static int _ev_pipe_on_ipc_mode_io_read_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size != 0)
    {
        return _ev_pipe_on_ipc_mode_io_read_remain(pipe);
    }

    assert(pipe->backend.ipc_mode.rio.curr.reading == NULL);

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.rqueue);
    assert(it != NULL);

    pipe->backend.ipc_mode.rio.curr.reading = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    pipe->backend.ipc_mode.rio.curr.head_read_size = 0;
    pipe->backend.ipc_mode.rio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.rio.curr.buf_pos = 0;

    return _ev_pipe_on_ipc_mode_io_read_first(pipe);
}

static ssize_t _ev_pipe_sendmsg_unix(int fd, int fd_to_send, struct iovec* iov, int iovcnt)
{
    struct msghdr msg;
    struct cmsghdr* cmsg;

    ev_ipc_msghdr msg_ctrl_hdr;
    memset(&msg_ctrl_hdr, 0, sizeof(msg_ctrl_hdr));

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = iovcnt;
    msg.msg_flags = 0;

    msg.msg_control = msg_ctrl_hdr;
    msg.msg_controllen = sizeof(msg_ctrl_hdr);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));

    /* silence aliasing warning */
    {
        void* pv = CMSG_DATA(cmsg);
        int* pi = pv;
        *pi = fd_to_send;
    }

    ssize_t n;
    do
    {
        n = sendmsg(fd, &msg, 0);
    } while (n == -1 && errno == EINTR);

    if (n >= 0)
    {
        return n;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }
    return ev__translate_sys_error(err);
}

static int _ev_pipe_on_ipc_mode_io_write_remain_body_unix(ev_pipe_t* pipe)
{
    ev_buf_t bufs[EV_IOV_MAX];
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.curr.writing;
    assert(req != NULL);

    size_t buf_idx = pipe->backend.ipc_mode.wio.curr.buf_idx;
    size_t buf_pos = pipe->backend.ipc_mode.wio.curr.buf_pos;

    size_t idx;
    for (idx = 0; idx < ARRAY_SIZE(bufs) && buf_idx < req->base.nbuf; idx++, buf_idx++)
    {
        bufs[idx].data = (uint8_t*)req->base.bufs[buf_idx].data + buf_pos;
        bufs[idx].size = req->base.bufs[buf_idx].size - buf_pos;
        buf_pos = 0;
    }

    ssize_t write_size = ev__writev_unix(pipe->pipfd, bufs, idx);
    if (write_size < 0)
    {
        return write_size;
    }

    req->base.size += write_size;
    while (write_size > 0)
    {
        size_t left_size = req->base.bufs[pipe->backend.ipc_mode.wio.curr.buf_idx].size -
            pipe->backend.ipc_mode.wio.curr.buf_pos;
        if (left_size > (size_t)write_size)
        {
            pipe->backend.ipc_mode.wio.curr.buf_pos += write_size;
            write_size = 0;
            break;
        }

        write_size -= left_size;
        pipe->backend.ipc_mode.wio.curr.buf_idx++;
        pipe->backend.ipc_mode.wio.curr.buf_pos = 0;
    }

    /* send finish */
    if (pipe->backend.ipc_mode.wio.curr.buf_idx >= req->base.nbuf)
    {
        pipe->backend.ipc_mode.wio.curr.writing = NULL;
        _ev_pipe_w_user_callback_unix(pipe, req, req->base.size);
    }

    return 0;
}

static int _ev_pipe_on_ipc_mode_io_write_remain_head_unix(ev_pipe_t* pipe)
{
    EV_LOG_TRACE("pipe(%p) send remain header", pipe);

    const size_t buffer_pos = pipe->backend.ipc_mode.wio.curr.head_send_size;
    void* buffer = pipe->backend.ipc_mode.wio.buffer + buffer_pos;
    size_t buffer_size = pipe->backend.ipc_mode.wio.curr.head_send_capacity - buffer_pos;

    ssize_t send_size = ev__write_unix(pipe->pipfd, buffer, buffer_size);
    if (send_size < 0)
    {
        return send_size;
    }
    pipe->backend.ipc_mode.wio.curr.head_send_size += send_size;

    /* try again */
    if ((size_t)send_size < buffer_size)
    {
        return 0;
    }

    /* Write body */
    return _ev_pipe_on_ipc_mode_io_write_remain_body_unix(pipe);
}

static int _ev_pipe_on_ipc_mode_io_write_remain_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.wio.curr.head_send_size < pipe->backend.ipc_mode.wio.curr.head_send_capacity)
    {
        return _ev_pipe_on_ipc_mode_io_write_remain_head_unix(pipe);
    }

    return _ev_pipe_on_ipc_mode_io_write_remain_body_unix(pipe);
}

static int _ev_pipe_ipc_mode_write_new_frame_unix(ev_pipe_t* pipe)
{
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.curr.writing;

    ev_ipc_frame_hdr_t frame_hdr;
    ev__ipc_init_frame_hdr(&frame_hdr, 0, 0, req->base.capacity);
    memcpy(pipe->backend.ipc_mode.wio.buffer, &frame_hdr, sizeof(frame_hdr));
    pipe->backend.ipc_mode.wio.curr.head_send_capacity = sizeof(frame_hdr);

    ssize_t send_size;
    if (req->handle.role == EV_ROLE_EV_TCP)
    {
        struct iovec iov = {
            pipe->backend.ipc_mode.wio.buffer,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity
        };

        send_size = _ev_pipe_sendmsg_unix(pipe->pipfd, req->handle.u.os_socket, &iov, 1);
        if (send_size >= 0)
        {
            req->handle.role = EV_ROLE_UNKNOWN;
            req->handle.u.os_socket = EV_OS_SOCKET_INVALID;
        }
    }
    else
    {
        send_size = ev__write_unix(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity);
    }

    if (send_size == 0)
    {/* If data not send, try again */
        EV_LOG_TRACE("pipe(%p) data not send, try again", pipe);
        ev_list_push_front(&pipe->backend.ipc_mode.wio.wqueue, &req->base.node);
        pipe->backend.ipc_mode.wio.curr.writing = NULL;
        return 0;
    }
    else if (send_size < 0)
    {/* send_size is error code */
        EV_LOG_ERROR("pipe(%p) data send failed, err:%d", pipe, send_size);
        return send_size;
    }

    pipe->backend.ipc_mode.wio.curr.head_send_size = send_size;

    /* try again to send frame header */
    if ((size_t)send_size < pipe->backend.ipc_mode.wio.curr.head_send_capacity)
    {
        EV_LOG_TRACE("pipe(%p) frame header remain %zu bytes", pipe,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity - send_size);
        return 0;
    }

    return _ev_pipe_on_ipc_mode_io_write_remain_unix(pipe);
}

static void _ev_pipe_ipc_mode_reset_wio_curr_cnt(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.wio.curr.head_send_size = 0;
    pipe->backend.ipc_mode.wio.curr.head_send_capacity = 0;
    pipe->backend.ipc_mode.wio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.wio.curr.buf_pos = 0;
}

static void _ev_pipe_ipc_mode_reset_rio_curr_cnt(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.rio.curr.head_read_size = 0;
    pipe->backend.ipc_mode.rio.curr.data_remain_size = 0;
    pipe->backend.ipc_mode.rio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.rio.curr.buf_pos = 0;
}

static int _ev_pipe_on_ipc_mode_io_write_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.wio.curr.writing != NULL)
    {
        return _ev_pipe_on_ipc_mode_io_write_remain_unix(pipe);
    }

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.wqueue);
    assert(it != NULL);
    pipe->backend.ipc_mode.wio.curr.writing =
        EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
    _ev_pipe_ipc_mode_reset_wio_curr_cnt(pipe);

    return _ev_pipe_ipc_mode_write_new_frame_unix(pipe);
}

static void _ev_pipe_ipc_mode_cancel_all_rio_unix(ev_pipe_t* pipe, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.rqueue)) != NULL)
    {
        ev_pipe_read_req_t* req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_unix(pipe, req, stat);
    }
}

static void _ev_pipe_ipc_mode_cancel_all_wio_unix(ev_pipe_t* pipe, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.wqueue)) != NULL)
    {
        ev_pipe_write_req_t* req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_unix(pipe, req, stat);
    }
}

static void _ev_pipe_abort_unix(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ev__nonblock_io_del(pipe->base.loop, &pipe->backend.ipc_mode.io, EV_IO_IN | EV_IO_OUT);
        _ev_pipe_close_unix(pipe);

        _ev_pipe_ipc_mode_cancel_all_rio_unix(pipe, stat);
        _ev_pipe_ipc_mode_cancel_all_wio_unix(pipe, stat);
    }
    else
    {
        if (pipe->base.data.flags & EV_HANDLE_PIPE_STREAMING)
        {
            ev__nonblock_stream_exit(&pipe->backend.data_mode.stream);
            pipe->base.data.flags &= ~EV_HANDLE_PIPE_STREAMING;
        }
        _ev_pipe_close_unix(pipe);
    }
}

static void _ev_pipe_on_ipc_mode_io_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;

    int ret = 0;
    ev_pipe_t* pipe = EV_CONTAINER_OF(io, ev_pipe_t, backend.ipc_mode.io);

    if (evts & (EPOLLIN | EPOLLHUP))
    {
        if ((ret = _ev_pipe_on_ipc_mode_io_read_unix(pipe)) != 0)
        {
            goto err;
        }
        if (ev_list_size(&pipe->backend.ipc_mode.rio.rqueue) == 0
            && pipe->backend.ipc_mode.rio.curr.reading == NULL)
        {
            pipe->backend.ipc_mode.mask.rio_pending = 0;
            ev__nonblock_io_del(pipe->base.loop, &pipe->backend.ipc_mode.io, EPOLLIN);
        }
    }
    if (evts & (EPOLLOUT | EPOLLERR))
    {
        if ((ret = _ev_pipe_on_ipc_mode_io_write_unix(pipe)) != 0)
        {
            goto err;
        }
        if (ev_list_size(&pipe->backend.ipc_mode.wio.wqueue) == 0
            && pipe->backend.ipc_mode.wio.curr.writing == NULL)
        {
            pipe->backend.ipc_mode.mask.wio_pending = 0;
            ev__nonblock_io_del(pipe->base.loop, &pipe->backend.ipc_mode.io, EPOLLOUT);
        }
    }

    return;

err:
    _ev_pipe_abort_unix(pipe, ret);
}

static void _ev_pipe_init_as_ipc_mode_unix(ev_pipe_t* pipe)
{
    ev__nonblock_io_init(&pipe->backend.ipc_mode.io, pipe->pipfd, _ev_pipe_on_ipc_mode_io_unix, NULL);
    memset(&pipe->backend.ipc_mode.mask, 0, sizeof(pipe->backend.ipc_mode.mask));

    _ev_pipe_ipc_mode_reset_rio_curr_cnt(pipe);
    pipe->backend.ipc_mode.rio.curr.reading = NULL;
    ev_list_init(&pipe->backend.ipc_mode.rio.rqueue);

    _ev_pipe_ipc_mode_reset_wio_curr_cnt(pipe);
    pipe->backend.ipc_mode.wio.curr.writing = NULL;
    ev_list_init(&pipe->backend.ipc_mode.wio.wqueue);
}

static void _ev_pipe_ipc_mode_want_write_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.mask.wio_pending)
    {
        return;
    }

    ev__nonblock_io_add(pipe->base.loop, &pipe->backend.ipc_mode.io, EV_IO_OUT);
    pipe->backend.ipc_mode.mask.wio_pending = 1;
}

static void _ev_pipe_ipc_mode_want_read_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.mask.rio_pending)
    {
        return;
    }

    ev__nonblock_io_add(pipe->base.loop, &pipe->backend.ipc_mode.io, EV_IO_IN);
    pipe->backend.ipc_mode.mask.rio_pending = 1;
}

static int _ev_pipe_write_ipc_mode_unix(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    if (req->base.capacity > UINT32_MAX)
    {
        return EV_E2BIG;
    }

    ev_list_push_back(&pipe->backend.ipc_mode.wio.wqueue, &req->base.node);
    _ev_pipe_ipc_mode_want_write_unix(pipe);

    return 0;
}

static int _ev_pipe_read_ipc_mode_unix(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    ev_list_push_back(&pipe->backend.ipc_mode.rio.rqueue, &req->base.node);
    _ev_pipe_ipc_mode_want_read_unix(pipe);
    return 0;
}

static int _ev_pipe_make_pipe(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    int errcode;
    fds[0] = EV_OS_PIPE_INVALID;
    fds[1] = EV_OS_PIPE_INVALID;

    if (pipe(fds) < 0)
    {
        errcode = errno;
        errcode = ev__translate_sys_error(errcode);
        goto err;
    }

    if ((errcode = ev__cloexec(fds[0], 1)) != 0)
    {
        goto err;
    }
    if ((errcode = ev__cloexec(fds[1], 1)) != 0)
    {
        goto err;
    }

    if (rflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[0], 1)) != 0)
        {
            goto err;
        }
    }

    if (wflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[1], 1)) != 0)
        {
            goto err;
        }
    }

    return 0;

err:
    if (fds[0] != EV_OS_PIPE_INVALID)
    {
        close(fds[0]);
        fds[0] = EV_OS_PIPE_INVALID;
    }
    if (fds[1] != EV_OS_PIPE_INVALID)
    {
        close(fds[1]);
        fds[1] = EV_OS_PIPE_INVALID;
    }
    return errcode;
}

static int _ev_pipe_make_socketpair(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    int errcode;
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    if (rflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[0], 1)) != 0)
        {
            goto err;
        }
    }

    if (wflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[1], 1)) != 0)
        {
            goto err;
        }
    }

    return 0;

err:
    close(fds[0]);
    close(fds[1]);
    return errcode;
}

int ev_pipe_make(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    if ((rflags & EV_PIPE_IPC) != (wflags & EV_PIPE_IPC))
    {
        return EV_EINVAL;
    }

    int is_ipc = rflags & EV_PIPE_IPC;

    if (is_ipc)
    {
        return _ev_pipe_make_socketpair(fds, rflags, wflags);
    }

    return _ev_pipe_make_pipe(fds, rflags, wflags);
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;
    pipe->base.data.flags |= ipc ? EV_HANDLE_PIPE_IPC : 0;

    return 0;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    pipe->close_cb = cb;
    _ev_pipe_abort_unix(pipe, EV_ECANCELED);
    ev__handle_exit(&pipe->base, _ev_pipe_on_close_unix);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        return EV_EEXIST;
    }

    int mode = ev__fcntl_getfl_unix(handle);
    if (mode == -1)
    {
        return ev__translate_sys_error(errno);
    }

    int ret;
    if ((ret = ev__nonblock(handle, 1)) != 0)
    {
        return ret;
    }

    pipe->pipfd = handle;
    pipe->base.data.flags |= EV_HANDLE_PIPE_STREAMING;

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_init_as_ipc_mode_unix(pipe);
    }
    else
    {
        ev__nonblock_stream_init(pipe->base.loop, &pipe->backend.data_mode.stream, handle,
            _ev_pipe_on_data_mode_write_unix, _ev_pipe_on_data_mode_read_unix);
    }

    return 0;
}

int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size,
    ev_pipe_write_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_write_init_ext(req, cb, bufs, nbuf,
        handle_role, handle_addr, handle_size);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_write_ipc_mode_unix(pipe, req);
    }
    else
    {
        ret = ev__nonblock_stream_write(&pipe->backend.data_mode.stream, &req->base);
    }

    if (ret != 0)
    {
        ev__handle_event_dec(&pipe->base);
        _ev_pipe_abort_unix(pipe, ret);
    }

    return ret;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_read_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_read_init(req, bufs, nbuf, cb);
    if (ret != 0)
    {
        return ret;
    }

    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_read_ipc_mode_unix(pipe, req);
    }
    else
    {
        ret = ev__nonblock_stream_read(&pipe->backend.data_mode.stream, &req->base);
    }

    if (ret != 0)
    {
        ev__handle_event_dec(&pipe->base);
        _ev_pipe_abort_unix(pipe, ret);
    }

    return ret;
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    if (!(pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
        || handle_role != EV_ROLE_EV_TCP
        || handle_addr == NULL)
    {
        return EV_EINVAL;
    }
    if (req->handle.os_socket == EV_OS_SOCKET_INVALID)
    {
        return EV_ENOENT;
    }
    if (handle_size < sizeof(ev_buf_t))
    {
        return EV_ENOMEM;
    }

    ev_tcp_t* tcp = handle_addr;
    return ev__tcp_open(tcp, req->handle.os_socket);
}

void ev_pipe_close(ev_os_pipe_t fd)
{
    if (fd != EV_OS_PIPE_INVALID)
    {
        close(fd);
    }
}

#line 1 "src/unix/process_unix.c"
#define _GNU_SOURCE
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "allocator.h" */
/* AMALGAMATE: #include "loop_unix.h" */
/* AMALGAMATE: #include "io_unix.h" */
/* AMALGAMATE: #include "pipe.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "process_unix.h" */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdio.h>

typedef struct dup_pair_s
{
    int*    src;
    int     dst;
} dup_pair_t;

typedef struct spawn_helper_s
{
    /**
     * Channel for internal IPC.
     *
     * pipefd[0] refers to the read end of the pipe. pipefd[1] refers to the
     * write end of the pipe.
     *
     * The data always system errno.
     */
    int         pipefd[2];

    /**
     * The file descriptor always need close after fork().
     */
    struct
    {
        int     fd_stdin;
        int     fd_stdout;
        int     fd_stderr;
    } child;
} spawn_helper_t;

static void _ev_spawn_write_errno_and_exit(spawn_helper_t* helper, int value)
{
    ssize_t n;
    do
    {
        n = write(helper->pipefd[1], &value, sizeof(value));
    } while (n == -1 && errno == EINTR);

    exit(EXIT_FAILURE);
}

static int _ev_process_setup_child_fd(int fd)
{
    int ret;

    /* must in block mode */
    if ((ret = ev__nonblock(fd, 0)) != 0)
    {
        return ret;
    }

    /* cannot have FD_CLOEXEC */
    if ((ret = ev__cloexec(fd, 0)) != 0)
    {
        return ret;
    }

    return 0;
}

static int _ev_spawn_setup_stdio_as_fd(int* handle, int fd)
{
    int dup_fd = dup(fd);
    if (dup_fd < 0)
    {
        int errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    int ret = _ev_process_setup_child_fd(dup_fd);
    if (ret != 0)
    {
        close(dup_fd);
        return ret;
    }

    *handle = dup_fd;
    return 0;
}

static int _ev_spawn_setup_stdio_as_null(int* handle, int mode)
{
    int null_fd = open("/dev/null", mode);
    if (null_fd < 0)
    {
        int errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    int ret = _ev_process_setup_child_fd(null_fd);
    if (ret != 0)
    {
        close(null_fd);
        return ret;
    }

    *handle = null_fd;
    return 0;
}

static int _ev_spawn_setup_stdio_as_pipe(ev_pipe_t* pipe, int* handle, int is_pipe_read)
{
    int errcode;
    ev_os_pipe_t pipfd[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };

    /* fd for #ev_pipe_t should open in nonblock mode */
    int rflags = is_pipe_read ? EV_PIPE_NONBLOCK : 0;
    int wflags = is_pipe_read ? 0 : EV_PIPE_NONBLOCK;

    errcode = ev_pipe_make(pipfd, rflags, wflags);
    if (errcode != 0)
    {
        return errcode;
    }

    errcode = _ev_process_setup_child_fd(is_pipe_read ? pipfd[1] : pipfd[0]);
    if (errcode != 0)
    {
        goto err_exit;
    }

    errcode = ev_pipe_open(pipe, is_pipe_read ? pipfd[0] : pipfd[1]);
    if (errcode != 0)
    {
        goto err_exit;
    }

    *handle = is_pipe_read ? pipfd[1] : pipfd[0];
    return 0;

err_exit:
    ev_pipe_close(pipfd[0]);
    ev_pipe_close(pipfd[1]);
    return errcode;
}

static void _ev_spawn_dup_stdio(spawn_helper_t* helper)
{
    int ret = 0;
    dup_pair_t dup_list[] = {
        { &helper->child.fd_stdin, STDIN_FILENO },
        { &helper->child.fd_stdout, STDOUT_FILENO },
        { &helper->child.fd_stderr, STDERR_FILENO },
    };

    size_t i;
    for (i = 0; i < ARRAY_SIZE(dup_list); i++)
    {
        if (*dup_list[i].src == EV_OS_PIPE_INVALID)
        {
            continue;
        }

        ret = dup2(*dup_list[i].src, dup_list[i].dst);
        if (ret < 0)
        {
            ret = errno;
            goto err_dup;
        }

        close(*dup_list[i].src);
        *dup_list[i].src = EV_OS_PIPE_INVALID;
    }

    return;

err_dup:
    _ev_spawn_write_errno_and_exit(helper, ret);
}

static void _ev_spawn_child(spawn_helper_t* helper, const ev_process_options_t* opt)
{
    int errcode;
    _ev_spawn_dup_stdio(helper);

    if (opt->cwd != NULL && chdir(opt->cwd))
    {
        errcode = errno;
        _ev_spawn_write_errno_and_exit(helper, errcode);
    }

    const char* file = opt->file != NULL ? opt->file : opt->argv[0];

    if (opt->envp == NULL)
    {
        (void)execvp(file, opt->argv);
    }
    else
    {
        (void)execvpe(file, opt->argv, opt->envp);
    }

    /* Error */
    errcode = errno;
    _ev_spawn_write_errno_and_exit(helper, errcode);
}

static int _ev_spawn_parent(ev_process_t* handle, spawn_helper_t* spawn_helper)
{
    int status;
    pid_t pid_ret;
    int child_errno = 0;
    ssize_t r;

    do
    {
        r = read(spawn_helper->pipefd[0], &child_errno, sizeof(child_errno));
    } while (r == -1 && errno == EINTR);

    /* EOF, child exec success */
    if (r == 0)
    {
        return 0;
    }

    if (r == sizeof(child_errno))
    {
        do
        {
            pid_ret = waitpid(handle->pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == handle->pid);

        return ev__translate_sys_error(child_errno);
    }

    /* Something unknown happened to our child before spawn */
    if (r == -1 && errno == EPIPE)
    {
        do
        {
            pid_ret = waitpid(handle->pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == handle->pid);

        return EV_EPIPE;
    }

    /* unknown error */
    EV_ABORT();

    return EV_EPIPE;
}

EV_LOCAL void ev__init_process_unix(void)
{
    ev_list_init(&g_ev_loop_unix_ctx.process.wait_queue);
    ev_mutex_init(&g_ev_loop_unix_ctx.process.wait_queue_mutex, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    if (sigfillset(&sa.sa_mask) != 0)
    {
        EV_ABORT();
    }

    sa.sa_handler = ev_process_sigchld;

    if (sigaction(SIGCHLD, &sa, NULL) != 0)
    {
        EV_ABORT();
    }
}

static void _ev_process_on_async_close(ev_async_t* async)
{
    ev_process_t* handle = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    if (handle->exit_cb != NULL)
    {
        handle->exit_cb(handle);
    }
}

static void _ev_process_on_sigchild_unix(ev_async_t* async)
{
    ev_process_t* handle = EV_CONTAINER_OF(async, ev_process_t, sigchld);
    if (handle->backend.flags.waitpid)
    {
        return;
    }

    int wstatus;
    pid_t pid_ret;
    do
    {
        pid_ret = waitpid(handle->pid, &wstatus, WNOHANG);
    } while(pid_ret == -1 && errno == EINTR);

    if (pid_ret == 0)
    {
        return;
    }
    if (pid_ret == -1)
    {
        int errcode = errno;

        /**
         * The child died, and we missed it. This probably means someone else
         * stole the waitpid().
         */
        if (errcode == ECHILD)
        {
            handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
            goto fin;
        }
    }

    assert(pid_ret == handle->pid);

    if (WIFEXITED(wstatus))
    {
        handle->exit_status = EV_PROCESS_EXIT_NORMAL;
        handle->exit_code = WEXITSTATUS(wstatus);
    }

    if (WIFSIGNALED(wstatus))
    {
        handle->exit_status = EV_PROCESS_EXIT_SIGNAL;
        handle->exit_code = WTERMSIG(wstatus);
    }

fin:
    handle->backend.flags.waitpid = 1;

    if (handle->sigchild_cb != NULL)
    {
        handle->sigchild_cb(handle, handle->exit_status, handle->exit_code);
    }
}

static int _ev_process_init_process(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;

    handle->sigchild_cb = opt->on_exit;
    handle->exit_cb = NULL;
    handle->pid = EV_OS_PID_INVALID;
    handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
    handle->exit_code = 0;
    memset(&handle->backend.flags, 0, sizeof(handle->backend.flags));

    ret = ev_async_init(loop, &handle->sigchld, _ev_process_on_sigchild_unix);
    if (ret != 0)
    {
        return ret;
    }

    ev_mutex_enter(&g_ev_loop_unix_ctx.process.wait_queue_mutex);
    {
        ev_list_push_back(&g_ev_loop_unix_ctx.process.wait_queue, &handle->node);
    }
    ev_mutex_leave(&g_ev_loop_unix_ctx.process.wait_queue_mutex);

    return 0;
}

static int _ev_process_setup_child_stdin(spawn_helper_t* helper,
        const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_spawn_setup_stdio_as_null(&helper->child.fd_stdin, O_RDONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stdin, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stdin, 0);
    }

    return 0;
}

static int _ev_process_setup_child_stdout(spawn_helper_t* helper,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return  _ev_spawn_setup_stdio_as_null(&helper->child.fd_stdout, O_WRONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stdout, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stdout, 1);
    }

    return 0;
}

static int _ev_process_setup_child_stderr(spawn_helper_t* helper,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return  _ev_spawn_setup_stdio_as_null(&helper->child.fd_stderr, O_WRONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stderr, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stderr, 1);
    }

    return 0;
}

static void _ev_process_close_read_end(spawn_helper_t* helper)
{
    if (helper->pipefd[0] != EV_OS_PIPE_INVALID)
    {
        close(helper->pipefd[0]);
        helper->pipefd[0] = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_write_end(spawn_helper_t* helper)
{
    if (helper->pipefd[1] != EV_OS_PIPE_INVALID)
    {
        close(helper->pipefd[1]);
        helper->pipefd[1] = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stdin(spawn_helper_t* helper)
{
    if (helper->child.fd_stdin != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stdin);
        helper->child.fd_stdin = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stdout(spawn_helper_t* helper)
{
    if (helper->child.fd_stdout != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stdout);
        helper->child.fd_stdout = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stderr(spawn_helper_t* helper)
{
    if (helper->child.fd_stderr != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stderr);
        helper->child.fd_stderr = EV_OS_PIPE_INVALID;
    }
}

static int _ev_process_init_spawn_helper(spawn_helper_t* helper, const ev_process_options_t* opt)
{
    int ret;
    memset(helper, 0, sizeof(*helper));
    helper->child.fd_stdin = EV_OS_PIPE_INVALID;
    helper->child.fd_stdout = EV_OS_PIPE_INVALID;
    helper->child.fd_stderr = EV_OS_PIPE_INVALID;

    if ((ret = pipe2(helper->pipefd, O_CLOEXEC)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    if ((ret = _ev_process_setup_child_stdin(helper, &opt->stdios[0])) != 0)
    {
        goto err_close_pipe;
    }

    if ((ret = _ev_process_setup_child_stdout(helper, &opt->stdios[1])) != 0)
    {
        goto err_close_stdin;
    }

    if ((ret = _ev_process_setup_child_stderr(helper, &opt->stdios[2])) != 0)
    {
        goto err_close_stdout;
    }

    return 0;

err_close_stdout:
    _ev_process_close_stdout(helper);
err_close_stdin:
    _ev_process_close_stdin(helper);
err_close_pipe:
    _ev_process_close_read_end(helper);
    _ev_process_close_write_end(helper);
    return ret;
}

static void _ev_process_exit_spawn_helper(spawn_helper_t* helper)
{
    _ev_process_close_read_end(helper);
    _ev_process_close_write_end(helper);
    _ev_process_close_stdin(helper);
    _ev_process_close_stdout(helper);
    _ev_process_close_stderr(helper);
}

int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;

    spawn_helper_t spawn_helper;
    if ((ret = _ev_process_init_spawn_helper(&spawn_helper, opt)) != 0)
    {
        return ret;
    }

    if ((ret = _ev_process_init_process(loop, handle, opt)) != 0)
    {
        goto finish;
    }

    handle->pid = fork();
    switch (handle->pid)
    {
    case -1:    /* fork failed */
        ret = errno;
        ret = ev__translate_sys_error(ret);
        goto finish;

    case 0:     /* Child process */
        _ev_process_close_read_end(&spawn_helper);
        _ev_spawn_child(&spawn_helper, opt);
        break;

    default:    /* parent process */
        _ev_process_close_write_end(&spawn_helper);
        ret = _ev_spawn_parent(handle, &spawn_helper);
        goto finish;
    }

    /* should not reach here. */
    EV_ABORT();
    return EV_EPIPE;

finish:
    _ev_process_exit_spawn_helper(&spawn_helper);
    return ret;
}

void ev_process_exit(ev_process_t* handle, ev_process_exit_cb cb)
{
    if (handle->pid != EV_OS_PID_INVALID)
    {
        handle->pid = EV_OS_PID_INVALID;
    }

    ev_mutex_enter(&g_ev_loop_unix_ctx.process.wait_queue_mutex);
    {
        ev_list_erase(&g_ev_loop_unix_ctx.process.wait_queue, &handle->node);
    }
    ev_mutex_leave(&g_ev_loop_unix_ctx.process.wait_queue_mutex);

    handle->exit_cb = cb;
    ev_async_exit(&handle->sigchld, _ev_process_on_async_close);
}

void ev_process_sigchld(int signum)
{
    assert(signum == SIGCHLD); (void)signum;

    ev_list_node_t* it = ev_list_begin(&g_ev_loop_unix_ctx.process.wait_queue);
    for (; it != NULL; it = ev_list_next(it))
    {
        ev_process_t* handle = EV_CONTAINER_OF(it, ev_process_t, node);
        ev_async_wakeup(&handle->sigchld);
    }
}

ssize_t ev_getcwd(char* buffer, size_t size)
{
    size_t str_len;
    int errcode;

    if (buffer != NULL && getcwd(buffer, size) != NULL)
    {
        str_len = strlen(buffer);
        if (buffer[str_len - 1] == '/')
        {
            buffer[str_len - 1] = '\0';
            str_len -= 1;
        }

        return str_len;
    }

    const size_t max_path_size = PATH_MAX + 1;
    char* tmp_buf = ev_malloc(max_path_size);
    if (tmp_buf == NULL)
    {
        return EV_ENOMEM;
    }

    if (getcwd(tmp_buf, max_path_size) == NULL)
    {
        errcode = errno;
        ev_free(tmp_buf);
        return ev__translate_sys_error(errcode);
    }

    str_len = strlen(tmp_buf);
    if (tmp_buf[str_len - 1] == '/')
    {
        tmp_buf[str_len - 1] = '\0';
        str_len -= 1;
    }

    if (buffer != NULL)
    {
        snprintf(buffer, size, "%s", tmp_buf);
    }

    ev_free(tmp_buf);
    return str_len;
}

ssize_t ev_exepath(char* buffer, size_t size)
{
    int errcode;

    size_t tmp_size = PATH_MAX;
    char* tmp_buffer = ev_malloc(tmp_size);
    if (tmp_buffer == NULL)
    {
        errcode = EV_ENOMEM;
        goto error;
    }

    ssize_t ret = readlink("/proc/self/exe", tmp_buffer, tmp_size - 1);
    if (ret < 0)
    {
        errcode = ev__translate_sys_error(errno);
        goto error;
    }
    tmp_buffer[ret] = '\0';

    if (buffer != NULL)
    {
        ret = snprintf(buffer, size, "%s", tmp_buffer);
    }
    ev_free(tmp_buffer);

    return ret;

error:
    if (buffer != NULL && size > 0)
    {
        buffer[0] = '\0';
    }
    ev_free(tmp_buffer);
    return errcode;
}

#line 1 "src/unix/sem_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop.h" */

void ev_sem_init(ev_sem_t* sem, unsigned value)
{
    if (sem_init(&sem->u.r, 0, value))
    {
        EV_ABORT();
    }
}

void ev_sem_exit(ev_sem_t* sem)
{
    if (sem_destroy(&sem->u.r))
    {
        EV_ABORT();
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    if (sem_post(&sem->u.r))
    {
        EV_ABORT();
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    int r;
    do
    {
        r = sem_wait(&sem->u.r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        EV_ABORT();
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    int r;

    do
    {
        r = sem_trywait(&sem->u.r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        if (errno == EAGAIN)
        {
            return EV_EAGAIN;
        }
        EV_ABORT();
    }

    return 0;
}

#line 1 "src/unix/shmem_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "shmem.h" */
/* AMALGAMATE: #include "loop.h" */
/* AMALGAMATE: #include "misc_unix.h" */
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int ev_shm_init(ev_shm_t* shm, const char* key, size_t size)
{
    int err;
    shm->size = size;

    int ret = snprintf(shm->backend.name, sizeof(shm->backend.name), "%s", key);
    if (ret >= (int)sizeof(shm->backend.name))
    {
        return EV_ENOMEM;
    }
    memset(&shm->backend.mask, 0, sizeof(shm->backend.mask));

    shm->backend.map_file = shm_open(key, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm->backend.map_file == -1)
    {
        err = errno;
        goto err_shm_open;
    }

    if (ftruncate(shm->backend.map_file, size) != 0)
    {
        err = errno;
        goto err_ftruncate;
    }

    shm->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->backend.map_file, 0);
    if (shm->addr == NULL)
    {
        err = errno;
        goto err_ftruncate;
    }

    return 0;

err_ftruncate:
    close(shm->backend.map_file);
err_shm_open:
    return ev__translate_sys_error(err);
}

int ev_shm_open(ev_shm_t* shm, const char* key)
{
    int err;
    int ret = snprintf(shm->backend.name, sizeof(shm->backend.name), "%s", key);
    if (ret >= (int)sizeof(shm->backend.name))
    {
        return EV_ENOMEM;
    }
    memset(&shm->backend.mask, 0, sizeof(shm->backend.mask));

    shm->backend.mask.is_open = 1;
    shm->backend.map_file = shm_open(key, O_RDWR, 0);
    if (shm->backend.map_file == -1)
    {
        err = errno;
        goto err_shm_open;
    }

    struct stat statbuf;
    if (fstat(shm->backend.map_file, &statbuf) != 0)
    {
        err = errno;
        goto err_fstat;
    }
    shm->size = statbuf.st_size;

    shm->addr = mmap(NULL, shm->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->backend.map_file, 0);
    if (shm->addr == NULL)
    {
        err = errno;
        goto err_fstat;
    }

    return 0;

err_fstat:
    close(shm->backend.map_file);
err_shm_open:
    return ev__translate_sys_error(err);
}

void ev_shm_exit(ev_shm_t* shm)
{
    if (!shm->backend.mask.is_open)
    {
        shm_unlink(shm->backend.name);
    }

    int ret = munmap(shm->addr, shm->size);
    assert(ret == 0); (void)ret;

    close(shm->backend.map_file);
}

#line 1 "src/unix/stream_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "stream_unix.h" */

static ssize_t _ev_stream_do_write_writev_unix(int fd, struct iovec* iov, int iovcnt, void* arg)
{
    (void)arg;
    return ev__writev_unix(fd, (ev_buf_t*)iov, iovcnt);
}

static int _ev_stream_do_write_once(ev_nonblock_stream_t* stream, ev_write_t* req)
{
    return ev__send_unix(stream->io.data.fd, req, _ev_stream_do_write_writev_unix, NULL);
}

static int _ev_stream_do_read_once(ev_nonblock_stream_t* stream, ev_read_t* req, size_t* size)
{
    int iovcnt = req->data.nbuf;
    if (iovcnt > g_ev_loop_unix_ctx.iovmax)
    {
        iovcnt = g_ev_loop_unix_ctx.iovmax;
    }

    ssize_t read_size;
    read_size = ev__readv_unix(stream->io.data.fd, req->data.bufs, iovcnt);
    if (read_size >= 0)
    {
        *size = read_size;
        return 0;
    }
    return read_size;
}

static void _ev_stream_do_write(ev_nonblock_stream_t* stream)
{
    ssize_t ret;
    ev_list_node_t* it;
    ev_write_t* req;

    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_write_t, node);
        if ((ret = _ev_stream_do_write_once(stream, req)) == 0)
        {
            stream->callbacks.w_cb(stream, req, req->size);
            continue;
        }

        /* Unsuccess operation should restore list */
        ev_list_push_front(&stream->pending.w_queue, it);

        if (ret == EV_EAGAIN)
        {
            break;
        }
        goto err;
    }

    return;

err:
    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_write_t, node);
        ret = ret < 0 ? ret : (ssize_t)req->size;
        stream->callbacks.w_cb(stream, req, ret);
    }
}

static void _ev_stream_do_read(ev_nonblock_stream_t* stream)
{
    int ret;
    ev_list_node_t* it = ev_list_pop_front(&stream->pending.r_queue);
    ev_read_t* req = EV_CONTAINER_OF(it, ev_read_t, node);

    size_t r_size = 0;
    ret = _ev_stream_do_read_once(stream, req, &r_size);
    req->data.size += r_size;

    if (ret == 0)
    {
        stream->callbacks.r_cb(stream, req, req->data.size);
        return;
    }

    ev_list_push_front(&stream->pending.r_queue, it);

    if (ret == EV_EAGAIN)
    {
        return;
    }

    /* If error, cleanup all pending read requests */
    while ((it = ev_list_pop_front(&stream->pending.r_queue)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, ret);
    }
}

static void _ev_stream_cleanup_r(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.r_queue)) != NULL)
    {
        ev_read_t* req = EV_CONTAINER_OF(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, errcode);
    }
}

static void _ev_stream_cleanup_w(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        ev_write_t* req = EV_CONTAINER_OF(it, ev_write_t, node);
        stream->callbacks.w_cb(stream, req, errcode);
    }
}

static void _ev_nonblock_stream_on_io(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;
    ev_nonblock_stream_t* stream = EV_CONTAINER_OF(io, ev_nonblock_stream_t, io);

    if (evts & EPOLLOUT)
    {
        _ev_stream_do_write(stream);
        if (ev_list_size(&stream->pending.w_queue) == 0)
        {
            ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_OUT);
            stream->flags.io_reg_w = 0;
        }
    }

    else if (evts & (EPOLLIN | EPOLLHUP))
    {
        _ev_stream_do_read(stream);
        if (ev_list_size(&stream->pending.r_queue) == 0)
        {
            ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_IN);
            stream->flags.io_reg_r = 0;
        }
    }
}

EV_LOCAL void ev__nonblock_stream_init(ev_loop_t* loop,
    ev_nonblock_stream_t* stream, int fd, ev_stream_write_cb wcb,
    ev_stream_read_cb rcb)
{
    stream->loop = loop;

    stream->flags.io_abort = 0;
    stream->flags.io_reg_r = 0;
    stream->flags.io_reg_w = 0;

    ev__nonblock_io_init(&stream->io, fd, _ev_nonblock_stream_on_io, NULL);

    ev_list_init(&stream->pending.w_queue);
    ev_list_init(&stream->pending.r_queue);

    stream->callbacks.w_cb = wcb;
    stream->callbacks.r_cb = rcb;
}

EV_LOCAL void ev__nonblock_stream_exit(ev_nonblock_stream_t* stream)
{
    ev__nonblock_stream_abort(stream);
    ev__nonblock_stream_cleanup(stream, EV_IO_IN | EV_IO_OUT);
    stream->loop = NULL;
    stream->callbacks.w_cb = NULL;
    stream->callbacks.r_cb = NULL;
}

EV_LOCAL int ev__nonblock_stream_write(ev_nonblock_stream_t* stream, ev_write_t* req)
{
    if (stream->flags.io_abort)
    {
        return EV_EBADF;
    }

    if (!stream->flags.io_reg_w)
    {
        ev__nonblock_io_add(stream->loop, &stream->io, EV_IO_OUT);
        stream->flags.io_reg_w = 1;
    }

    ev_list_push_back(&stream->pending.w_queue, &req->node);
    return 0;
}

EV_LOCAL int ev__nonblock_stream_read(ev_nonblock_stream_t* stream, ev_read_t* req)
{
    if (stream->flags.io_abort)
    {
        return EV_EBADF;
    }

    if (!stream->flags.io_reg_r)
    {
        ev__nonblock_io_add(stream->loop, &stream->io, EV_IO_IN);
        stream->flags.io_reg_r = 1;
    }

    ev_list_push_back(&stream->pending.r_queue, &req->node);
    return 0;
}

EV_LOCAL size_t ev__nonblock_stream_size(ev_nonblock_stream_t* stream, unsigned evts)
{
    size_t ret = 0;
    if (evts & EV_IO_IN)
    {
        ret += ev_list_size(&stream->pending.r_queue);
    }
    if (evts & EV_IO_OUT)
    {
        ret += ev_list_size(&stream->pending.w_queue);
    }
    return ret;
}

EV_LOCAL void ev__nonblock_stream_abort(ev_nonblock_stream_t* stream)
{
    if (!stream->flags.io_abort)
    {
        ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_IN | EV_IO_OUT);
        stream->flags.io_abort = 1;
    }
}

EV_LOCAL void ev__nonblock_stream_cleanup(ev_nonblock_stream_t* stream, unsigned evts)
{
    if (evts & EV_IO_OUT)
    {
        _ev_stream_cleanup_w(stream, EV_ECANCELED);
    }

    if (evts & EV_IO_IN)
    {
        _ev_stream_cleanup_r(stream, EV_ECANCELED);
    }
}

#line 1 "src/unix/tcp_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "loop_unix.h" */
/* AMALGAMATE: #include "stream_unix.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "tcp_unix.h" */
/* AMALGAMATE: #include "handle.h" */
#include <sys/uio.h>
#include <assert.h>
#include <unistd.h>

static void _ev_tcp_close_fd(ev_tcp_t* sock)
{
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        close(sock->sock);
        sock->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_tcp_cleanup_listen_queue(ev_tcp_t* s_sock, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&s_sock->backend.u.listen.accept_queue)) != NULL)
    {
        ev_tcp_t* c_sock = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.accept_node);
        c_sock->backend.u.accept.cb(s_sock, c_sock, stat);
    }
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);

    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        ev__nonblock_stream_exit(&sock->backend.u.stream);
        sock->base.data.flags &= ~EV_HANDLE_TCP_STREAMING;
    }

    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        _ev_tcp_cleanup_listen_queue(sock, EV_ECANCELED);
        sock->base.data.flags &= ~EV_HANDLE_TCP_LISTING;
    }

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        sock->backend.u.client.cb(sock, EV_ECANCELED);
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    }

    if (sock->close_cb != NULL)
    {
        sock->close_cb(sock);
    }
}

static void _ev_tcp_on_connect(ev_tcp_t* sock)
{
    int ret;
    socklen_t result_len = sizeof(ret);

    ev__nonblock_io_del(sock->base.loop, &sock->backend.u.client.io, EV_IO_OUT);
    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;

    /* Get connect result */
    if (getsockopt(sock->sock, SOL_SOCKET, SO_ERROR, &ret, &result_len) < 0)
    {
        sock->backend.u.client.stat = ev__translate_sys_error(errno);
        goto fin;
    }

    /* Result is in `result` */
    sock->backend.u.client.stat = ev__translate_sys_error(ret);

fin:
    ev__handle_event_dec(&sock->base);
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_w_user_callback_unix(ev_tcp_t* sock, ev_tcp_write_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&sock->base);
    ev__write_exit(&req->base);
    req->user_callback(req, size);
}

static void _ev_tcp_r_user_callback_unix(ev_tcp_t* sock, ev_tcp_read_req_t* req, ssize_t size)
{
    ev__handle_event_dec(&sock->base);
    ev__read_exit(&req->base);
    req->user_callback(req, size);
}

static void _on_tcp_write_done(ev_nonblock_stream_t* stream, ev_write_t* req, ssize_t size)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(stream, ev_tcp_t, backend.u.stream);
    ev_tcp_write_req_t* w_req = EV_CONTAINER_OF(req, ev_tcp_write_req_t, base);
    _ev_tcp_w_user_callback_unix(sock, w_req, size);
}

static void _on_tcp_read_done(ev_nonblock_stream_t* stream, ev_read_t* req, ssize_t size)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(stream, ev_tcp_t, backend.u.stream);

    ev_tcp_read_req_t* r_req = EV_CONTAINER_OF(req, ev_tcp_read_req_t, base);
    _ev_tcp_r_user_callback_unix(sock, r_req, size);
}

static void _ev_tcp_accept_user_callback_unix(ev_tcp_t* acpt, ev_tcp_t* conn, int ret)
{
    ev__handle_event_dec(&conn->base);
    ev__handle_event_dec(&acpt->base);
    conn->backend.u.accept.cb(acpt, conn, ret);
}

static void _ev_tcp_on_accept(ev_tcp_t* acpt)
{
    ev_list_node_t* it = ev_list_pop_front(&acpt->backend.u.listen.accept_queue);
    if (it == NULL)
    {
        EV_ABORT("empty accept queue");
    }

    ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.accept_node);
    _ev_tcp_close_fd(conn);

    do
    {
        conn->sock = accept(acpt->sock, NULL, NULL);
    } while (conn->sock == -1 && errno == EINTR);

    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;

    int ret = conn->sock >= 0 ? 0 : ev__translate_sys_error(errno);
    if (ret == 0)
    {/* Set non-block mode */
        if ((ret = ev__nonblock(conn->sock, 1)) != 0)
        {
            _ev_tcp_close_fd(conn);
        }
    }

    _ev_tcp_accept_user_callback_unix(acpt, conn, ret);

    /* might be close in callback */
    if (ev__handle_is_closing(&acpt->base))
    {
        return;
    }
    if (ev_list_size(&acpt->backend.u.listen.accept_queue) == 0)
    {
        ev__nonblock_io_del(acpt->base.loop, &acpt->backend.u.listen.io, EV_IO_IN);
    }
}

static void _ev_tcp_on_server_event(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_tcp_t* sock = EV_CONTAINER_OF(io, ev_tcp_t, backend.u.listen.io);

    _ev_tcp_on_accept(sock);
}

static void _ev_tcp_on_client_event(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_tcp_t* sock = EV_CONTAINER_OF(io, ev_tcp_t, backend.u.client.io);

    _ev_tcp_on_connect(sock);
}

/**
 * @return #ev_errno_t
 */
static int _ev_tcp_setup_fd(ev_tcp_t* sock, int domain, int is_server, int* new_fd)
{
    int ret = 0;
    int tmp_new_fd = 0;
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        goto fin;
    }

    if ((sock->sock = socket(domain, SOCK_STREAM, 0)) == EV_OS_SOCKET_INVALID)
    {
        ret = ev__translate_sys_error(errno);
        goto fin;
    }

    if ((ret = ev__nonblock(sock->sock, 1)) != 0)
    {
        goto err_nonblock;
    }

    tmp_new_fd = 1;
    if (is_server)
    {
        ev__nonblock_io_init(&sock->backend.u.listen.io, sock->sock, _ev_tcp_on_server_event, NULL);
    }
    else
    {
        ev__nonblock_io_init(&sock->backend.u.client.io, sock->sock, _ev_tcp_on_client_event, NULL);
    }

    goto fin;

err_nonblock:
    _ev_tcp_close_fd(sock);
fin:
    if (new_fd != NULL)
    {
        *new_fd = tmp_new_fd;
    }
    return ret;
}

static int _ev_tcp_is_listening(ev_tcp_t* sock)
{
    return sock->base.data.flags & EV_HANDLE_TCP_LISTING;
}

static void _ev_tcp_to_connect(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);

    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    sock->backend.u.client.cb(sock, 0);
}

static void _ev_tcp_setup_stream_once(ev_tcp_t* sock)
{
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        return;
    }
    ev__nonblock_stream_init(sock->base.loop, &sock->backend.u.stream, sock->sock,
        _on_tcp_write_done, _on_tcp_read_done);
    sock->base.data.flags |= EV_HANDLE_TCP_STREAMING;
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* sock)
{
    ev__handle_init(loop, &sock->base, EV_ROLE_EV_TCP);
    sock->close_cb = NULL;
    sock->sock = EV_OS_SOCKET_INVALID;

    return 0;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    /* Stop all pending IO actions */
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        ev__nonblock_stream_abort(&sock->backend.u.stream);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        ev__nonblock_io_del(sock->base.loop, &sock->backend.u.listen.io, EV_IO_IN);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        ev__nonblock_io_del(sock->base.loop, &sock->backend.u.client.io, EV_IO_OUT);
    }

    /* Close fd */
    _ev_tcp_close_fd(sock);

    sock->close_cb = cb;
    ev__handle_exit(&sock->base, _ev_tcp_on_close);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_fd;
    if ((ret = _ev_tcp_setup_fd(tcp, addr->sa_family, 1, &flag_new_fd)) != 0)
    {
        return ret;
    }

    if ((ret = bind(tcp->sock, addr, addrlen)) != 0)
    {
        ret = ev__translate_sys_error(errno);
        goto err_bind;
    }
    tcp->base.data.flags |= EV_HABDLE_TCP_BOUND;

    return 0;

err_bind:
    if (flag_new_fd)
    {
        _ev_tcp_close_fd(tcp);
    }
    return ret;
}

int ev_tcp_listen(ev_tcp_t* tcp, int backlog)
{
    if (_ev_tcp_is_listening(tcp))
    {
        return EV_EADDRINUSE;
    }

    int ret;
    if ((ret = listen(tcp->sock, backlog)) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    ev_list_init(&tcp->backend.u.listen.accept_queue);
    tcp->base.data.flags |= EV_HANDLE_TCP_LISTING;

    return 0;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    if (conn->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        return EV_EINPROGRESS;
    }
    assert(cb != NULL);

    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb = cb;
    ev_list_push_back(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.accept_node);
    ev__nonblock_io_add(lisn->base.loop, &lisn->backend.u.listen.io, EV_IO_IN);

    ev__handle_event_add(&lisn->base);
    ev__handle_event_add(&conn->base);

    return 0;
}

int ev_tcp_write(ev_tcp_t* sock, ev_tcp_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb)
{
    req->user_callback = cb;
    int ret = ev__write_init(&req->base, bufs, nbuf);
    if (ret != 0)
    {
        return ret;
    }

    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    _ev_tcp_setup_stream_once(sock);

    ev__handle_event_add(&sock->base);
    ret = ev__nonblock_stream_write(&sock->backend.u.stream, &req->base);

    if (ret != 0)
    {
        ev__handle_event_dec(&sock->base);
        return ret;
    }
    return 0;
}

int ev_tcp_read(ev_tcp_t* sock, ev_tcp_read_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    req->user_callback = cb;
    int ret = ev__read_init(&req->base, bufs, nbuf);
    if (ret != 0)
    {
        return ret;
    }

    _ev_tcp_setup_stream_once(sock);

    ev__handle_event_add(&sock->base);
    ret = ev__nonblock_stream_read(&sock->backend.u.stream, &req->base);

    if (ret != 0)
    {
        ev__handle_event_dec(&sock->base);
        return ret;
    }
    return 0;
}

int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    socklen_t socklen = *len;
    if (getsockname(sock->sock, name, &socklen) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    *len = (size_t)socklen;
    return 0;
}

int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    socklen_t socklen = *len;

    if ((ret = getpeername(sock->sock, name, &socklen)) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    *len = socklen;
    return 0;
}

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_tcp_connect_cb cb)
{
    int ret;
    ev_loop_t* loop = sock->base.loop;

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        return EV_EINPROGRESS;
    }
    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_STREAMING))
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_tcp_setup_fd(sock, addr->sa_family, 0, NULL)) != 0)
    {
        return ret;
    }

    sock->backend.u.client.cb = cb;
    sock->base.data.flags |= EV_HANDLE_TCP_CONNECTING;

    if ((ret = connect(sock->sock, addr, size)) == 0)
    {/* Connect success immediately */
        sock->backend.u.client.stat = 0;
        ev__backlog_submit(&sock->base, _ev_tcp_to_connect);
        return 0;
    }

    if (errno != EINPROGRESS)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        ret = ev__translate_sys_error(errno);
        goto err;
    }

    ev__handle_event_add(&sock->base);
    ev__nonblock_io_add(loop, &sock->backend.u.client.io, EV_IO_OUT);

    return 0;

err:
    _ev_tcp_close_fd(sock);
    return ret;
}

EV_LOCAL int ev__tcp_open(ev_tcp_t* tcp, int fd)
{
    int busy_flags = EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING |
            EV_HANDLE_TCP_STREAMING | EV_HANDLE_TCP_CONNECTING | EV_HABDLE_TCP_BOUND;
    if (tcp->base.data.flags & busy_flags)
    {
        return EV_EBUSY;
    }

    tcp->sock = fd;
    _ev_tcp_setup_stream_once(tcp);

    return 0;
}

#line 1 "src/unix/thread_unix.c"
#define _GNU_SOURCE
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "loop.h" */
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef struct ev_thread_helper_unix
{
    ev_thread_cb    cb;
    void*           arg;
    sem_t           sem;
}ev_thread_helper_unix_t;

static void* _ev_thread_proxy_unix(void* arg)
{
    ev_thread_helper_unix_t* p_helper = arg;
    ev_thread_helper_unix_t helper = *p_helper;

    if (sem_post(&p_helper->sem) != 0)
    {
        EV_ABORT();
    }

    helper.cb(helper.arg);
    return NULL;
}

int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg)
{
    int err = 0;

    pthread_attr_t* attr = NULL;
    pthread_attr_t attr_storage;

    if (opt != NULL && opt->flags.have_stack_size)
    {
        if (pthread_attr_init(&attr_storage) != 0)
        {
            err = errno;
            return ev__translate_sys_error(err);
        }

        if (pthread_attr_setstacksize(&attr_storage, opt->stack_size) != 0)
        {
            err = errno;
            goto err_fin;
        }

        attr = &attr_storage;
    }

    ev_thread_helper_unix_t helper;
    helper.cb = cb;
    helper.arg = arg;
    if (sem_init(&helper.sem, 0, 0) != 0)
    {
        err = errno;
        goto err_fin;
    }

    if ((err = pthread_create(thr, attr, _ev_thread_proxy_unix, &helper)) != 0)
    {
        goto release_sem;
    }

    do 
    {
        err = sem_wait(&helper.sem);
    } while (err == -1 && errno == EINTR);

    err = err != 0 ? errno : 0;

release_sem:
    sem_destroy(&helper.sem);
err_fin:
    if (attr != NULL)
    {
        pthread_attr_destroy(attr);
    }
    return ev__translate_sys_error(err);
}

int ev_thread_exit(ev_os_thread_t* thr, unsigned long timeout)
{
    int ret = EBUSY;
    if (timeout == EV_INFINITE_TIMEOUT)
    {
        int err = pthread_join(*thr, NULL);
        return ev__translate_sys_error(err);
    }

    const uint64_t t_start = ev_hrtime() / 1000;
    const uint64_t t_end = t_start + timeout;

    uint64_t t_now;
    while ((t_now = ev_hrtime() / 1000) < t_end)
    {
        if ((ret = pthread_tryjoin_np(*thr, NULL)) == 0)
        {
            break;
        }

        uint64_t t_diff = t_end - t_now;
        unsigned sleep_time = t_diff < 10 ? t_diff : 10;
        ev_thread_sleep(sleep_time);
    }

    /* try last time */
    if (ret == EBUSY)
    {
        ret = pthread_tryjoin_np(*thr, NULL);
    }

    return ret == EBUSY ? EV_ETIMEDOUT : ev__translate_sys_error(ret);
}

ev_os_thread_t ev_thread_self(void)
{
    return pthread_self();
}

ev_os_tid_t ev_thread_id(void)
{
    return syscall(__NR_gettid);
}

int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2)
{
    return pthread_equal(*t1, *t2);
}

void ev_thread_sleep(uint32_t timeout)
{
    struct timespec t_req, t_rem;
    t_req.tv_sec = timeout / 1000;
    t_req.tv_nsec = (timeout - t_req.tv_sec * 1000) * 1000 * 1000;

    int ret;
    while((ret = nanosleep(&t_req, &t_rem)) != 0)
    {
        ret = errno;
        if (ret != EINTR)
        {
            EV_ABORT();
        }
        t_req = t_rem;
    }
}

int ev_tls_init(ev_tls_t* tls)
{
    int ret = pthread_key_create(&tls->tls, NULL);
    if (ret == 0)
    {
        return 0;
    }
    return ev__translate_sys_error(ret);
}

void ev_tls_exit(ev_tls_t* tls)
{
    int ret = pthread_key_delete(tls->tls);
    if (ret != 0)
    {
        EV_ABORT();
    }
}

void ev_tls_set(ev_tls_t* tls, void* val)
{
    int ret = pthread_setspecific(tls->tls, val);
    if (ret != 0)
    {
        EV_ABORT();
    }
}

void* ev_tls_get(ev_tls_t* tls)
{
    return pthread_getspecific(tls->tls);
}

#line 1 "src/unix/threadpool_unix.c"
/* AMALGAMATE: #include "threadpool.h" */
/* AMALGAMATE: #include "loop_unix.h" */
/* AMALGAMATE: #include "async_unix.h" */
/* AMALGAMATE: #include "io_unix.h" */

static void _on_work_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;

    ev_loop_t* loop = EV_CONTAINER_OF(io, ev_loop_t, backend.threadpool.io);
    ev__async_pend(loop->backend.threadpool.evtfd[0]);

    ev__threadpool_process(loop);
}

EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop)
{
    ev__async_post(loop->backend.threadpool.evtfd[1]);
}

EV_LOCAL void ev__init_work(ev_loop_t* loop)
{
    ev__asyc_eventfd(loop->backend.threadpool.evtfd);

    ev__nonblock_io_init(&loop->backend.threadpool.io, loop->backend.threadpool.evtfd[0], _on_work_unix, NULL);
    ev__nonblock_io_add(loop, &loop->backend.threadpool.io, EV_IO_IN);
}

EV_LOCAL void ev__exit_work(ev_loop_t* loop)
{
    ev__async_eventfd_close(loop->backend.threadpool.evtfd[0]);
    loop->backend.threadpool.evtfd[0] = -1;

    ev__async_eventfd_close(loop->backend.threadpool.evtfd[1]);
    loop->backend.threadpool.evtfd[1] = -1;
}

#line 1 "src/unix/udp_unix.c"
/* AMALGAMATE: #include "ev.h" */
/* AMALGAMATE: #include "io_unix.h" */
/* AMALGAMATE: #include "misc_unix.h" */
/* AMALGAMATE: #include "udp.h" */
/* AMALGAMATE: #include "handle.h" */
#include <unistd.h>
#include <string.h>

static void _ev_udp_close_unix(ev_udp_t* udp)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        ev__nonblock_io_del(udp->base.loop, &udp->backend.io, EPOLLIN | EPOLLOUT);
        close(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_udp_w_user_callback_unix(ev_udp_t* udp, ev_udp_write_t* req, ssize_t size)
{
    ev__handle_event_dec(&udp->base);
    ev__write_exit(&req->base);
    ev__handle_exit(&req->handle, NULL);
    req->usr_cb(req, size);
}

static void _ev_udp_r_user_callback_unix(ev_udp_t* udp, ev_udp_read_t* req,
    const struct sockaddr* addr, ssize_t size)
{
    ev__handle_event_dec(&udp->base);
    ev__read_exit(&req->base);
    ev__handle_exit(&req->handle, NULL);
    req->usr_cb(req, addr, size);
}

static void _ev_udp_cancel_all_w_unix(ev_udp_t* udp, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&udp->send_list)) != NULL)
    {
        ev_udp_write_t* req = EV_CONTAINER_OF(it, ev_udp_write_t, base.node);
        _ev_udp_w_user_callback_unix(udp, req, err);
    }
}

static void _ev_udp_cancel_all_r_unix(ev_udp_t* udp, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&udp->recv_list)) != NULL)
    {
        ev_udp_read_t* req = EV_CONTAINER_OF(it, ev_udp_read_t, base.node);
        _ev_udp_r_user_callback_unix(udp, req, NULL, err);
    }
}

static void _ev_udp_abort_unix(ev_udp_t* udp, int err)
{
    _ev_udp_close_unix(udp);
    _ev_udp_cancel_all_w_unix(udp, err);
    _ev_udp_cancel_all_r_unix(udp, err);
}

static void _ev_udp_on_close_unix(ev_handle_t* handle)
{
    ev_udp_t* udp = EV_CONTAINER_OF(handle, ev_udp_t, base);

    _ev_udp_abort_unix(udp, EV_ECANCELED);

    if (udp->close_cb != NULL)
    {
        udp->close_cb(udp);
    }
}

/**
 * @return bool
 */
static int _ev_udp_is_connected_unix(ev_os_socket_t sock)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (getpeername(sock, (struct sockaddr*)&addr, &addrlen) != 0)
    {
        return 0;
    }
    return addrlen > 0;
}

static int _ev_udp_set_flags_unix(ev_udp_t* udp, unsigned flags)
{
    int err;
    if (flags & EV_UDP_IPV6_ONLY)
    {
        int yes = 1;
        if (setsockopt(udp->sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1)
        {
            err = errno;
            return ev__translate_sys_error(err);
        }
    }

    if (flags & EV_UDP_REUSEADDR)
    {
        if ((err = ev__reuse_unix(udp->sock)) != 0)
        {
            return err;
        }
    }

    return 0;
}

static int _ev_udp_disconnect_unix(ev_udp_t* udp)
{
    struct sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    addr.sa_family = AF_UNSPEC;

    int r;
    do
    {
        r = connect(udp->sock, &addr, sizeof(addr));
    } while (r == -1 && errno == EINTR);

    if (r == -1 && errno != EAFNOSUPPORT)
    {
        r = errno;
        return ev__translate_sys_error(r);
    }

    udp->base.data.flags &= ~EV_HANDLE_UDP_CONNECTED;
    return 0;
}

static ssize_t _ev_udp_sendmsg_unix(int fd, struct iovec* iov, int iovcnt, void* arg)
{
    struct msghdr* p_hdr = arg;
    p_hdr->msg_iov = iov;
    p_hdr->msg_iovlen = iovcnt;

    ssize_t write_size;
    do 
    {
        write_size = sendmsg(fd, p_hdr, 0);
    } while (write_size < 0 && errno == EINTR);

    if (write_size < 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    return write_size;
}

static int _ev_udp_do_sendmsg_unix(ev_udp_t* udp, ev_udp_write_t* req)
{
    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));

    if (req->backend.peer_addr.ss_family == AF_UNSPEC)
    {
        hdr.msg_name = NULL;
        hdr.msg_namelen = 0;
    }
    else
    {
        hdr.msg_name = &req->backend.peer_addr;
        hdr.msg_namelen = ev__get_addr_len((struct sockaddr*)&req->backend.peer_addr);
    }

    return ev__send_unix(udp->sock, &req->base, _ev_udp_sendmsg_unix, &hdr);
}

static int _ev_udp_on_io_write_unix(ev_udp_t* udp)
{
    int ret = 0;
    ev_list_node_t* it;
    while ((it = ev_list_begin(&udp->send_list)) != NULL)
    {
        ev_udp_write_t* req = EV_CONTAINER_OF(it, ev_udp_write_t, base.node);

        if ((ret = _ev_udp_do_sendmsg_unix(udp, req)) != 0)
        {
            break;
        }

        ev_list_erase(&udp->send_list, it);
        _ev_udp_w_user_callback_unix(udp, req, req->base.size);
    }

    if (ret == EV_EAGAIN)
    {
        ret = 0;
    }
    return ret;
}

static int _ev_udp_do_recvmsg_unix(ev_udp_t* udp, ev_udp_read_t* req)
{
    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    memset(&req->addr, 0, sizeof(req->addr));

    hdr.msg_name = &req->addr;
    hdr.msg_namelen = sizeof(req->addr);
    hdr.msg_iov = (struct iovec*)req->base.data.bufs;
    hdr.msg_iovlen = req->base.data.nbuf;

    ssize_t read_size;
    do
    {
        read_size = recvmsg(udp->sock, &hdr, 0);
    } while (read_size < 0 && errno == EINTR);

    if (read_size < 0)
    {
        int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK)
        {
            return EV_EAGAIN;
        }
        return ev__translate_sys_error(err);
    }

    req->base.data.size += read_size;
    return 0;
}

static int _ev_udp_on_io_read_unix(ev_udp_t* udp)
{
    int ret = 0;
    ev_list_node_t* it;
    while ((it = ev_list_begin(&udp->recv_list)) != NULL)
    {
        ev_udp_read_t* req = EV_CONTAINER_OF(it, ev_udp_read_t, base.node);

        if ((ret = _ev_udp_do_recvmsg_unix(udp, req)) != 0)
        {
            break;
        }

        ev_list_erase(&udp->recv_list, it);
        _ev_udp_r_user_callback_unix(udp, req, (struct sockaddr*)&req->addr, req->base.data.size);
    }

    if (ret == EV_EAGAIN)
    {
        ret = 0;
    }
    return ret;
}

static void _ev_udp_on_io_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;
    int ret;
    ev_udp_t* udp = EV_CONTAINER_OF(io, ev_udp_t, backend.io);

    if (evts & EPOLLOUT)
    {
        if ((ret = _ev_udp_on_io_write_unix(udp)) != 0)
        {
            goto err;
        }

        if (ev_list_size(&udp->send_list) == 0)
        {
            ev__nonblock_io_del(udp->base.loop, &udp->backend.io, EPOLLOUT);
        }
    }

    if (evts & EPOLLIN)
    {
        if ((ret = _ev_udp_on_io_read_unix(udp)) != 0)
        {
            goto err;
        }

        if (ev_list_size(&udp->recv_list) == 0)
        {
            ev__nonblock_io_del(udp->base.loop, &udp->backend.io, EPOLLIN);
        }
    }

    return;

err:
    _ev_udp_abort_unix(udp, ret);
}

static int _ev_udp_maybe_deferred_socket_unix(ev_udp_t* udp, int domain)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return 0;
    }

    if ((udp->sock = socket(domain, SOCK_DGRAM, 0)) < 0)
    {
        int ret = errno;
        return ev__translate_sys_error(ret);
    }

    ev__nonblock_io_init(&udp->backend.io, udp->sock, _ev_udp_on_io_unix, NULL);

    return 0;
}

static int _ev_udp_do_connect_unix(ev_udp_t* udp, const struct sockaddr* addr)
{
    int ret;
    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_udp_maybe_deferred_socket_unix(udp, addr->sa_family)) != 0)
    {
        return ret;
    }

    do 
    {
        ret = connect(udp->sock, addr, addrlen);
    } while (ret == -1 && errno == EINTR);

    if (ret != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    udp->base.data.flags |= EV_HANDLE_UDP_CONNECTED;
    return 0;
}

static int _ev_udp_maybe_deferred_bind_unix(ev_udp_t* udp, int domain, int flags)
{
    int ret = _ev_udp_maybe_deferred_socket_unix(udp, domain);
    if (ret != 0)
    {
        return ret;
    }

    if (udp->base.data.flags & EV_HANDLE_UDP_BOUND)
    {
        return 0;
    }

    struct sockaddr_storage addr;
    struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
    struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;

    switch (domain)
    {
    case AF_INET:
    {
        memset(addr4, 0, sizeof(*addr4));
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return ev_udp_bind(udp, (struct sockaddr*)addr4, flags);
    }

    case AF_INET6:
    {
        memset(addr6, 0, sizeof(*addr6));
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        return ev_udp_bind(udp, (struct sockaddr*)addr6, flags);
    }

    default:
        break;
    }

    EV_ABORT();
}

static int _ev_udp_convert_interface_addr4_unix(struct ip_mreq* dst,
    const struct sockaddr_in* multicast_addr, const char* interface_addr)
{
    int ret;
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = inet_pton(AF_INET, interface_addr, &dst->imr_interface.s_addr)) != 1)
        {
            int ret = errno;
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        dst->imr_interface.s_addr = htonl(INADDR_ANY);
    }

    dst->imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;

    return 0;
}

static int _ev_udp_setmembership4_unix(ev_udp_t* udp, struct sockaddr_in* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;

    struct ip_mreq mreq;
    if ((ret = _ev_udp_convert_interface_addr4_unix(&mreq, multicast_addr, interface_addr)) != 0)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static void _ev_udp_convert_interface_addr6_unix(struct ipv6_mreq* dst,
    struct sockaddr_in6* multicast_addr, const char* interface_addr)
{
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != 0)
    {
        struct sockaddr_in6 addr6;
        ev_ipv6_addr(interface_addr, 0, &addr6);

        dst->ipv6mr_interface = addr6.sin6_scope_id;
    }
    else
    {
        dst->ipv6mr_interface = 0;
    }
    dst->ipv6mr_multiaddr = multicast_addr->sin6_addr;
}

static int _ev_udp_setmembership6_unix(ev_udp_t* udp, struct sockaddr_in6* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;
    struct ipv6_mreq mreq;
    _ev_udp_convert_interface_addr6_unix(&mreq, multicast_addr, interface_addr);

    int optname = membership == EV_UDP_ENTER_GROUP ? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static int _ev_udp_convert_source_interface_addr4_unix(struct ip_mreq_source* dst,
    const char* interface_addr, const struct sockaddr_in* multicast_addr, const struct sockaddr_in* source_addr)
{
    int ret;
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = inet_pton(AF_INET, interface_addr, &dst->imr_interface.s_addr)) != 1)
        {
            ret = errno;
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        dst->imr_interface.s_addr = htonl(INADDR_ANY);
    }

    dst->imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;
    dst->imr_sourceaddr.s_addr = source_addr->sin_addr.s_addr;

    return 0;
}

static int _ev_udp_set_source_membership4_unix(ev_udp_t* udp, const struct sockaddr_in* multicast_addr,
    const char* interface_addr, const struct sockaddr_in* source_addr, ev_udp_membership_t membership)
{
    int ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET, EV_UDP_REUSEADDR);
    if (ret != 0)
    {
        return ret;
    }

    struct ip_mreq_source mreq;
    if ((ret = _ev_udp_convert_source_interface_addr4_unix(&mreq, interface_addr,
        multicast_addr, source_addr)) != 0)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static int _ev_udp_convert_source_interface_addr6_unix(struct group_source_req* dst,
    const char* interface_addr, const struct sockaddr_in6* multicast_addr,
    const struct sockaddr_in6* source_addr)
{
    int ret;
    struct sockaddr_in6 addr_6;

    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = ev_ipv6_addr(interface_addr, 0, &addr_6)) != 0)
        {
            return ret;
        }
        dst->gsr_interface = addr_6.sin6_scope_id;
    }
    else
    {
        dst->gsr_interface = 0;
    }

    memcpy(&dst->gsr_group, multicast_addr, sizeof(*multicast_addr));
    memcpy(&dst->gsr_source, source_addr, sizeof(*source_addr));

    return 0;
}

static int _ev_udp_set_source_membership6_unix(ev_udp_t* udp, const struct sockaddr_in6* multicast_addr,
    const char* interface_addr, const struct sockaddr_in6* source_addr, ev_udp_membership_t membership)
{
    int ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET6, EV_UDP_REUSEADDR);
    if (ret != 0)
    {
        return ret;
    }

    struct group_source_req mreq;
    if ((ret = _ev_udp_convert_source_interface_addr6_unix(&mreq, interface_addr,
        multicast_addr, source_addr)) != 0)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? MCAST_JOIN_SOURCE_GROUP : MCAST_LEAVE_SOURCE_GROUP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

static int _ev_udp_set_ttl_unix(ev_udp_t* udp, int ttl, int option4, int option6)
{
    int level;
    int option;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        option = option6;
    }
    else
    {
        level = IPPROTO_IP;
        option = option4;
    }

    void* optval = &ttl;
    socklen_t optlen = sizeof(ttl);

    /**
     * On Solaris and derivatives such as SmartOS, the length of socket options
     * is sizeof(int) for IPV6_MULTICAST_LOOP and sizeof(char) for
     * IP_MULTICAST_LOOP, so hardcode the size of the option in the IPv6 case,
     * and use the general uv__setsockopt_maybe_char call otherwise.
     */
#if defined(__sun) || defined(_AIX) || defined(__OpenBSD__) || defined(__MVS__) || defined(__QNX__)
    char char_val = ttl;
    if (!(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
    {
        optval = &char_val;
        optlen = sizeof(char_val);
    }
#endif

    if (setsockopt(udp->sock, level, option, optval, optlen) != 0)
    {
        int ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

EV_LOCAL int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req)
{
    (void)req;
    if (ev_list_size(&udp->recv_list) == 1)
    {
        ev__nonblock_io_add(udp->base.loop, &udp->backend.io, EPOLLIN);
    }

    ev__handle_event_add(&udp->base);

    return 0;
}

EV_LOCAL int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req,
    const struct sockaddr* addr, socklen_t addrlen)
{
    int ret;

    if (addr == NULL)
    {
        req->backend.peer_addr.ss_family = AF_UNSPEC;
    }
    else
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, addr->sa_family, 0)) != 0)
        {
            return ret;
        }

        if (addrlen == (socklen_t)-1)
        {
            return EV_EINVAL;
        }

        memcpy(&req->backend.peer_addr, addr, addrlen);
    }

    if (ev_list_size(&udp->send_list) == 1)
    {
        ev__nonblock_io_add(udp->base.loop, &udp->backend.io, EPOLLOUT);
    }

    ev__handle_event_add(&udp->base);

    return 0;
}

int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain)
{
    int err;
    if (domain != AF_INET && domain != AF_INET6 && domain != AF_UNSPEC)
    {
        return EV_EINVAL;
    }

    udp->sock = EV_OS_SOCKET_INVALID;
    if (domain != AF_UNSPEC)
    {
        if ((err = _ev_udp_maybe_deferred_socket_unix(udp, domain)) != 0)
        {
            return err;
        }
    }

    ev__handle_init(loop, &udp->base, EV_ROLE_EV_UDP);
    ev_list_init(&udp->send_list);
    ev_list_init(&udp->recv_list);

    return 0;
}

int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock)
{
    int err;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_EBUSY;
    }

    if ((err = ev__nonblock(sock, 1)) != 0)
    {
        return err;
    }

    if (_ev_udp_is_connected_unix(sock))
    {
        udp->base.data.flags |= EV_HANDLE_UDP_CONNECTED;
    }

    return 0;
}

void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb)
{
    _ev_udp_close_unix(udp);
    udp->close_cb = close_cb;
    ev__handle_exit(&udp->base, _ev_udp_on_close_unix);
}

int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags)
{
    int ret;
    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_udp_maybe_deferred_socket_unix(udp, addr->sa_family)) != 0)
    {
        return ret;
    }

    if ((ret = _ev_udp_set_flags_unix(udp, flags)) != 0)
    {
        return ret;
    }

    if (bind(udp->sock, addr, addrlen) < 0)
    {
        ret = errno;
        if (ret == EAFNOSUPPORT)
        {
            return EV_EINVAL;
        }
        return ev__translate_sys_error(ret);
    }

    if (addr->sa_family == AF_INET6)
    {
        udp->base.data.flags |= EV_HANDLE_UDP_IPV6;
    }

    udp->base.data.flags |= EV_HANDLE_UDP_BOUND;
    return 0;
}

int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr)
{
    if (addr == NULL)
    {
        if (!(udp->base.data.flags & EV_HANDLE_UDP_CONNECTED))
        {
            return EV_ENOTCONN;
        }
        return _ev_udp_disconnect_unix(udp);
    }

    return _ev_udp_do_connect_unix(udp, addr);
}

int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    socklen_t wrap_len = *len;
    if (getsockname(udp->sock, name, &wrap_len) != 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return 0;
}

int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    socklen_t wrap_len = *len;
    if (getpeername(udp->sock, name, &wrap_len) != 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return 0;
}

int ev_udp_set_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr;
    memset(addr_4, 0, sizeof(*addr_4));

    if (ev_ipv4_addr(multicast_addr, 0, addr_4) == 0)
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET, EV_UDP_REUSEADDR)) != 0)
        {
            return ret;
        }

        return _ev_udp_setmembership4_unix(udp, addr_4, interface_addr, membership);
    }

    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr;
    memset(addr_6, 0, sizeof(*addr_6));

    if (ev_ipv6_addr(multicast_addr, 0, addr_6) == 0)
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET6, EV_UDP_REUSEADDR)) != 0)
        {
            return ret;
        }

        return _ev_udp_setmembership6_unix(udp, addr_6, interface_addr, membership);
    }

    return EV_EINVAL;
}

int ev_udp_set_source_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, const char* source_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage mcast_addr;
    struct sockaddr_storage src_addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    struct sockaddr_in* mcast_addr_4 = (struct sockaddr_in*)&mcast_addr;
    if ((ret = ev_ipv4_addr(multicast_addr, 0, mcast_addr_4)) == 0)
    {
        struct sockaddr_in* src_addr_4 = (struct sockaddr_in*)&src_addr;
        if ((ret = ev_ipv4_addr(source_addr, 0, src_addr_4)) != 0)
        {
            return ret;
        }

        return _ev_udp_set_source_membership4_unix(udp, mcast_addr_4, interface_addr, src_addr_4, membership);
    }

    struct sockaddr_in6* mcast_addr_6 = (struct sockaddr_in6*)&mcast_addr;
    if ((ret = ev_ipv6_addr(multicast_addr, 0, mcast_addr_6)) == 0)
    {
        struct sockaddr_in6* src_addr_6 = (struct sockaddr_in6*)&src_addr;
        if ((ret = ev_ipv6_addr(source_addr, 0, src_addr_6)) != 0)
        {
            return ret;
        }

        return _ev_udp_set_source_membership6_unix(udp, mcast_addr_6, interface_addr, src_addr_6, membership);
    }

    return ret;
}

int ev_udp_set_multicast_loop(ev_udp_t* udp, int on)
{
    return _ev_udp_set_ttl_unix(udp, on, IP_MULTICAST_LOOP, IPV6_MULTICAST_LOOP);
}

int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl)
{
    return _ev_udp_set_ttl_unix(udp, ttl, IP_MULTICAST_TTL, IPV6_MULTICAST_HOPS);
}

int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr)
{
    int ret;
    struct sockaddr_storage addr_st;
    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr_st;
    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr_st;

    int is_ipv6 = udp->base.data.flags & EV_HANDLE_UDP_IPV6;
    if ((ret = ev__udp_interface_addr_to_sockaddr(&addr_st, interface_addr, is_ipv6)) != 0)
    {
        return ret;
    }

    int level;
    int optname;
    void* optval;
    socklen_t optlen;

    if (addr_st.ss_family == AF_INET)
    {
        level = IPPROTO_IP;
        optname = IP_MULTICAST_IF;
        optval = &addr_4->sin_addr;
        optlen = sizeof(addr_4->sin_addr);
    }
    else if(addr_st.ss_family == AF_INET6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_IF;
        optval = &addr_6->sin6_scope_id;
        optlen = sizeof(addr_6->sin6_scope_id);
    }
    else
    {
        EV_ABORT();
    }

    if (setsockopt(udp->sock, level, optname, optval, optlen) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return 0;
}

int ev_udp_set_broadcast(ev_udp_t* udp, int on)
{
    int err;
    if (setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) != 0)
    {
        err = errno;
        return ev__translate_sys_error(err);
    }

    return 0;
}

int ev_udp_set_ttl(ev_udp_t* udp, int ttl)
{
    if (ttl < 1 || ttl > 255)
    {
        return EV_EINVAL;
    }

#if defined(__MVS__)
    /* zOS does not support setting ttl for IPv4 */
    if (!(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
    {
        return EV_ENOTSUP;
    }
#endif

    return _ev_udp_set_ttl_unix(udp, ttl, IP_TTL, IPV6_UNICAST_HOPS);
}

#endif              /* AMALGAMATE: ev.c (3/3) */
#if defined(EV_HAVE_LUA_BINDING) /* FEATURE: EV_HAVE_LUA_BINDING (1/2) */
#include "lua.h"
#include "lauxlib.h"
#line 1 "src/lua/ev.lua.internal.h"
#ifndef __EV_LUA_INTERNAL_H__
#define __EV_LUA_INTERNAL_H__

/* AMALGAMATE: #include "ev.lua.h" */
/* AMALGAMATE: #include "ev.h" */

#define lev_error(L, loop, errcode, fmt, ...)   \
    lev_error_ex(__FILE__, __LINE__, L, loop, errcode, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

/* AMALGAMATE: #include "lua.h" */
/* AMALGAMATE: #include "lauxlib.h" */

/**
 * @brief Make a loop on top of lua stack and return the handle.
 * @param[in] L     Lua Stack.
 * @return          Event loop.
 */
ev_loop_t* lev_make_loop(lua_State* L);

/**
 * @brief Wakeup event loop.
 * @param[in] loop  Event loop.
 */
void lev_wakeup(ev_loop_t* loop);

/**
 * @brief Set coroutine state.
 * @param[in] L     Lua stack.
 * @param[in] loop  Event loop.
 * @param[in] busy  Is busy.
 * @return          This function does not failure.
 */
int lev_set_state(lua_State* L, ev_loop_t* loop, int busy);

/**
 * @brief Check if \p L is yieldable in event loop.
 * @param[in] L     Lua coroutine to check.
 * @param[in] loop  Event loop.
 * @return          Boolean.
 */
int lev_is_yieldable(lua_State* L, ev_loop_t* loop);

/**
 * @brief Convert value at \p idx into event loop.
 * @param[in] L     Lua Stack.
 * @param[in] idx   Stack index.
 * @return          Event loop.
 */
ev_loop_t* lev_to_loop(lua_State* L, int idx);

/**
 * @brief Generate a error.
 * @param[in] L         Lua Stack.
 * @param[in] loop      Event loop.
 * @param[in] errcode   Error code.
 * @param[in] fmt       User format.
 * @param[in] ...       Format list.
 * @return This function does not return.
 */
int lev_error_ex(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/channel.lua.h"
#ifndef __EV_LUA_CHANNEL_H__
#define __EV_LUA_CHANNEL_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

int lev_channel(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/fs.lua.h"
#ifndef __EV_LUA_FS_H__
#define __EV_LUA_FS_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

#define LEV_FILE_FLAG_MAP(xx)   \
    xx(EV_FS_O_APPEND)          \
    xx(EV_FS_O_CREAT)           \
    xx(EV_FS_O_DSYNC)           \
    xx(EV_FS_O_EXCL)            \
    xx(EV_FS_O_SYNC)            \
    xx(EV_FS_O_TRUNC)           \
    xx(EV_FS_O_RDONLY)          \
    xx(EV_FS_O_WRONLY)          \
    xx(EV_FS_O_RDWR)            \
    \
    xx(EV_FS_S_IRUSR)           \
    xx(EV_FS_S_IWUSR)           \
    xx(EV_FS_S_IXUSR)           \
    xx(EV_FS_S_IRWXU)           \
    \
    xx(EV_FS_SEEK_BEG)          \
    xx(EV_FS_SEEK_CUR)          \
    xx(EV_FS_SEEK_END)

/**
 * @brief Open a file.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_file(lua_State* L);

/**
 * @brief Open a file or directory.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_remove(lua_State* L);

/**
 * @brief Read dir.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_readdir(lua_State* L);

/**
 * @brief Try to convert file handle.
 * @param[in] L     Lua stack.
 * @param[in] idx   Stack index.
 * @return          File handle.
 */
ev_file_t* lev_try_to_file(lua_State* L, int idx);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/misc.lua.h"
#ifndef __EV_LUA_MISC_H__
#define __EV_LUA_MISC_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

int lev_hrtime(lua_State* L);
int lev_strerror(lua_State* L);

/**
 * @brief Convert value at \p idx into sockaddr_storage object.
 * @param[in] L     Lua stack.
 * @param[in] idx   Stack index.
 * @return          Address of sockaddr_storage object.
 */
struct sockaddr_storage* lev_to_sockaddr_storage(lua_State* L, int idx);

/**
 * @brief Create a sockaddr_storage object on top of \p L.
 * @param[in] L     Lua stack.
 * @return          Address of sockaddr_storage object.
 */
struct sockaddr_storage* lev_push_sockaddr_storage(lua_State* L);

/**
 * @brief Convert ip and port into network address.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_ip_addr(lua_State* L);

/**
 * @brief Convert network address into ip and port.
 * @param[in] L     Lua stack.
 * @return          Always 2.
 */
int lev_ip_name(lua_State* L);

/**
 * @brief Pack arguments.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_arg_pack(lua_State* L);

/**
 * @brief Unpack arguments.
 * @param[in] L     Lua stack.
 * @return          The number of unpacked arguments.
 */
int lev_arg_unpack(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/pipe.lua.h"
#ifndef __EV_LUA_PIPE_H__
#define __EV_LUA_PIPE_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

int lev_pipe(lua_State* L);

ev_pipe_t* lev_try_to_pipe(lua_State* L, int idx);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/process.lua.h"
#ifndef __EV_LUA_PROCESS_H__
#define __EV_LUA_PROCESS_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

int lev_process(lua_State* L);

int lev_getcwd(lua_State* L);

int lev_exepath(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/promise.lua.h"
#ifndef __EV_LUA_PROMISE_H__
#define __EV_LUA_PROMISE_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lev_promise_s lev_promise_t;

/**
 * @brief Get value callback.
 * @param[in] L         Lua stack.
 * @param[in] promise   Promise handle.
 * @param[in] narg      The number of values on stack.
 * @param[in] data      User defined argument.
 * @return              The number of values on stack.
 */
typedef int (*lev_promise_cb)(lua_State* L, lev_promise_t* promise, int narg, void* data);

/**
 * @brief Create a new promise and push on top of Lua stack.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_promise(lua_State* L);

/**
 * @brief Create a new promise and push on top of Lua stack, and return it's handle.
 * @param[in] L     Lua stack.
 * @param[in] loop  Event loop.
 * @return          Promise handle.
 */
lev_promise_t* lev_promise_make(lua_State* L, ev_loop_t* loop);

/**
 * @brief Set value for promise.
 * @param[in] L         Lua stack.
 * @param[in] promise   The promise handle.
 * @param[in] narg      The number of arguments. Arguments should on top of Lua
 *   stack in order.
 * @return              #ev_errno_t
 */
int lev_promise_set_value(lua_State* L, lev_promise_t* promise, int narg);

/**
 * @brief Get value for promise.
 * @warning This function should be called only in a tail call.
 * @param[in] L         Lua stack.
 * @param[in] promise   Promise handle.
 * @param[in] cb        Value callback.
 * @param[in] data      Private data.
 * @return              The same value from \p cb.
 */
int lev_promise_get_value(lua_State* L, lev_promise_t* promise, lev_promise_cb cb, void* data);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/lua/tcp.lua.h"
#ifndef __EV_LUA_TCP_H__
#define __EV_LUA_TCP_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new tcp socket.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_tcp(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/timer.lua.h"
#ifndef __EV_LUA_TIMER_H__
#define __EV_LUA_TIMER_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

int lev_sleep(lua_State* L);

#ifdef __cplusplus
}
#endif
#endif

#line 1 "src/lua/udp.lua.h"
#ifndef __EV_LUA_UDP_H__
#define __EV_LUA_UDP_H__

/* AMALGAMATE: #include "ev.lua.internal.h" */

#ifdef __cplusplus
extern "C" {
#endif

#define LEV_UDP_FLAG_MAP(xx)    \
    xx(AF_INET)                 \
    xx(AF_INET6)                \
    xx(AF_UNSPEC)               \
    \
    xx(EV_UDP_IPV6_ONLY)        \
    xx(EV_UDP_REUSEADDR)        \
    \
    xx(EV_UDP_LEAVE_GROUP)      \
    xx(EV_UDP_ENTER_GROUP)

int lev_udp(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif

#line 1 "src/lua/ev.lua.c"
/* AMALGAMATE: #include "channel.lua.h" */
/* AMALGAMATE: #include "fs.lua.h" */
/* AMALGAMATE: #include "misc.lua.h" */
/* AMALGAMATE: #include "pipe.lua.h" */
/* AMALGAMATE: #include "process.lua.h" */
/* AMALGAMATE: #include "promise.lua.h" */
/* AMALGAMATE: #include "tcp.lua.h" */
/* AMALGAMATE: #include "timer.lua.h" */
/* AMALGAMATE: #include "udp.lua.h" */
#include <assert.h>
#include <string.h>

/**
 * @brief Map of ev public interface.
 */
#define LEV_API_MAP(xx)                 \
    xx("exepath",       lev_exepath )   \
    xx("getcwd",        lev_getcwd)     \
    xx("hrtime",        lev_hrtime)     \
    xx("ip_addr",       lev_ip_addr)    \
    xx("ip_name",       lev_ip_name)    \
    xx("strerror",      lev_strerror)   \
    xx("arg_pack",      lev_arg_pack)   \
    xx("arg_unpack",    lev_arg_unpack)

/**
 * @brief Map of loop public interface.
 */
#define LEV_LOOP_API_MAP(xx)            \
    xx("channel",       lev_channel)    \
    xx("fs_file",       lev_fs_file)    \
    xx("fs_readdir",    lev_fs_readdir) \
    xx("fs_remove",     lev_fs_remove)  \
    xx("pipe",          lev_pipe)       \
    xx("process",       lev_process)    \
    xx("promise",       lev_promise)    \
    xx("sleep",         lev_sleep)      \
    xx("tcp",           lev_tcp)        \
    xx("udp",           lev_udp)

/**
 * @brief Map of public flags.
 */
#define LEV_OPT_MAP(xx)                 \
    LEV_FILE_FLAG_MAP(xx)               \
    LEV_UDP_FLAG_MAP(xx)

/**
 * @brief Loop type name in Lua.
 */
#define LEV_LOOP_NAME                   "__ev_loop"

typedef struct lev_loop_s
{
    ev_loop_t       loop;
    ev_async_t      async;

    ev_map_t        record;
    ev_list_t       wait_queue;
    ev_list_t       busy_queue;
} lev_loop_t;

typedef struct lev_coroutine
{
    ev_map_node_t   r_node;
    ev_list_node_t  q_node;     /**< Node for #lev_loop_t::busy_queue or #lev_loop_t::wait_queue. */

    lev_loop_t*     belong;     /**< Belong loop. */
    lua_State*      thread;     /**< Coroutine handle. */
    int             ref_thread; /**< Coroutine reference. */
    int             nresult;    /**< The number of parameter / results. */

    int             ref_self;   /**< Reference to self. */
    int             is_busy;    /**< If in busy_queue. */
} lev_coroutine_t;

static int _lev_on_cmp_co(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    lev_coroutine_t* co1 = EV_CONTAINER_OF(key1, lev_coroutine_t, r_node);
    lev_coroutine_t* co2 = EV_CONTAINER_OF(key2, lev_coroutine_t, r_node);
    if (co1->thread == co2->thread)
    {
        return 0;
    }
    return co1->thread < co2->thread ? -1 : 1;
}

static void _lev_co_unlink(lev_coroutine_t* co)
{
    if (co->belong == NULL)
    {
        return;
    }

    if (co->is_busy)
    {
        ev_list_erase(&co->belong->busy_queue, &co->q_node);
    }
    else
    {
        ev_list_erase(&co->belong->wait_queue, &co->q_node);
    }
    ev_map_erase(&co->belong->record, &co->r_node);

    co->belong = NULL;
}

static void _lev_release_co(lua_State* L, lev_coroutine_t* co)
{
    /* Remove from scheduler. */
    _lev_co_unlink(co);

    /* No let GC handle left things. */
    luaL_unref(L, LUA_REGISTRYINDEX, co->ref_self);
    co->ref_self = LUA_NOREF;
}

static int _lev_loop_gc(lua_State* L)
{
    int ret;
    ev_list_node_t* it;
    lev_loop_t* loop = lua_touserdata(L, 1);

    /* Remove reference to all coroutine. */
    while ((it = ev_list_begin(&loop->busy_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);
        _lev_release_co(L, co);
    }
    while ((it = ev_list_begin(&loop->wait_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);
        _lev_release_co(L, co);
    }

    /* Exit async */
    ev_async_exit(&loop->async, NULL);
    ev_loop_run(&loop->loop, EV_LOOP_MODE_NOWAIT);

    /* Exit loop. */
    if ((ret = ev_loop_exit(&loop->loop)) != 0)
    {
        return lev_error(L, &loop->loop, ret, NULL);
    }

    return 0;
}

static int _lev_co_gc(lua_State* L)
{
    lev_coroutine_t* co = lua_touserdata(L, 1);

    _lev_co_unlink(co);

    if (co->ref_self != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, co->ref_self);
        co->ref_self = LUA_NOREF;
    }

    if (co->ref_thread != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, co->ref_thread);
        co->ref_thread = LUA_NOREF;
    }
    co->thread = NULL;

    return 0;
}

static void _lev_copy_stack(lua_State* L, lev_coroutine_t* co, int beg, int end)
{
    int i;
    for (i = beg; i <= end; i++)
    {
        lua_pushvalue(L, i);
    }
    lua_xmove(L, co->thread, end - beg + 1);

    co->nresult = end - beg;
}

static void _lev_co_set_state(lev_coroutine_t* co, int is_busy)
{
    if (co->is_busy == is_busy)
    {
        return;
    }

    if (co->is_busy)
    {
        ev_list_erase(&co->belong->busy_queue, &co->q_node);
        ev_list_push_back(&co->belong->wait_queue, &co->q_node);
    }
    else
    {
        ev_list_erase(&co->belong->wait_queue, &co->q_node);
        ev_list_push_back(&co->belong->busy_queue, &co->q_node);
    }
    co->is_busy = is_busy;
}

static int _lev_loop_onpass(lua_State* L, lev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_begin(&loop->busy_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);

        int ret = lua_resume(co->thread, L, co->nresult, &co->nresult);

        /* Coroutine yield */
        if (ret == LUA_YIELD)
        {
            /* Anything received treat as busy coroutine */
            if (co->nresult != 0)
            {
                lua_pop(co->thread, co->nresult);
                co->nresult = 0;
            }

            continue;
        }

        /* Error affect main thread */
        if (ret != LUA_OK)
        {
            lua_xmove(co->thread, L, 1);
            return lua_error(L);
        }

        /* Finish execution. */
        _lev_release_co(L, co);
    }

    return 0;
}

static int _lev_loop_run_once(lua_State* L, lev_loop_t* loop, int mode)
{
    _lev_loop_onpass(L, loop);

    if (ev_list_size(&loop->wait_queue) != 0)
    {
        ev_loop_run(&loop->loop, mode);
    }

    _lev_loop_onpass(L, loop);

    return 0;
}

static int _lev_loop_run_default(lua_State* L, lev_loop_t* loop)
{
    while (1)
    {
        _lev_loop_run_once(L, loop, EV_LOOP_MODE_ONCE);

        if (ev_list_size(&loop->wait_queue) == 0 && ev_list_size(&loop->busy_queue) == 0)
        {
            break;
        }
    }

    return 0;
}

static void _lev_set_options(lua_State* L)
{
#define IMPORT_OPTIONS(x)   \
    lua_pushinteger(L, x);\
    lua_setfield(L, -2, #x);

    LEV_OPT_MAP(IMPORT_OPTIONS);

#undef IMPORT_OPTIONS

    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");
}

static char* _lev_strerr(const char* fmt, va_list ap)
{
    int cnt;
    if (fmt == NULL)
    {
        return NULL;
    }

    {
        va_list ap_bak;
        va_copy(ap_bak, ap);
        cnt = vsnprintf(NULL, 0, fmt, ap_bak);
        va_end(ap_bak);
    }

    char* buf = ev_malloc(cnt + 1);
    snprintf(buf, cnt + 1, fmt, ap);
    return buf;
}

static void _lev_dump_error_as_json_str(luaL_Buffer* b, const char* umsg)
{
    size_t i;
    char c;
    char buf[8];

    if (umsg == NULL)
    {
        luaL_addstring(b, "null");
        return;
    }

    luaL_addchar(b, '\"');

    for (i = 0; (c = umsg[i]) != '\0'; i++)
    {
        if ((c > 31) && (c != '\"') && (c != '\\'))
        {
            luaL_addchar(b, c);
        }
        else
        {
            luaL_addchar(b, '\\');
            switch (c)
            {
            case '\\': luaL_addchar(b, '\\'); break;
            case '\"': luaL_addchar(b, '\"'); break;
            case '\b': luaL_addchar(b, 'b'); break;
            case '\f': luaL_addchar(b, 'f'); break;
            case '\n': luaL_addchar(b, 'n'); break;
            case '\r': luaL_addchar(b, 'r'); break;
            case '\t': luaL_addchar(b, 't'); break;
            default:
                snprintf(buf, sizeof(buf), "u%04x", (unsigned int)c);
                luaL_addstring(b, buf);
                break;
            }
        }
    }

    luaL_addchar(b, '\"');
}

static const char* _lev_push_error_object(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* msg)
{
    char buffer[4096];
    luaL_Buffer buf;
    luaL_buffinit(L, &buf);

    snprintf(buffer, sizeof(buffer),
        "{"
        "\"file\":\"%s\","
        "\"line\":%d,"
        "\"loop\":\"%p\","
        "\"errnum\":%d,"
        "\"errmsg\":\"%s\","
        "\"extmsg\":"
        , file, line, loop, errcode, ev_strerror(errcode));
    luaL_addstring(&buf, buffer);

    _lev_dump_error_as_json_str(&buf, msg);
    luaL_addstring(&buf, "}");

    luaL_pushresult(&buf);

    return lua_tostring(L, -1);
}

static lev_coroutine_t* _lev_find_coroutine(lua_State* L, ev_loop_t* loop)
{
    lev_loop_t* impl = EV_CONTAINER_OF(loop, lev_loop_t, loop);

    lev_coroutine_t tmp; tmp.thread = L;
    ev_map_node_t* it = ev_map_find(&impl->record, &tmp.r_node);
    if (it == NULL)
    {
        return NULL;
    }

    lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, r_node);
    return co;
}

/**
 * @brief Same as #lev_make_loop() but always return 1.
 * @param[in] L     Lua Stack.
 * @return          Always 1.
 */
static int _lev_mkloop(lua_State* L)
{
    lev_make_loop(L);
    return 1;
}

int luaopen_ev(lua_State* L)
{
#if LUA_VERSION_NUM >= 502
    luaL_checkversion(L);
#endif

    static const luaL_Reg s_ev_api_map[] = {
        { "loop",   _lev_mkloop },
#define EXPAND_EV_LUA_API_MAP(n, f) { n, f },
        LEV_API_MAP(EXPAND_EV_LUA_API_MAP)
#undef EXPAND_EV_LUA_API_MAP
        { NULL,     NULL },
    };
    luaL_newlib(L, s_ev_api_map);

    _lev_set_options(L);

    return 1;
}

static int _lev_loop_co(lua_State* L)
{
    lev_loop_t* loop = luaL_checkudata(L, 1, LEV_LOOP_NAME);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    int sp = lua_gettop(L);

    lev_coroutine_t* co = lua_newuserdata(L, sizeof(lev_coroutine_t));
    memset(co, 0, sizeof(*co));

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_co_gc },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, "__ev_coroutine") != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    {
        co->thread = lua_newthread(L);
        co->ref_thread = luaL_ref(L, LUA_REGISTRYINDEX);
        co->belong = loop;
    }

    _lev_copy_stack(L, co, 2, sp);

    co->ref_self = luaL_ref(L, LUA_REGISTRYINDEX);

    ev_list_push_back(&loop->busy_queue, &co->q_node);
    ev_map_insert(&loop->record, &co->r_node);
    co->is_busy = 1;

    return 0;
}

static int _lev_loop_run(lua_State* L)
{
    lev_loop_t* loop = luaL_checkudata(L, 1, LEV_LOOP_NAME);

    /* Get loop mode. */
    int mode = EV_LOOP_MODE_DEFAULT;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        mode = (int)lua_tonumber(L, 2);
    }

    switch (mode)
    {
    case EV_LOOP_MODE_ONCE:
        return _lev_loop_run_once(L, loop, EV_LOOP_MODE_ONCE);

    case EV_LOOP_MODE_NOWAIT:
        return _lev_loop_run_once(L, loop, EV_LOOP_MODE_NOWAIT);

    default:
        break;
    }

    return _lev_loop_run_default(L, loop);
}

static int _lev_error(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    int errcode = (int)luaL_checkinteger(L, 2);
    const char* msg = NULL;
    if (lua_type(L, 3) == LUA_TSTRING)
    {
        msg = lua_tostring(L, 3);
    }

    _lev_push_error_object(NULL, 0, L, loop, errcode, msg);
    return lua_error(L);
}

static void _lev_loop_on_weakup(ev_async_t* async)
{
    (void)async;
}

ev_loop_t* lev_make_loop(lua_State* L)
{
    lev_loop_t* loop = lua_newuserdata(L, sizeof(lev_loop_t));

    memset(loop, 0, sizeof(*loop));
    ev_loop_init(&loop->loop);
    ev_map_init(&loop->record, _lev_on_cmp_co, NULL);

    ev_async_init(&loop->loop, &loop->async, _lev_loop_on_weakup);

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_loop_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        /* Control */
        { "co",         _lev_loop_co },
        { "run",        _lev_loop_run },
        { "error",      _lev_error },
        /* Component */
#define EXPAND_EV_LOOP_API_MAP(n, f) { n, f },
        LEV_LOOP_API_MAP(EXPAND_EV_LOOP_API_MAP)
#undef EXPAND_EV_LOOP_API_MAP
        /* End */
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_LOOP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return &loop->loop;
}

ev_loop_t* lev_to_loop(struct lua_State* L, int idx)
{
    lev_loop_t* loop = luaL_checkudata(L, idx, LEV_LOOP_NAME);
    return &loop->loop;
}

int lev_set_state(lua_State* L, ev_loop_t* loop, int busy)
{
    lev_coroutine_t* co = _lev_find_coroutine(L, loop);
    if (co == NULL)
    {
        return lev_error(L, loop, EV_ENOENT, "coroutine %p not found", L);
    }

    _lev_co_set_state(co, busy);
    return 0;
}

int lev_is_yieldable(lua_State* L, ev_loop_t* loop)
{
    lev_coroutine_t* co = _lev_find_coroutine(L, loop);
    if (co == NULL)
    {
        return 0;
    }

    return lua_isyieldable(L);
}

int lev_error_ex(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* umsg = _lev_strerr(fmt, ap);
    va_end(ap);

    _lev_push_error_object(file, line, L, loop, errcode, umsg);

    ev_free(umsg);

    return lua_error(L);
}

void lev_wakeup(ev_loop_t* loop)
{
    lev_loop_t* self = EV_CONTAINER_OF(loop, lev_loop_t, loop);
    ev_async_wakeup(&self->async);
}

#line 1 "src/lua/channel.lua.c"
/* AMALGAMATE: #include "channel.lua.h" */
#include <string.h>

#define LEV_CHANNEL_NAME    "__ev_channel"

typedef struct lev_channel_s
{
    ev_loop_t*      loop;
    int             is_closed;

    ev_list_t       data_queue;
    ev_list_t       wait_queue;
    ev_list_t       busy_queue;
} lev_channel_t;

typedef struct lev_channel_data_token
{
    ev_list_node_t  node;

    size_t          refkey_sz;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    int             refkey[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_channel_data_token_t;

typedef struct lev_channel_wait_token
{
    ev_list_node_t  node;

    lev_channel_t*  channel;
    lua_State*      wait_L;

    int             is_busy;
} lev_channel_wait_token_t;

static void _lev_channel_drop_data(lua_State* L, lev_channel_data_token_t* token)
{
    size_t i;
    for (i = 0; i < token->refkey_sz; i++)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, token->refkey[i]);
        token->refkey[i] = LUA_NOREF;
    }

    ev_free(token);
}

static void _lev_channel_drop_wait(lua_State* L, lev_channel_wait_token_t* wait_token)
{
    (void)L;

    lev_channel_t* self = wait_token->channel;
    wait_token->channel = NULL;
    wait_token->wait_L = NULL;

    if (wait_token->is_busy)
    {
        ev_list_erase(&self->busy_queue, &wait_token->node);
    }
    else
    {
        ev_list_erase(&self->wait_queue, &wait_token->node);
    }

    ev_free(wait_token);
}

static int _lev_channel_gc(lua_State* L)
{
    ev_list_node_t* it;
    lev_channel_t* self = lua_touserdata(L, 1);

    while ((it = ev_list_pop_front(&self->data_queue)) != NULL)
    {
        lev_channel_data_token_t* data_token = EV_CONTAINER_OF(it, lev_channel_data_token_t, node);
        _lev_channel_drop_data(L, data_token);
    }

    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);
        _lev_channel_drop_wait(L, wait_token);
    }

    while ((it = ev_list_pop_front(&self->busy_queue)) != NULL)
    {
        lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);
        _lev_channel_drop_wait(L, wait_token);
    }

    return 0;
}

static int _lev_channel_send(lua_State* L)
{
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    if (self->is_closed)
    {
        lua_pushinteger(L, EV_EPIPE);
        return 1;
    }

    int sp = lua_gettop(L);

    size_t refkey_sz = sp - 1;
    size_t malloc_size = sizeof(lev_channel_data_token_t) + sizeof(int) * refkey_sz;

    /* Save data */
    lev_channel_data_token_t* data_token = ev_malloc(malloc_size);
    data_token->refkey_sz = refkey_sz;
    for (; sp > 1; sp--)
    {
        data_token->refkey[sp - 2] = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    ev_list_push_back(&self->data_queue, &data_token->node);

    /* Wakeup one */
    ev_list_node_t* node = ev_list_pop_front(&self->wait_queue);
    if (node == NULL)
    {
        goto finish;
    }

    lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(node, lev_channel_wait_token_t, node);
    lev_set_state(wait_token->wait_L, self->loop, 1);
    wait_token->is_busy = 1;
    ev_list_push_back(&self->busy_queue, &wait_token->node);

finish:
    lua_pushnil(L);
    return 1;
}

static int _lev_channel_process_recv(lua_State* L, lev_channel_t* self)
{
    ev_list_node_t* it = ev_list_pop_front(&self->data_queue);
    if (it != NULL)
    {
        lua_pushnil(L);

        lev_channel_data_token_t* data_token = EV_CONTAINER_OF(it, lev_channel_data_token_t, node);
        size_t refkey_sz = data_token->refkey_sz;

        size_t i;
        for (i = 0; i < data_token->refkey_sz; i++)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, data_token->refkey[i]);
            luaL_unref(L, LUA_REGISTRYINDEX, data_token->refkey[i]);
            data_token->refkey[i] = LUA_NOREF;
        }

        ev_free(data_token);
        return 1 + (int)refkey_sz;
    }

    if (self->is_closed)
    {
        lua_pushnumber(L, EV_EOF);
        return 1;
    }

    return lev_error(L, self->loop, EV_EPIPE, "no data");
}

static int _lev_channel_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_channel_wait_token_t* wait_token = (lev_channel_wait_token_t*)ctx;
    lev_channel_t* self = wait_token->channel;

    int ret = _lev_channel_process_recv(L, self);

    _lev_channel_drop_wait(L, wait_token);

    return ret;
}

static int _lev_channel_recv(lua_State* L)
{
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    if (ev_list_size(&self->data_queue) != 0 || self->is_closed)
    {
        return _lev_channel_process_recv(L, self);
    }

    lev_channel_wait_token_t* wait_token = ev_malloc(sizeof(lev_channel_wait_token_t));
    wait_token->wait_L = L;
    wait_token->channel = self;
    wait_token->is_busy = 0;
    ev_list_push_back(&self->wait_queue, &wait_token->node);
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)wait_token, _lev_channel_recv_resume);
}

static void _lev_channel_wakeup_all(lev_channel_t* self)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_channel_wait_token_t* token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);

        token->is_busy = 1;
        ev_list_push_back(&self->busy_queue, &token->node);
        lev_set_state(token->wait_L, token->channel->loop, 1);
    }
}

static int _lev_channel_close(lua_State* L)
{
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    self->is_closed = 1;
    _lev_channel_wakeup_all(self);

    return 0;
}

static lev_channel_t* _lev_make_channel(lua_State* L, ev_loop_t* loop)
{
    lev_channel_t* self = lua_newuserdata(L, sizeof(lev_channel_t));
    memset(self, 0, sizeof(*self));

    self->loop = loop;
    self->is_closed = 0;
    ev_list_init(&self->data_queue);
    ev_list_init(&self->wait_queue);
    ev_list_init(&self->busy_queue);

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_channel_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "close",  _lev_channel_close },
        { "send",   _lev_channel_send },
        { "recv",   _lev_channel_recv },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, LEV_CHANNEL_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_channel(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    _lev_make_channel(L, loop);

    return 1;
}

#line 1 "src/lua/fs.lua.c"
/* AMALGAMATE: #include "fs.lua.h" */

#define LEV_FILE_NAME   "__ev_file"

typedef struct lev_file_s
{
    ev_file_t   file;
    ev_loop_t*  loop;

    int         closed;
    lua_State*  close_L;    /**< Lua state for close */
} lev_file_t;

typedef struct lev_file_open
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_open_t;

typedef struct lev_file_seek
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_seek_t;

typedef struct lev_file_read
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
    size_t      chunk_sz;

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    char        buf[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_file_read_t;

typedef struct lev_file_write
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_write_t;

typedef struct lev_file_stat
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_stat_t;

typedef struct lev_file_remove
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_remove_t;

typedef struct lev_file_readdir
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_readdir_t;

static void _lev_file_on_open(ev_fs_req_t* req)
{
    lev_file_open_t* token = EV_CONTAINER_OF(req, lev_file_open_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_open_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;

    lev_file_open_t* token = (lev_file_open_t*)ctx;
    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result != 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pop(L, 1); /* Remove token. Now file is on top of stack. */
    lua_pushnil(L);
    lua_insert(L, lua_gettop(L) - 1);

    return 2;
}

static int _lev_file_gc(lua_State* L)
{
    lev_file_t* file = lua_touserdata(L, 1);

    if (!file->closed)
    {
        ev_file_exit(&file->file, NULL);
        ev_loop_run(file->loop, EV_LOOP_MODE_NOWAIT);
        file->closed = 1;
    }

    return 0;
}

static void _lev_file_on_read_done(ev_fs_req_t* req)
{
    lev_file_read_t* token = EV_CONTAINER_OF(req, lev_file_read_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_read_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_read_t* token = (lev_file_read_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buf, result);
    return 2;
}

static int _lev_file_read_ex(lua_State* L, lev_file_t* file, size_t size)
{
    int ret;
    size_t malloc_size = sizeof(lev_file_read_t) + size;
    lev_file_read_t* token = lua_newuserdata(L, malloc_size);
    token->chunk_sz = size;

    ev_buf_t buf = ev_buf_make(token->buf, size);
    ret = ev_file_read(&file->file, &token->req, &buf, 1, _lev_file_on_read_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, file->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_read_resume);
}

static int _lev_file_read(lua_State* L)
{
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    size_t read_size = 64 * 1024;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        read_size = lua_tointeger(L, 2);
    }

    return _lev_file_read_ex(L, file, read_size);
}

static void _lev_file_on_write_done(ev_fs_req_t* req)
{
    lev_file_write_t* token = EV_CONTAINER_OF(req, lev_file_write_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_write_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_write_t* token = (lev_file_write_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_file_write(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_file_write_t* token = lua_newuserdata(L, sizeof(lev_file_write_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_file_write(&file->file, &token->req, &buf, 1, _lev_file_on_write_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_write_resume);
}

static void _lev_file_on_seek_done(ev_fs_req_t* req)
{
    lev_file_seek_t* token = EV_CONTAINER_OF(req, lev_file_seek_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_seek_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_seek_t* token = (lev_file_seek_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushinteger(L, result);
    return 2;
}

static int _lev_file_seek(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);
    int whence = (int)luaL_checkinteger(L, 2);
    ssize_t offset = luaL_checkinteger(L, 3);

    lev_file_seek_t* token = lua_newuserdata(L, sizeof(lev_file_seek_t));
    ret = ev_file_seek(&file->file, &token->req, whence, offset, _lev_file_on_seek_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_seek_resume);
}

static void _lev_file_on_stat_done(ev_fs_req_t* req)
{
    lev_file_stat_t* token = EV_CONTAINER_OF(req, lev_file_stat_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_stat_resume(lua_State* L, int status, lua_KContext ctx)
{
#define PUSH_FIELD_UINT64(L, stat, field)  \
    do {\
        lua_pushinteger(L, (stat)->field);\
        lua_setfield(L, -2, #field);\
    } while (0)

#define PUSH_FILED_TIMESPEC(L, stat, field) \
    do {\
        lua_newtable(L);\
        lua_pushinteger(L, (stat)->field.tv_sec);\
        lua_setfield(L, -2, "tv_sec");\
        lua_pushinteger(L, (stat)->field.tv_nsec);\
        lua_setfield(L, -2, "tv_nsec");\
        lua_setfield(L, -2, #field);\
    } while (0)

    (void)status;
    lev_file_stat_t* token = (lev_file_stat_t*)ctx;

    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }
    
    ev_fs_stat_t* stat = ev_fs_get_statbuf(&token->req);

    lua_pushnil(L);
    lua_newtable(L);

    PUSH_FIELD_UINT64(L, stat, st_dev);
    PUSH_FIELD_UINT64(L, stat, st_ino);
    PUSH_FIELD_UINT64(L, stat, st_mode);
    PUSH_FIELD_UINT64(L, stat, st_nlink);
    PUSH_FIELD_UINT64(L, stat, st_uid);
    PUSH_FIELD_UINT64(L, stat, st_gid);
    PUSH_FIELD_UINT64(L, stat, st_rdev);
    PUSH_FIELD_UINT64(L, stat, st_size);
    PUSH_FIELD_UINT64(L, stat, st_blksize);
    PUSH_FIELD_UINT64(L, stat, st_blocks);
    PUSH_FIELD_UINT64(L, stat, st_flags);
    PUSH_FIELD_UINT64(L, stat, st_gen);

    PUSH_FILED_TIMESPEC(L, stat, st_atim);
    PUSH_FILED_TIMESPEC(L, stat, st_mtim);
    PUSH_FILED_TIMESPEC(L, stat, st_ctim);
    PUSH_FILED_TIMESPEC(L, stat, st_birthtim);

    return 2;

#undef PUSH_FIELD_UINT64
#undef PUSH_FILED_TIMESPEC
}

static int _lev_file_stat(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);
    lev_file_stat_t* token = lua_newuserdata(L, sizeof(lev_file_stat_t));

    ret = ev_file_stat(&file->file, &token->req, _lev_file_on_stat_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_stat_resume);
}

static void _lev_file_on_close_done(ev_file_t* file)
{
    lev_file_t* impl = EV_CONTAINER_OF(file, lev_file_t, file);

    lev_set_state(impl->close_L, impl->loop, 1);
    impl->close_L = NULL;
}

static int _lev_file_on_close_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)L; (void)status; (void)ctx;
    return 0;
}

static int _lev_file_close(lua_State* L)
{
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    if (file->closed)
    {
        return 0;
    }

    file->closed = 1;
    ev_file_exit(&file->file, _lev_file_on_close_done);

    file->close_L = L;
    lev_set_state(L, file->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, _lev_file_on_close_resume);
}

static void _lev_fs_on_remove_done(ev_fs_req_t* req)
{
    lev_file_remove_t* token = EV_CONTAINER_OF(req, lev_file_remove_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_fs_on_remove_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_remove_t* token = (lev_file_remove_t*)ctx;

    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static void _lev_fs_on_readdir_done(ev_fs_req_t* req)
{
    lev_file_readdir_t* token = EV_CONTAINER_OF(req, lev_file_readdir_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static const char* _lev_fs_dirent_type_tostring(ev_dirent_type_t type)
{
    switch (type)
    {
    case EV_DIRENT_FILE:    return "file";
    case EV_DIRENT_DIR:     return "dir";
    case EV_DIRENT_LINK:    return "link";
    case EV_DIRENT_FIFO:    return "fifo";
    case EV_DIRENT_SOCKET:  return "socket";
    case EV_DIRENT_CHR:     return "chr";
    case EV_DIRENT_BLOCK:   return "block";
    default:                return "unknown";
    }
}

static int _lev_fs_on_readdir_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;

    lev_file_readdir_t* token = (lev_file_readdir_t*)ctx;
    if (token->req.result < 0)
    {
        lua_pushinteger(L, token->req.result);
        lua_pushnil(L);
        goto finish;
    }

    lua_pushnil(L);
    lua_newtable(L);

    ev_dirent_t* dir = ev_fs_get_first_dirent(&token->req);
    for (; dir != NULL; dir = ev_fs_get_next_dirent(dir))
    {
        lua_pushstring(L, _lev_fs_dirent_type_tostring(dir->type));
        lua_setfield(L, -2, dir->name);
    }

finish:
    ev_fs_req_cleanup(&token->req);
    return 2;
}

int lev_fs_file(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);
    int flags = EV_FS_O_RDWR;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        flags = (int)lua_tointeger(L, 3);
    }

    int mode = EV_FS_S_IRUSR | EV_FS_S_IWUSR;
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
        mode = (int)lua_tointeger(L, 4);
    }

    lev_file_t* file = lua_newuserdata(L, sizeof(lev_file_t)); /* SP + 1 */
    file->closed = 0;
    file->loop = loop;

    if ((ret = ev_file_init(loop, &file->file)) != 0)
    {
        file->closed = 1;
        return lev_error(L, loop, ret, NULL);
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_file_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "close",  _lev_file_close },
        { "read",   _lev_file_read },
        { "seek",   _lev_file_seek },
        { "stat",   _lev_file_stat },
        { "write",  _lev_file_write },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, LEV_FILE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    lev_file_open_t* token = lua_newuserdata(L, sizeof(lev_file_open_t)); /* SP + 2 */
    ret = ev_file_open(&file->file, &token->req, path, flags, mode, _lev_file_on_open);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = loop;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_open_resume);
}

int lev_fs_remove(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);
    int recursion = 0;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        recursion = (int)lua_tonumber(L, 3);
    }

    lev_file_remove_t* token = lua_newuserdata(L, sizeof(lev_file_remove_t));

    ret = ev_fs_remove(loop, &token->req, path, recursion, _lev_fs_on_remove_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->loop = loop;
    token->L = L;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_fs_on_remove_resume);
}

int lev_fs_readdir(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);

    lev_file_readdir_t* token = lua_newuserdata(L, sizeof(lev_file_readdir_t));

    int ret = ev_fs_readdir(loop, &token->req, path, _lev_fs_on_readdir_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->loop = loop;
    token->L = L;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_fs_on_readdir_resume);
}

ev_file_t* lev_try_to_file(lua_State* L, int idx)
{
    lev_file_t* self = luaL_testudata(L, idx, LEV_FILE_NAME);
    return self != NULL ? &self->file : NULL;
}

#line 1 "src/lua/misc.lua.c"
/* AMALGAMATE: #include "misc.lua.h" */

#define LEV_SOCKADDR_STORAGE_NAME   "__ev_sockaddr_storage"

int lev_hrtime(lua_State* L)
{
    static uint64_t basis = 0;

    uint64_t t = ev_hrtime();
    if (basis == 0)
    {
        basis = t;
    }

    uint64_t dif = t - basis;

    lua_pushinteger(L, (lua_Integer)dif);
    return 1;
}

int lev_strerror(lua_State* L)
{
    int errcode = (int)luaL_checkinteger(L, 1);
    lua_pushstring(L, ev_strerror(errcode));
    return 1;
}

struct sockaddr_storage* lev_push_sockaddr_storage(lua_State* L)
{
    size_t malloc_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage* addr = lua_newuserdata(L, malloc_size);

    luaL_newmetatable(L, LEV_SOCKADDR_STORAGE_NAME);
    lua_setmetatable(L, -2);

    return addr;
}

struct sockaddr_storage* lev_to_sockaddr_storage(lua_State* L, int idx)
{
    return luaL_checkudata(L, idx, LEV_SOCKADDR_STORAGE_NAME);
}

int lev_ip_addr(lua_State* L)
{
    int ret;
    const char* ip = luaL_checkstring(L, 1);
    int port = (int)luaL_checkinteger(L, 2);

    size_t addr_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    if ((ret = ev_ip_addr(ip, port, (struct sockaddr*)addr, addr_size)) != 0)
    {
        return lev_error(L, NULL, ret, NULL);
    }

    return 1;
}

int lev_ip_name(lua_State* L)
{
    int ret;
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 1);

    char ip[64]; int port = 0;
    if ((ret = ev_ip_name((struct sockaddr*)addr, &port, ip, sizeof(ip))) < 0)
    {
        return lev_error(L, NULL, ret, NULL);
    }

    lua_pushstring(L, ip);
    lua_pushinteger(L, port);
    return 2;
}

int lev_arg_pack(lua_State* L)
{
    int i;
    int sp = lua_gettop(L);

    lua_newtable(L); // sp + 1
    for (i = 1; i <= sp; i++)
    {
        lua_pushvalue(L, i); // sp + 2
        lua_rawseti(L, sp + 1, i);
    }

    return 1;
}

int lev_arg_unpack(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    int sp = lua_gettop(L);
    lua_State* tmp = lua_newthread(L); // sp + 1

    lua_pushnil(L); // sp + 2
    while (lua_next(L, 1) != 0)
    {// key at sp+2, value at sp+3
        lua_xmove(L, tmp, 1); // sp + 2
    }

    int tmp_sp = lua_gettop(tmp);
    lua_xmove(tmp, L, tmp_sp);
    lua_remove(L, sp + 1);

    return tmp_sp;
}

#line 1 "src/lua/pipe.lua.c"
/* AMALGAMATE: #include "pipe.lua.h" */
#include <string.h>

#define LEV_PIPE_NAME   "__ev_pipe"

typedef struct lev_pipe_s
{
    ev_pipe_t           pipe;

    ev_loop_t*          loop;
    lua_State*          L;

    int                 closed;
    lua_State*          close_L;
} lev_pipe_t;

typedef struct lev_pipe_send_token
{
    ev_pipe_write_req_t token;

    lua_State*          L;
    ev_loop_t*          loop;

    ssize_t             ret;
} lev_pipe_send_token_t;

typedef struct lev_pipe_recv_token
{
    ev_pipe_read_req_t  token;

    lua_State*          L;
    ev_loop_t*          loop;

    size_t              cap;
    ssize_t             ret;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    char                buffer[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_pipe_recv_token_t;

static int _lev_pipe_sync_close(lev_pipe_t* self)
{
    if (!self->closed)
    {
        self->closed = 1;
        ev_pipe_exit(&self->pipe, NULL);
        ev_loop_run(self->loop, EV_LOOP_MODE_NOWAIT);
    }
    return 0;
}

static int _lev_pipe_gc(lua_State* L)
{
    lev_pipe_t* self = lua_touserdata(L, 1);
    return _lev_pipe_sync_close(self);
}

static void _lev_pipe_on_exit(ev_pipe_t* handle)
{
    lev_pipe_t* self = EV_CONTAINER_OF(handle, lev_pipe_t, pipe);

    lev_set_state(self->close_L, self->loop, 1);
    self->close_L = NULL;
}

static int _lev_pipe_close(lua_State* L)
{
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);

    if (self->closed)
    {
        return 0;
    }

    self->closed = 1;
    if (!lev_is_yieldable(L, self->loop))
    {
        return _lev_pipe_sync_close(self);
    }

    self->close_L = L;
    ev_pipe_exit(&self->pipe, _lev_pipe_on_exit);
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static void _lev_pipe_on_send_done(ev_pipe_write_req_t* req, ssize_t size)
{
    lev_pipe_send_token_t* token = EV_CONTAINER_OF(req, lev_pipe_send_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_pipe_on_send_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_pipe_send_token_t* token = (lev_pipe_send_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_pipe_send(lua_State* L)
{
    int ret;
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);
    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_pipe_send_token_t* token = lua_newuserdata(L, sizeof(lev_pipe_send_token_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_pipe_write(&self->pipe, &token->token, &buf, 1, _lev_pipe_on_send_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->L = L;
    token->loop = self->loop;
    token->ret = 0;
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_pipe_on_send_resume);
}

static void _lev_pipe_on_recv_done(ev_pipe_read_req_t* req, ssize_t result)
{
    lev_pipe_recv_token_t* token = EV_CONTAINER_OF(req, lev_pipe_recv_token_t, token);

    token->ret = result;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_pipe_on_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_pipe_recv_token_t* token = (lev_pipe_recv_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buffer, token->ret);
    return 2;
}

static int _lev_pipe_recv(lua_State* L)
{
    int ret;
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);

    size_t cap = 64 * 1024;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        cap = lua_tointeger(L, 2);
    }

    size_t malloc_size = sizeof(lev_pipe_recv_token_t) + cap;
    lev_pipe_recv_token_t* token = lua_newuserdata(L, malloc_size);
    token->cap = cap;

    ev_buf_t buf = ev_buf_make(token->buffer, token->cap);
    ret = ev_pipe_read(&self->pipe, &token->token, &buf, 1, _lev_pipe_on_recv_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = self->loop;
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_pipe_on_recv_resume);
}

static lev_pipe_t* _lev_make_pipe(lua_State* L, ev_loop_t* loop)
{
    int ret;
    lev_pipe_t* self = lua_newuserdata(L, sizeof(lev_pipe_t));
    memset(self, 0, sizeof(*self));

    self->loop = loop;
    if ((ret = ev_pipe_init(loop, &self->pipe, 0)) != 0)
    {
        lev_error(L, loop, ret, NULL);
        return NULL;
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_pipe_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "send",       _lev_pipe_send },
        { "recv",       _lev_pipe_recv },
        { "close",      _lev_pipe_close },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PIPE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_pipe(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    _lev_make_pipe(L, loop);

    return 1;
}

ev_pipe_t* lev_try_to_pipe(lua_State* L, int idx)
{
    lev_pipe_t* self = luaL_testudata(L, idx, LEV_PIPE_NAME);
    return self != NULL ? &self->pipe : NULL;
}

#line 1 "src/lua/process.lua.c"
/* AMALGAMATE: #include "process.lua.h" */
/* AMALGAMATE: #include "fs.lua.h" */
/* AMALGAMATE: #include "pipe.lua.h" */
#include <string.h>
#include <stdlib.h>

#define LEV_PROCESS_NAME            "__ev_process"
#define LEV_PROCESS_WAITPID_NAME    "__ev_process_waitpid"

#if defined(_MSC_VER)
#   define strdup(s) _strdup(s)
#endif

typedef struct lev_process_s
{
    ev_process_t                process;
    ev_loop_t*                  loop;

    int                         ref_stdin;
    int                         ref_stdout;
    int                         ref_stderr;

    ev_list_t                   wait_queue;

    int                         exited;
    ev_process_exit_status_t    exit_status;
    int                         exit_code;
} lev_process_t;

typedef struct lev_process_wait_token
{
    ev_list_node_t              node;

    lua_State*                  L;
    ev_loop_t*                  loop;

    lev_process_t*              process;
} lev_process_wait_token_t;

static void _lev_process_cleanup_opt(ev_process_options_t* opt)
{
    size_t i;

    if (opt->file != NULL)
    {
        free((char*)opt->file);
        opt->file = NULL;
    }

    for (i = 0; opt->argv != NULL && opt->argv[i] != NULL; i++)
    {
        free(opt->argv[i]);
    }
    opt->argv = NULL;

    for (i = 0; opt->envp != NULL && opt->envp[i] != NULL; i++)
    {
        free(opt->envp[i]);
    }
    opt->envp = NULL;

    if (opt->cwd != NULL)
    {
        free((char*)opt->cwd);
        opt->cwd = NULL;
    }
}

static char** _lev_to_string_array(lua_State* L, ev_loop_t* loop, int idx)
{
    char** argv = NULL;
    size_t len = 1;

    lua_pushnil(L);
    while (lua_next(L, idx) != 0)
    {
        len++;

        char** new_argv = realloc(argv, sizeof(char*) * len);
        if (new_argv == NULL)
        {
            lev_error(L, loop, EV_ENOMEM, NULL);
            return NULL;
        }
        argv = new_argv;

        argv[len - 2] = strdup(luaL_checkstring(L, -1));
        argv[len - 1] = NULL;

        lua_pop(L, 1);
    }

    return argv;
}

static int _lev_process_opt_stdio(lua_State* L, int idx, lev_process_t* process, ev_process_options_t* opt)
{
#define LEV_PIPE_CHECK_STDIO(name, arr) \
    do {\
        int type = LUA_TNIL;\
        ev_pipe_t* pipe = NULL;\
        ev_file_t* file = NULL;\
        if ((type = lua_getfield(L, idx, #name)) != LUA_TNIL) {\
            if (type == LUA_TLIGHTUSERDATA && lua_touserdata(L, top_sp + 1) == NULL) {\
                opt->stdios[arr].flag = EV_PROCESS_STDIO_REDIRECT_NULL;\
            } else if (type == LUA_TUSERDATA && (pipe = lev_try_to_pipe(L, top_sp + 1)) != NULL) {\
                opt->stdios[arr].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;\
                opt->stdios[arr].data.pipe = pipe;\
                lua_pushvalue(L, -1);\
                process->ref_##name = luaL_ref(L, LUA_REGISTRYINDEX);\
            } else if (type == LUA_TUSERDATA && (file = lev_try_to_file(L, top_sp + 1)) != NULL) {\
                opt->stdios[arr].flag = EV_PROCESS_STDIO_REDIRECT_FD;\
                opt->stdios[arr].data.fd = file->file;\
                lua_pushvalue(L, -1);\
                process->ref_##name = luaL_ref(L, LUA_REGISTRYINDEX);\
            } else {\
                return lev_error(L, process->loop, EV_EINVAL, "argument to " #name " is invalid");\
            }\
        }\
        lua_pop(L, 1);\
    } while (0)

    int top_sp = lua_gettop(L);

    LEV_PIPE_CHECK_STDIO(stdin,  0);
    LEV_PIPE_CHECK_STDIO(stdout, 1);
    LEV_PIPE_CHECK_STDIO(stderr, 2);

    return 0;

#undef LEV_PIPE_CHECK_STDIO
}

static int _lev_process_opt(lua_State* L, ev_loop_t* loop, int idx,
    lev_process_t* process, ev_process_options_t* opt)
{
    int sp = lua_gettop(L);
    if (lua_type(L, idx) != LUA_TTABLE)
    {
        return 0;
    }

    if (lua_getfield(L, idx, "file") == LUA_TSTRING)
    {
        free((char*)opt->file);
        opt->file = strdup(lua_tostring(L, -1));
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "argv") == LUA_TTABLE) /* SP + 1 */
    {
        opt->argv = _lev_to_string_array(L, loop, sp + 1);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "envp") == LUA_TTABLE)
    {
        opt->envp = _lev_to_string_array(L, loop, sp + 1);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "cwd") == LUA_TSTRING)
    {
        opt->cwd = strdup(lua_tostring(L, -1));
    }
    lua_pop(L, 1);

    _lev_process_opt_stdio(L, idx, process, opt);

    return 0;
}

static int _lev_process_gc(lua_State* L)
{
    lev_process_t* process = lua_touserdata(L, 1);

    if (process->ref_stdin != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stdin);
        process->ref_stdin = LUA_NOREF;
    }
    if (process->ref_stdout != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stdout);
        process->ref_stdout = LUA_NOREF;
    }
    if (process->ref_stderr != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stderr);
        process->ref_stderr = LUA_NOREF;
    }

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&process->wait_queue)) != NULL)
    {
        lev_process_wait_token_t* token = EV_CONTAINER_OF(it, lev_process_wait_token_t, node);
        lev_set_state(token->L, token->loop, 1);
        token->process = NULL;
        token->loop = NULL;
    }

    ev_process_exit(&process->process, NULL);
    ev_loop_run(process->loop, EV_LOOP_MODE_NOWAIT);

    return 0;
}

static int _lev_process_waitpid_gc(lua_State* L)
{
    lev_process_wait_token_t* token = lua_touserdata(L, 1);

    if (token->process != NULL)
    {
        ev_list_erase(&token->process->wait_queue, &token->node);
        token->process = NULL;
    }

    token->loop = NULL;

    return 0;
}

static int _lev_path_template(lua_State* L, ssize_t(*cb)(char*, size_t))
{
    size_t size = 4096;
    char* buffer = ev_malloc(size);
    if (buffer == NULL)
    {
        goto err_nomem;
    }

    ssize_t len = cb(buffer, size);
    if (len < 0)
    {
        ev_free(buffer); buffer = NULL;
        return lev_error(L, NULL, (int)len, NULL);
    }

    if ((size_t)len >= size)
    {
        char* new_buf = ev_realloc(buffer, len + 1);
        if (new_buf == NULL)
        {
            goto err_nomem;
        }
        buffer = new_buf;

        len = cb(buffer, len + 1);
    }

    lua_pushlstring(L, buffer, len);

    ev_free(buffer);
    buffer = NULL;

    return 1;

err_nomem:
    if (buffer != NULL)
    {
        ev_free(buffer);
        buffer = NULL;
    }
    return lev_error(L, NULL, EV_ENOMEM, NULL);
}

static void _lev_process_on_exit(ev_process_t* handle,
    ev_process_exit_status_t exit_status, int exit_code)
{
    lev_process_t* self = EV_CONTAINER_OF(handle, lev_process_t, process);

    self->exited = 1;
    self->exit_status = exit_status;
    self->exit_code = exit_code;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_process_wait_token_t* token = EV_CONTAINER_OF(it, lev_process_wait_token_t, node);
        lev_set_state(token->L, token->loop, 1);
        token->process = NULL;
    }
}

static int _lev_process_waitpid_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_process_wait_token_t* token = (lev_process_wait_token_t*)ctx;

    if (token->process == NULL)
    {
        return 0;
    }

    lua_pushinteger(L, token->process->exit_code);
    return 1;
}

static int _lev_process_waitpid(lua_State* L)
{
    lev_process_t* self = luaL_checkudata(L, 1, LEV_PROCESS_NAME);

    if (self->exited)
    {
        lua_pushinteger(L, self->exit_code);
        return 1;
    }

    lev_process_wait_token_t* token = lua_newuserdata(L, sizeof(lev_process_wait_token_t));
    token->L = L;
    token->loop = self->loop;
    token->process = self;

    static const luaL_Reg s_meta[] = {
         { "__gc",       _lev_process_waitpid_gc },
         { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROCESS_WAITPID_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    ev_list_push_back(&self->wait_queue, &token->node);
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_process_waitpid_resume);
}

int lev_process(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    lev_process_t* process = lua_newuserdata(L, sizeof(lev_process_t));
    memset(process, 0, sizeof(*process));
    ev_list_init(&process->wait_queue);
    process->ref_stdin = LUA_NOREF;
    process->ref_stdout = LUA_NOREF;
    process->ref_stderr = LUA_NOREF;

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.on_exit = _lev_process_on_exit;
    _lev_process_opt(L, loop, 2, process, &opt);
    process->loop = loop;

    int ret = ev_process_spawn(loop, &process->process, &opt);
    _lev_process_cleanup_opt(&opt);

    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_process_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "waitpid",    _lev_process_waitpid },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROCESS_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    lua_pushnil(L);
    lua_insert(L, lua_gettop(L) - 1);

    return 2;
}

int lev_getcwd(lua_State* L)
{
    return _lev_path_template(L, ev_getcwd);
}

int lev_exepath(lua_State* L)
{
    return _lev_path_template(L, ev_exepath);
}

#line 1 "src/lua/promise.lua.c"
/* AMALGAMATE: #include "promise.lua.h" */

#define LEV_PROMISE_NAME    "__ev_promise"

typedef struct lev_promise_value
{
    int                     refkey_sz;
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4200)
#endif
    int                     refkey[];
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif
} lev_promise_value_t;

struct lev_promise_s
{
    ev_loop_t*              loop;

    lev_promise_value_t*    value;
    ev_list_t               wait_queue;
    ev_list_t               busy_queue;
};

typedef struct lev_promise_wait_token
{
    ev_list_node_t          node;

    lua_State*              wait_L;
    ev_loop_t*              loop;

    lev_promise_cb          cb;
    void*                   data;

    int                     is_busy;
    lev_promise_t*          promise;
} lev_promise_wait_token_t;

static void _lev_promise_drop_value(lua_State* L, lev_promise_t* self)
{
    if (self->value == NULL)
    {
        return;
    }

    int i;
    for (i = 0; i < self->value->refkey_sz; i++)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, self->value->refkey[i]);
        self->value->refkey[i] = LUA_NOREF;
    }
    ev_free(self->value);
    self->value = NULL;
}

static void _lev_promise_drop_wait_token(lev_promise_wait_token_t* token)
{
    lev_promise_t* self = token->promise;

    if (token->is_busy)
    {
        ev_list_erase(&self->busy_queue, &token->node);
    }
    else
    {
        ev_list_erase(&self->wait_queue, &token->node);
    }

    ev_free(token);
}

static int _lev_promise_gc(lua_State* L)
{
    ev_list_node_t* it;
    lev_promise_t* self = lua_touserdata(L, 1);

    _lev_promise_drop_value(L, self);

    while ((it = ev_list_begin(&self->wait_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        _lev_promise_drop_wait_token(token);
    }

    while ((it = ev_list_begin(&self->busy_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        _lev_promise_drop_wait_token(token);
    }

    return 0;
}

static void _lev_promise_wakeup_all(lev_promise_t* self)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        token->is_busy = 1;
        ev_list_push_back(&self->busy_queue, &token->node);
        lev_set_state(token->wait_L, token->loop, 1);
    }
}

static int _lev_promise_set_value(lua_State* L)
{
    lev_promise_t* self = luaL_checkudata(L, 1, LEV_PROMISE_NAME);

    int sp = lua_gettop(L);
    int refkey_sz = sp - 1;

    int ret = lev_promise_set_value(L, self, refkey_sz);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int _lev_promise_push_value(lua_State* L, lev_promise_t* self, lev_promise_cb cb, void* data)
{
    int i;
    lev_promise_value_t* value = self->value;
    if (value == NULL)
    {
        return cb(L, self, 0, data);
    }

    for (i = 0; i < value->refkey_sz; i++)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value->refkey[i]);
    }
    return cb(L, self, value->refkey_sz, data);
}

static int _lev_promise_get_value_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_promise_wait_token_t* token = (lev_promise_wait_token_t*)ctx;
    lev_promise_t* self = token->promise;

    lev_promise_cb cb = token->cb;
    void* data = token->data;

    _lev_promise_drop_wait_token(token);

    return _lev_promise_push_value(L, self, cb, data);
}

static int _lev_promise_on_get_value(lua_State* L, lev_promise_t* promise, int narg, void* data)
{
    (void)L; (void)promise; (void)data;
    return narg;
}

static int _lev_promise_get_value(lua_State* L)
{
    lev_promise_t* self = luaL_checkudata(L, 1, LEV_PROMISE_NAME);
    return lev_promise_get_value(L, self, _lev_promise_on_get_value, NULL);
}

int lev_promise_get_value(lua_State* L, lev_promise_t* promise, lev_promise_cb cb, void* data)
{
    if (promise->value != NULL)
    {
        return _lev_promise_push_value(L, promise, cb, data);
    }

    lev_promise_wait_token_t* token = ev_malloc(sizeof(lev_promise_wait_token_t));
    if (token == NULL)
    {
        return lev_error(L, promise->loop, EV_ENOMEM, NULL);
    }

    token->is_busy = 0;
    token->wait_L = L;
    token->loop = promise->loop;
    token->promise = promise;
    token->cb = cb;
    token->data = data;

    ev_list_push_back(&promise->wait_queue, &token->node);
    lev_set_state(L, promise->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_promise_get_value_resume);
}

lev_promise_t* lev_promise_make(lua_State* L, ev_loop_t* loop)
{
    lev_promise_t* self = lua_newuserdata(L, sizeof(lev_promise_t));

    self->loop = loop;
    self->value = NULL;
    ev_list_init(&self->wait_queue);
    ev_list_init(&self->busy_queue);

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_promise_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "set_value",  _lev_promise_set_value },
        { "get_value",  _lev_promise_get_value },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROMISE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_promise(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    lev_promise_make(L, loop);

    return 1;
}

int lev_promise_set_value(lua_State* L, lev_promise_t* promise, int narg)
{
    if (promise->value != NULL)
    {
        return EV_EEXIST;
    }

    size_t malloc_size = sizeof(lev_promise_value_t) + sizeof(int) * narg;
    if ((promise->value = ev_malloc(malloc_size)) == NULL)
    {
        return lev_error(L, promise->loop, EV_ENOMEM, NULL);
    }

    /* Save value */
    promise->value->refkey_sz = narg;
    for (; narg > 0; narg--)
    {
        promise->value->refkey[narg - 1] = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    _lev_promise_wakeup_all(promise);
    lev_wakeup(promise->loop);

    return 0;
}

#line 1 "src/lua/tcp.lua.c"
/* AMALGAMATE: #include "tcp.lua.h" */
/* AMALGAMATE: #include "misc.lua.h" */
#include <string.h>

#define LEV_TCP_NAME                "__ev_tcp"

typedef struct lev_tcp_s
{
    ev_tcp_t            sock;

    ev_loop_t*          loop;
    lua_State*          L;

    int                 closed;
    lua_State*          close_L;

    int                 ret;
} lev_tcp_t;

typedef struct lev_tcp_write_token
{
    ev_tcp_write_req_t  token;

    lua_State*          L;
    ev_loop_t*          loop;

    ssize_t             ret;
} lev_tcp_write_token_t;

typedef struct lev_tcp_recv_token
{
    ev_tcp_read_req_t   token;

    lua_State*          L;
    ev_loop_t*          loop;

    char                buffer[64 * 1024];
    ssize_t             ret;
} lev_tcp_recv_token_t;

static lev_tcp_t* lev_make_tcp(lua_State* L, ev_loop_t* loop);

static void _lev_tcp_sync_close(lev_tcp_t* tcp)
{
    if (!tcp->closed)
    {
        tcp->closed = 1;
        ev_tcp_exit(&tcp->sock, NULL);
        ev_loop_run(tcp->loop, EV_LOOP_MODE_NOWAIT);
    }
}

static int _lev_tcp_gc(lua_State* L)
{
    lev_tcp_t* tcp = lua_touserdata(L, 1);
    _lev_tcp_sync_close(tcp);
    return 0;
}

static void _lev_tcp_on_connect(ev_tcp_t* sock, int stat)
{
    lev_tcp_t* tcp = EV_CONTAINER_OF(sock, lev_tcp_t, sock);
    tcp->ret = stat;

    lev_set_state(tcp->L, tcp->loop, 1);
}

static int _lev_tcp_on_connect_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_t* tcp = (lev_tcp_t*)ctx;
    int ret = tcp->ret;

    if (ret < 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static int _lev_tcp_connect(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    ret = ev_tcp_connect(&tcp->sock, (struct sockaddr*)addr, sizeof(*addr), _lev_tcp_on_connect);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    tcp->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)tcp, _lev_tcp_on_connect_resume);
}

static void _lev_tcp_on_write_done(ev_tcp_write_req_t* req, ssize_t size)
{
    lev_tcp_write_token_t* token = EV_CONTAINER_OF(req, lev_tcp_write_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_tcp_on_write_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_write_token_t* token = (lev_tcp_write_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
        return 1;
    }

    return 0;
}

static int _lev_tcp_send(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_tcp_write_token_t* token = lua_newuserdata(L, sizeof(lev_tcp_write_token_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_tcp_write(&tcp->sock, &token->token, &buf, 1, _lev_tcp_on_write_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->loop = tcp->loop;
    token->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_tcp_on_write_resume);
}

static void _lev_tcp_on_read_done(ev_tcp_read_req_t* req, ssize_t size)
{
    lev_tcp_recv_token_t* token = EV_CONTAINER_OF(req, lev_tcp_recv_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_tcp_on_read_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_recv_token_t* token = (lev_tcp_recv_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buffer, token->ret);
    return 2;
}

static int _lev_tcp_recv(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    if (tcp->closed)
    {
        lua_pushinteger(L, EV_EPIPE);
        lua_pushnil(L);
        return 2;
    }

    lev_tcp_recv_token_t* token = lua_newuserdata(L, sizeof(lev_tcp_recv_token_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_tcp_read(&tcp->sock, &token->token, &buf, 1, _lev_tcp_on_read_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = tcp->loop;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_tcp_on_read_resume);
}

static int _lev_tcp_listen(lua_State* L)
{
    int ret = 0;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    int backlog = 1024;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        backlog = (int)lua_tointeger(L, 3);
    }

    if ((ret = ev_tcp_bind(&tcp->sock, (struct sockaddr*)addr, sizeof(*addr))) != 0)
    {
        goto finish;
    }

    if ((ret = ev_tcp_listen(&tcp->sock, backlog)) != 0)
    {
        goto finish;
    }

finish:
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static void _lev_tcp_on_accept(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    (void)lisn;
    lev_tcp_t* sock = EV_CONTAINER_OF(conn, lev_tcp_t, sock);
    sock->ret = stat;

    lev_set_state(sock->L, sock->loop, 1);
}

static int _lev_tcp_on_accept_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    int sp = lua_gettop(L);
    lev_tcp_t* sock = (lev_tcp_t*)ctx;
    int ret = sock->ret;

    if (ret == 0)
    {
        lua_pushnil(L);
        lua_insert(L, sp);
        return 2;
    }

    lua_pushinteger(L, ret);
    lua_pushnil(L);

    return 2;
}

static int _lev_tcp_accept(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    lev_tcp_t* sock = lev_make_tcp(L, tcp->loop);

    if ((ret = ev_tcp_accept(&tcp->sock, &sock->sock, _lev_tcp_on_accept)) != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    sock->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)sock, _lev_tcp_on_accept_resume);
}

static int _lev_tcp_name(lua_State* L, int(*cb)(ev_tcp_t*, struct sockaddr*, size_t*))
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    size_t addr_len = sizeof(*addr);
    if ((ret = cb(&tcp->sock, (struct sockaddr*)addr, &addr_len)) != 0)
    {
        return lev_error(L, tcp->loop, ret, NULL);
    }

    return 1;
}

static int _lev_tcp_sockname(lua_State* L)
{
    return _lev_tcp_name(L, ev_tcp_getsockname);
}

static int _lev_tcp_peername(lua_State* L)
{
    return _lev_tcp_name(L, ev_tcp_getpeername);
}

static void _lev_tcp_on_close(ev_tcp_t* sock)
{
    lev_tcp_t* tcp = EV_CONTAINER_OF(sock, lev_tcp_t, sock);

    lev_set_state(tcp->close_L, tcp->loop, 1);
    tcp->close_L = NULL;
}

static int _lev_tcp_close(lua_State* L)
{
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    if (tcp->closed)
    {
        return 0;
    }

    tcp->closed = 1;
    if (!lev_is_yieldable(L, tcp->loop))
    {
        _lev_tcp_sync_close(tcp);
        return 0;
    }

    tcp->close_L = L;
    ev_tcp_exit(&tcp->sock, _lev_tcp_on_close);
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static lev_tcp_t* lev_make_tcp(lua_State* L, ev_loop_t* loop)
{
    int  ret;
    lev_tcp_t* tcp = lua_newuserdata(L, sizeof(lev_tcp_t));
    memset(tcp, 0, sizeof(*tcp));

    tcp->loop = loop;
    if ((ret = ev_tcp_init(loop, &tcp->sock)) != 0)
    {
        tcp->closed = 1;
        lev_error(L, loop, ret, NULL);
        return NULL;
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_tcp_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "accept",     _lev_tcp_accept },
        { "close",      _lev_tcp_close },
        { "connect",    _lev_tcp_connect },
        { "listen",     _lev_tcp_listen },
        { "send",       _lev_tcp_send },
        { "recv",       _lev_tcp_recv },
        { "sockname",   _lev_tcp_sockname },
        { "peername",   _lev_tcp_peername },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_TCP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return tcp;
}

int lev_tcp(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    lev_make_tcp(L, loop);

    return 1;
}

#line 1 "src/lua/timer.lua.c"
/* AMALGAMATE: #include "timer.lua.h" */

typedef struct lev_timer
{
    ev_timer_t      timer;
    ev_loop_t*      loop;
    lua_State*      L;
} lev_timer_t;

static void _lev_on_timer(ev_timer_t* timer)
{
    lev_timer_t* self = EV_CONTAINER_OF(timer, lev_timer_t, timer);
    lev_set_state(self->L, self->loop, 1);
}

static int _lev_on_sleep_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)L; (void)status; (void)ctx;
    return 0;
}

static int _lev_timer_gc(lua_State* L)
{
    lev_timer_t* timer = lua_touserdata(L, 1);

    ev_timer_exit(&timer->timer, NULL);
    ev_loop_run(timer->loop, EV_LOOP_MODE_NOWAIT);

    return 0;
}

int lev_sleep(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    uint64_t timeout = luaL_checkinteger(L, 2);

    lev_timer_t* timer = lua_newuserdata(L, sizeof(lev_timer_t));
    if ((ret = ev_timer_init(loop, &timer->timer)) != 0)
    {
        return lev_error(L, loop, ret, NULL);
    }

    timer->L = L;
    timer->loop = loop;

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_timer_gc },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, "__ev_timer") != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    if ((ret = ev_timer_start(&timer->timer, _lev_on_timer, timeout, 0)) != 0)
    {
        return lev_error(L, loop, ret, NULL);
    }

    lev_set_state(L, loop, 0);
    return lua_yieldk(L, 0, (lua_KContext)NULL, _lev_on_sleep_resume);
}

#line 1 "src/lua/udp.lua.c"
/* AMALGAMATE: #include "udp.lua.h" */
/* AMALGAMATE: #include "misc.lua.h" */
#include <string.h>
#include <stdlib.h>

#define LEV_UDP_NAME    "__ev_udp"

typedef struct lev_udp_s
{
    ev_udp_t                sock;
    ev_loop_t*              loop;

    lua_State*              close_L;
    int                     closed;
} lev_udp_t;

typedef struct lev_udp_send
{
    ev_udp_write_t          req;

    lua_State*              L;
    ev_loop_t*              loop;

    ssize_t                 result;
} lev_udp_send_t;

typedef struct lev_udp_recv
{
    ev_udp_read_t           req;
    lua_State*              L;
    ev_loop_t*              loop;

    struct sockaddr_storage storage;

    ssize_t                 result;
    char                    buffer[64 * 1024];
} lev_udp_recv_t;

static void _lev_udp_close_sync(lev_udp_t* udp)
{
    if (!udp->closed)
    {
        udp->closed = 1;
        ev_udp_exit(&udp->sock, NULL);
        ev_loop_run(udp->loop, EV_LOOP_MODE_NOWAIT);
    }
}

static int _lev_udp_gc(lua_State* L)
{
    lev_udp_t* udp = lua_touserdata(L, 1);
    _lev_udp_close_sync(udp);

    return 0;
}

static int _lev_udp_bind(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    int flags = 0;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        flags = (int)lua_tointeger(L, 3);
    }

    if ((ret = ev_udp_bind(&udp->sock, (struct sockaddr*)addr, flags)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_connect(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    if ((ret = ev_udp_connect(&udp->sock, (struct sockaddr*)addr)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_name(lua_State* L, int(*cb)(ev_udp_t*,struct sockaddr*,size_t*))
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    size_t addr_len = sizeof(*addr);
    if ((ret = cb(&udp->sock, (struct sockaddr*)addr, &addr_len)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 1;
}

static int _lev_udp_getsockname(lua_State* L)
{
    return _lev_udp_name(L, ev_udp_getsockname);
}

static int _lev_udp_getpeername(lua_State* L)
{
    return _lev_udp_name(L, ev_udp_getpeername);
}

static int _lev_udp_set_membership(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* multicast_addr = luaL_checkstring(L, 2);
    const char* interface_addr = luaL_checkstring(L, 3);
    int membership = (int)luaL_checkinteger(L, 4);

    int ret = ev_udp_set_membership(&udp->sock, multicast_addr, interface_addr, membership);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_source_membership(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* multicast_addr = luaL_checkstring(L, 2);
    const char* interface_addr = luaL_checkstring(L, 3);
    const char* source_addr = luaL_checkstring(L, 4);
    int membership = (int)luaL_checkinteger(L, 5);

    int ret = ev_udp_set_source_membership(&udp->sock, multicast_addr,
        interface_addr, source_addr, membership);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_multicast_loop(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int on = lua_toboolean(L, 2);

    if ((ret = ev_udp_set_multicast_loop(&udp->sock, on)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_multicast_ttl(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int ttl = (int)luaL_checkinteger(L, 2);

    if ((ret = ev_udp_set_multicast_ttl(&udp->sock, ttl)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_multicast_interface(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* interface_addr = luaL_checkstring(L, 2);

    if ((ret = ev_udp_set_multicast_interface(&udp->sock, interface_addr)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_broadcast(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int on = lua_toboolean(L, 2);

    if ((ret = ev_udp_set_broadcast(&udp->sock, on)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_udp_set_ttl(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int ttl = (int)luaL_checkinteger(L, 2);

    if ((ret = ev_udp_set_ttl(&udp->sock, ttl)) != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static void _lev_udp_on_send_done(ev_udp_write_t* req, ssize_t size)
{
    lev_udp_send_t* token = EV_CONTAINER_OF(req, lev_udp_send_t, req);
    token->result = size;

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_send_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_send_t* token = (lev_udp_send_t*)ctx;

    if (token->result < 0)
    {
        lua_pushinteger(L, token->result);
    }
    else
    {
        lua_pushnil(L);
    }
    
    return 1;
}

static int _lev_udp_send_template(lua_State* L,
    int(*cb)(ev_udp_t*,ev_udp_write_t*,ev_buf_t*,size_t,const struct sockaddr*, ev_udp_write_cb))
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    struct sockaddr_storage* addr = NULL;
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
        addr = lev_to_sockaddr_storage(L, 3);
    }

    lev_udp_send_t* token = lua_newuserdata(L, sizeof(lev_udp_send_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = cb(&udp->sock, &token->req, &buf, 1, (struct sockaddr*)addr, _lev_udp_on_send_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_send_resume);
}

static int _lev_udp_send(lua_State* L)
{
    return _lev_udp_send_template(L, ev_udp_send);
}

static int _lev_udp_try_send(lua_State* L)
{
    return _lev_udp_send_template(L, ev_udp_try_send);
}

static void _lev_udp_on_recv_done(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    (void)addr;
    lev_udp_recv_t* token = EV_CONTAINER_OF(req, lev_udp_recv_t, req);
    token->result = size;

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_recv_t* token = (lev_udp_recv_t*)ctx;

    if (token->result < 0)
    {
        lua_pushinteger(L, token->result);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buffer, token->result);
    return 2;
}

static int _lev_udp_recv(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    lev_udp_recv_t* token = lua_newuserdata(L, sizeof(lev_udp_recv_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_udp_recv(&udp->sock, &token->req, &buf, 1, _lev_udp_on_recv_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_recv_resume);
}

static void _lev_copy_sockaddr(struct sockaddr_storage* dst, const struct sockaddr* src)
{
    switch (src->sa_family)
    {
    case AF_INET:
        memcpy(dst, src, sizeof(struct sockaddr_in));
        break;

    case AF_INET6:
        memcpy(dst, src, sizeof(struct sockaddr_in6));
        break;

    default:
        abort();
    }
}

static void _lev_udp_on_recvfrom_done(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    lev_udp_recv_t* token = EV_CONTAINER_OF(req, lev_udp_recv_t, req);
    token->result = size;

    if (token->result >= 0)
    {
        _lev_copy_sockaddr(&token->storage, addr);
    }

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_recvfrom_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_recv_t* token = (lev_udp_recv_t*)ctx;

    if (token->result < 0)
    {
        lua_pushinteger(L, token->result);
        lua_pushnil(L);
        lua_pushnil(L);
        return 3;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buffer, token->result);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    memcpy(addr, &token->storage, sizeof(token->storage));

    return 3;
}

static int _lev_udp_recvfrom(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    lev_udp_recv_t* token = lua_newuserdata(L, sizeof(lev_udp_recv_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_udp_recv(&udp->sock, &token->req, &buf, 1, _lev_udp_on_recvfrom_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        lua_pushnil(L);
        return 3;
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_recvfrom_resume);
}

static void _lev_udp_on_close(ev_udp_t* sock)
{
    lev_udp_t* udp = EV_CONTAINER_OF(sock, lev_udp_t, sock);

    lev_set_state(udp->close_L, udp->loop, 1);
    udp->close_L = NULL;
}

static int _lev_udp_close(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    if (udp->closed)
    {
        return 0;
    }

    udp->closed = 1;
    if (!lev_is_yieldable(L, udp->loop))
    {
        _lev_udp_close_sync(udp);
        return 0;
    }

    udp->close_L = L;
    ev_udp_exit(&udp->sock, _lev_udp_on_close);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static lev_udp_t* lev_make_udp(lua_State* L, ev_loop_t* loop, int domain)
{
    int ret;
    lev_udp_t* udp = lua_newuserdata(L, sizeof(lev_udp_t));

    udp->close_L = NULL;
    udp->closed = 0;
    udp->loop = loop;

    if ((ret = ev_udp_init(loop, &udp->sock, domain)) != 0)
    {
        lev_error(L, loop, ret, NULL);
        return NULL;
    }
    
    static const luaL_Reg s_meta[] = {
        { "__gc",                       _lev_udp_gc },
        { NULL,                         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "bind",                       _lev_udp_bind },
        { "close",                      _lev_udp_close },
        { "connect",                    _lev_udp_connect },
        { "getsockname",                _lev_udp_getsockname },
        { "getpeername",                _lev_udp_getpeername },
        { "recv",                       _lev_udp_recv },
        { "recvfrom",                   _lev_udp_recvfrom },
        { "set_membership",             _lev_udp_set_membership },
        { "set_source_membership",      _lev_udp_set_source_membership },
        { "set_multicast_loop",         _lev_udp_set_multicast_loop },
        { "set_multicast_ttl",          _lev_udp_set_multicast_ttl },
        { "set_multicast_interface",    _lev_udp_set_multicast_interface },
        { "set_broadcast",              _lev_udp_set_broadcast },
        { "set_ttl",                    _lev_udp_set_ttl },
        { "send",                       _lev_udp_send },
        { "try_send",                   _lev_udp_try_send },
        { NULL,                         NULL },
    };
    if (luaL_newmetatable(L, LEV_UDP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return udp;
}

int lev_udp(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    int domain = AF_UNSPEC;
    lev_make_udp(L, loop, domain);

    return 1;
}

#endif                           /* FEATURE: EV_HAVE_LUA_BINDING (2/2) */
