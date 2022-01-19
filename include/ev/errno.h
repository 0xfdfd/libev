#ifndef __EV_ERRNO_H__
#define __EV_ERRNO_H__
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
    EV_SUCCESS          =  0,                   /**< Operation success */

    /* POSIX compatible error code */
    EV_EPERM            = -1,                   /**< Operation not permitted (POSIX.1-2001) */
    EV_ENOENT           = -2,                   /**< No such file or directory (POSIX.1-2001) */
    EV_EIO              = -5,                   /**< Host is unreachable (POSIX.1-2001) */
    EV_E2BIG            = -7,                   /**< Argument list too long (POSIX.1-2001) */
    EV_EBADF            = -9,                   /**< Bad file descriptor (POSIX.1-2001) */
    EV_EAGAIN           = -11,                  /**< Resource temporarily unavailable (POSIX.1-2001) */
    EV_ENOMEM           = -12,                  /**< Not enough space/cannot allocate memory (POSIX.1-2001) */
    EV_EACCES           = -13,                  /**< Permission denied (POSIX.1-2001) */
    EV_EFAULT           = -14,                  /**< Bad address (POSIX.1-2001) */
    EV_EBUSY            = -16,                  /**< Device or resource busy (POSIX.1-2001) */
    EV_EEXIST           = -17,                  /**< File exists (POSIX.1-2001) */
    EV_EXDEV            = -18,                  /**< Improper link (POSIX.1-2001) */
    EV_EISDIR           = -21,                  /**< Is a directory (POSIX.1-2001) */
    EV_EINVAL           = -22,                  /**< Invalid argument (POSIX.1-2001) */
    EV_EMFILE           = -24,                  /**< Too many open files (POSIX.1-2001) */
    EV_ENOSPC           = -28,                  /**< No space left on device (POSIX.1-2001) */
    EV_EROFS            = -30,                  /**< Read-only filesystem (POSIX.1-2001) */
    EV_EPIPE            = -32,                  /**< Broken pipe (POSIX.1-2001) */
    EV_ENAMETOOLONG     = -38,                  /**< Filename too long (POSIX.1-2001) */
    EV_ENOTEMPTY        = -41,                  /**< Directory not empty (POSIX.1-2001) */
    EV_EADDRINUSE       = -100,                 /**< Address already in use (POSIX.1-2001) */
    EV_EADDRNOTAVAIL    = -101,                 /**< Address not available (POSIX.1-2001) */
    EV_EAFNOSUPPORT     = -102,                 /**< Address family not supported (POSIX.1-2001) */
    EV_EALREADY         = -103,                 /**< Connection already in progress (POSIX.1-2001) */
    EV_ECANCELED        = -105,                 /**< Operation canceled (POSIX.1-2001) */
    EV_ECONNABORTED     = -106,                 /**< Connection aborted (POSIX.1-2001) */
    EV_ECONNREFUSED     = -107,                 /**< Connection refused (POSIX.1-2001) */
    EV_ECONNRESET       = -108,                 /**< Connection reset (POSIX.1-2001) */
    EV_EHOSTUNREACH     = -110,                 /**< Host is unreachable (POSIX.1-2001) */
    EV_EINPROGRESS      = -112,                 /**< Operation in progress (POSIX.1-2001) */
    EV_EISCONN          = -113,                 /**< Socket is connected (POSIX.1-2001) */
    EV_ELOOP            = -114,                 /**< Too many levels of symbolic links (POSIX.1-2001) */
    EV_EMSGSIZE         = -115,                 /**< Message too long (POSIX.1-2001) */
    EV_ENETUNREACH      = -118,                 /**< Network unreachable (POSIX.1-2001) */
    EV_ENOBUFS          = -119,                 /**< No buffer space available (POSIX.1 (XSI STREAMS option)) */
    EV_ENOTCONN         = -126,                 /**< The socket is not connected (POSIX.1-2001) */
    EV_ENOTSOCK         = -128,                 /**< Not a socket (POSIX.1-2001) */
    EV_ENOTSUP          = -129,                 /**< Operation not supported (POSIX.1-2001) */
    EV_EPROTO           = -134,                 /**< Protocol error (POSIX.1-2001) */
    EV_EPROTONOSUPPORT  = -135,                 /**< Protocol not supported (POSIX.1-2001) */
    EV_ETIMEDOUT        = -138,                 /**< Operation timed out (POSIX.1-2001) */

    /* Extend error code */
    EV_UNKNOWN          = -1001,                /**< Unknown error */
    EV_EOF              = -1002,                /**< End of file */
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

#ifdef __cplusplus
}
#endif
#endif
