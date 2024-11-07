#ifndef __EV_ERRNO_H__
#define __EV_ERRNO_H__

#include <errno.h>

#if EDOM > 0
#define EV__ERR(x)              (-(x))
#else
#define EV__ERR(x)              (x)
#endif

#if defined(EPERM) && !defined(_WIN32)
#   define EV__EPERM            EV__ERR(EPERM)
#else
#   define EV__EPERM            (-4093)
#endif

#if defined(ENOENT) && !defined(_WIN32)
#   define EV__ENOENT           EV__ERR(ENOENT)
#else
#   define EV__ENOENT           (-4092)
#endif

#if defined(EIO) && !defined(_WIN32)
#   define EV__EIO              EV__ERR(EIO)
#else
#   define EV__EIO              (-4089)
#endif

#if defined(E2BIG) && !defined(_WIN32)
#   define EV__E2BIG            EV__ERR(E2BIG)
#else
#   define EV__E2BIG            (-4087)
#endif

#if defined(EBADF) && !defined(_WIN32)
#   define EV__EBADF            EV__ERR(EBADF)
#else
#   define EV__EBADF            (-4085)
#endif

#if defined(EAGAIN) && !defined(_WIN32)
#   define EV__EAGAIN           EV__ERR(EAGAIN)
#else
#   define EV__EAGAIN           (-4083)
#endif

#if defined(ENOMEM) && !defined(_WIN32)
#   define EV__ENOMEM           EV__ERR(ENOMEM)
#else
#   define EV__ENOMEM           (-4082)
#endif

#if defined(EACCES) && !defined(_WIN32)
#   define EV__EACCES           EV__ERR(EACCES)
#else
#   define EV__EACCES           (-4081)
#endif

#if defined(EFAULT) && !defined(_WIN32)
#   define EV__EFAULT           EV__ERR(EFAULT)
#else
#   define EV__EFAULT           (-4080)
#endif

#if defined(EBUSY) && !defined(_WIN32)
#   define EV__EBUSY            EV__ERR(EBUSY)
#else
#   define EV__EBUSY            (-4078)
#endif

#if defined(EEXIST) && !defined(_WIN32)
#   define EV__EEXIST           EV__ERR(EEXIST)
#else
#   define EV__EEXIST           (-4077)
#endif

#if defined(EXDEV) && !defined(_WIN32)
#   define EV__EXDEV            EV__ERR(EXDEV)
#else
#   define EV__EXDEV            (-4076)
#endif

#if defined(ENOTDIR) && !defined(_WIN32)
#   define EV__ENOTDIR          EV__ERR(ENOTDIR)
#else
#   define EV__ENOTDIR          (-4074)
#endif

#if defined(EISDIR) && !defined(_WIN32)
#   define EV__EISDIR           EV__ERR(EISDIR)
#else
#   define EV__EISDIR           (-4073)
#endif

#if defined(EINVAL) && !defined(_WIN32)
#   define EV__EINVAL           EV__ERR(EINVAL)
#else
#   define EV__EINVAL           (-4072)
#endif

#if defined(ENFILE) && !defined(_WIN32)
#   define EV__ENFILE           EV__ERR(ENFILE)
#else
#   define EV__ENFILE           (-4071)
#endif

#if defined(EMFILE) && !defined(_WIN32)
#   define EV__EMFILE           EV__ERR(EMFILE)
#else
#   define EV__EMFILE           (-4070)
#endif

#if defined(ENOSPC) && !defined(_WIN32)
#   define EV__ENOSPC           EV__ERR(ENOSPC)
#else
#   define EV__ENOSPC           (-4066)
#endif

#if defined(EROFS) && !defined(_WIN32)
#   define EV__EROFS            EV__ERR(EROFS)
#else
#   define EV__EROFS            (-4064)
#endif

#if defined(EPIPE) && !defined(_WIN32)
#   define EV__EPIPE            EV__ERR(EPIPE)
#else
#   define EV__EPIPE            (-4062)
#endif

#if defined(ENAMETOOLONG) && !defined(_WIN32)
#   define EV__ENAMETOOLONG     EV__ERR(ENAMETOOLONG)
#else
#   define EV__ENAMETOOLONG     (-4058)
#endif

#if defined(ENOSYS) && !defined(_WIN32)
#   define EV__ENOSYS           EV__ERR(ENOSYS)
#else
#   define EV__ENOSYS           (-4056)
#endif

#if defined(ENOTEMPTY) && !defined(_WIN32)
#   define EV__ENOTEMPTY        EV__ERR(ENOTEMPTY)
#else
#   define EV__ENOTEMPTY        (-4055)
#endif

#if defined(ELOOP) && !defined(_WIN32)
#   define EV__ELOOP            EV__ERR(ELOOP)
#else
#   define EV__ELOOP            (-4054)
#endif

#if defined(EPROTO) && !defined(_WIN32)
#   define EV__EPROTO           EV__ERR(EPROTO)
#else
#   define EV__EPROTO           (-4023)
#endif

