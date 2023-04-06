#ifndef __EV_DEFINES_INTERNAL_H__
#define __EV_DEFINES_INTERNAL_H__

#include "ev.h"

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

#if defined(EV_AMALGAMATE_BUILD)
#   if defined(__GNUC__) || defined(__clang__)
#       define API_LOCAL    static __attribute__((unused))
#   else
#       define API_LOCAL    static
#   endif
#elif (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#   define API_LOCAL    __attribute__((visibility ("hidden")))
#else
#   define API_LOCAL
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