#if defined(ENOTSOCK) && !defined(_WIN32)
#   define EV__ENOTSOCK         EV__ERR(ENOTSOCK)
#else
#   define EV__ENOTSOCK         (-4006)
#endif

#if defined(EMSGSIZE) && !defined(_WIN32)
#   define EV__EMSGSIZE         EV__ERR(EMSGSIZE)
#else
#   define EV__EMSGSIZE         (-4004)
#endif

#if defined(EPROTONOSUPPORT) && !defined(_WIN32)
#   define EV__EPROTONOSUPPORT  EV__ERR(EPROTONOSUPPORT)
#else
#   define EV__EPROTONOSUPPORT  (-4001)
#endif

#if defined(ENOTSUP) && !defined(_WIN32)
#   define EV__ENOTSUP          EV__ERR(ENOTSUP)
#else
#   define EV__ENOTSUP          (-3999)
#endif

#if defined(EAFNOSUPPORT) && !defined(_WIN32)
#   define EV__EAFNOSUPPORT     EV__ERR(EAFNOSUPPORT)
#else
#   define EV__EAFNOSUPPORT     (-3997)
#endif

#if defined(EADDRINUSE) && !defined(_WIN32)
#   define EV__EADDRINUSE       EV__ERR(EADDRINUSE)
#else
#   define EV__EADDRINUSE       (-3996)
#endif

#if defined(EADDRNOTAVAIL) && !defined(_WIN32)
#   define EV__EADDRNOTAVAIL    EV__ERR(EADDRNOTAVAIL)
#else
#   define EV__EADDRNOTAVAIL    (-3995)
#endif

#if defined(ENETUNREACH) && !defined(_WIN32)
#   define EV__ENETUNREACH      EV__ERR(ENETUNREACH)
#else
#   define EV__ENETUNREACH      (-3993)
#endif

#if defined(ECONNABORTED) && !defined(_WIN32)
#   define EV__ECONNABORTED     EV__ERR(ECONNABORTED)
#else
#   define EV__ECONNABORTED     (-3991)
#endif

#if defined(ECONNRESET) && !defined(_WIN32)
#   define EV__ECONNRESET       EV__ERR(ECONNRESET)
#else
#   define EV__ECONNRESET       (-3990)
#endif

#if defined(ENOBUFS) && !defined(_WIN32)
#   define EV__ENOBUFS          EV__ERR(ENOBUFS)
#else
#   define EV__ENOBUFS          (-3989)
#endif

#if defined(EISCONN) && !defined(_WIN32)
#   define EV__EISCONN          EV__ERR(EISCONN)
#else
#   define EV__EISCONN          (-3988)
#endif

#if defined(ENOTCONN) && !defined(_WIN32)
#   define EV__ENOTCONN         EV__ERR(ENOTCONN)
#else
#   define EV__ENOTCONN         (-3987)
#endif

#if defined(ETIMEDOUT) && !defined(_WIN32)
#   define EV__ETIMEDOUT        EV__ERR(ETIMEDOUT)
#else
#   define EV__ETIMEDOUT        (-3984)
#endif

#if defined(ECONNREFUSED) && !defined(_WIN32)
#   define EV__ECONNREFUSED     EV__ERR(ECONNREFUSED)
#else
#   define EV__ECONNREFUSED     (-3983)
#endif

#if defined(EHOSTUNREACH) && !defined(_WIN32)
#   define EV__EHOSTUNREACH     EV__ERR(EHOSTUNREACH)
#else
#   define EV__EHOSTUNREACH     (-3981)
#endif

#if defined(EALREADY) && !defined(_WIN32)
#   define EV__EALREADY         EV__ERR(EALREADY)
#else
#   define EV__EALREADY         (-3980)
#endif

#if defined(EINPROGRESS) && !defined(_WIN32)
#   define EV__EINPROGRESS      EV__ERR(EINPROGRESS)
#else
#   define EV__EINPROGRESS      (-3979)
#endif

#if defined(ECANCELED) && !defined(_WIN32)
#   define EV__ECANCELED        EV__ERR(ECANCELED)
#else
#   define EV__ECANCELED        (-3969)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ERRNO Error number
 * @{
 */

/**
 * @brief Error number
 */
typedef enum ev_errno
{
    /* POSIX compatible error code */
    EV_EPERM            = EV__EPERM,            /**< Operation not permitted (POSIX.1-2001) */
    EV_ENOENT           = EV__ENOENT,           /**< No such file or directory (POSIX.1-2001) */
    EV_EIO              = EV__EIO,              /**< Host is unreachable (POSIX.1-2001) */
    EV_E2BIG            = EV__E2BIG,            /**< Argument list too long (POSIX.1-2001) */
    EV_EBADF            = EV__EBADF,            /**< Bad file descriptor (POSIX.1-2001) */
    EV_EAGAIN           = EV__EAGAIN,           /**< Resource temporarily unavailable (POSIX.1-2001) */
    EV_ENOMEM           = EV__ENOMEM,           /**< Not enough space/cannot allocate memory (POSIX.1-2001) */
    EV_EACCES           = EV__EACCES,           /**< Permission denied (POSIX.1-2001) */
    EV_EFAULT           = EV__EFAULT,           /**< Bad address (POSIX.1-2001) */
    EV_EBUSY            = EV__EBUSY,            /**< Device or resource busy (POSIX.1-2001) */
    EV_EEXIST           = EV__EEXIST,           /**< File exists (POSIX.1-2001) */
    EV_EXDEV            = EV__EXDEV,            /**< Improper link (POSIX.1-2001) */
    EV_ENOTDIR          = EV__ENOTDIR,          /**< Not a directory (POSIX.1-2001) */
    EV_EISDIR           = EV__EISDIR,           /**< Is a directory (POSIX.1-2001) */
    EV_EINVAL           = EV__EINVAL,           /**< Invalid argument (POSIX.1-2001) */
    EV_ENFILE           = EV__ENFILE,           /**< Too many open files in system (POSIX.1-2001) */
    EV_EMFILE           = EV__EMFILE,           /**< Too many open files (POSIX.1-2001) */
    EV_ENOSPC           = EV__ENOSPC,           /**< No space left on device (POSIX.1-2001) */
    EV_EROFS            = EV__EROFS,            /**< Read-only filesystem (POSIX.1-2001) */
    EV_EPIPE            = EV__EPIPE,            /**< Broken pipe (POSIX.1-2001) */
    EV_ENAMETOOLONG     = EV__ENAMETOOLONG,     /**< Filename too long (POSIX.1-2001) */
    EV_ENOSYS           = EV__ENOSYS,           /**< Function not implemented (POSIX.1-2001) */
    EV_ENOTEMPTY        = EV__ENOTEMPTY,        /**< Directory not empty (POSIX.1-2001) */
    EV_ELOOP            = EV__ELOOP,            /**< Too many levels of symbolic links (POSIX.1-2001) */
    EV_EPROTO           = EV__EPROTO,           /**< Protocol error (POSIX.1-2001) */
    EV_ENOTSOCK         = EV__ENOTSOCK,         /**< Not a socket (POSIX.1-2001) */
    EV_EMSGSIZE         = EV__EMSGSIZE,         /**< Message too long (POSIX.1-2001) */
    EV_EPROTONOSUPPORT  = EV__EPROTONOSUPPORT,  /**< Protocol not supported (POSIX.1-2001) */
    EV_ENOTSUP          = EV__ENOTSUP,          /**< Operation not supported (POSIX.1-2001) */
    EV_EAFNOSUPPORT     = EV__EAFNOSUPPORT,     /**< Address family not supported (POSIX.1-2001) */
    EV_EADDRINUSE       = EV__EADDRINUSE,       /**< Address already in use (POSIX.1-2001) */
    EV_EADDRNOTAVAIL    = EV__EADDRNOTAVAIL,    /**< Address not available (POSIX.1-2001) */
    EV_ENETUNREACH      = EV__ENETUNREACH,      /**< Network unreachable (POSIX.1-2001) */
    EV_ECONNABORTED     = EV__ECONNABORTED,     /**< Connection aborted (POSIX.1-2001) */
    EV_ECONNRESET       = EV__ECONNRESET,       /**< Connection reset (POSIX.1-2001) */
    EV_ENOBUFS          = EV__ENOBUFS,          /**< No buffer space available (POSIX.1 (XSI STREAMS option)) */
    EV_EISCONN          = EV__EISCONN,          /**< Socket is connected (POSIX.1-2001) */
    EV_ENOTCONN         = EV__ENOTCONN,         /**< The socket is not connected (POSIX.1-2001) */
    EV_ETIMEDOUT        = EV__ETIMEDOUT,        /**< Operation timed out (POSIX.1-2001) */
    EV_ECONNREFUSED     = EV__ECONNREFUSED,     /**< Connection refused (POSIX.1-2001) */
    EV_EHOSTUNREACH     = EV__EHOSTUNREACH,     /**< Host is unreachable (POSIX.1-2001) */
    EV_EALREADY         = EV__EALREADY,         /**< Connection already in progress (POSIX.1-2001) */
    EV_EINPROGRESS      = EV__EINPROGRESS,      /**< Operation in progress (POSIX.1-2001) */
    EV_ECANCELED        = EV__ECANCELED,        /**< Operation canceled (POSIX.1-2001) */

    /* Extend error code */
    EV_EOF              = -4095,
    EV_EUNKNOWN         = -4094,
} ev_errno_t;

/**
 * @brief Describe the error code
 * @param[in] err   Error code
 * @return          Describe string
 */
EV_API const char* ev_strerror(int err);

/**
 * @} EV_ERRNO
 */

#ifdef __cplusplus
}
#endif
#endif
