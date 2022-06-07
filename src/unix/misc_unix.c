#include "ev/errno.h"
#include "misc_unix.h"
#include <errno.h>

int ev__translate_sys_error(int syserr)
{
    switch (syserr) {
        /* Success */
        case 0:                 return EV_SUCCESS;
            /* Posix */
        case EPERM:             return EV_EPERM;
        case ENOENT:            return EV_ENOENT;
        case EIO:               return EV_EIO;
        case E2BIG:             return EV_E2BIG;
        case EBADF:             return EV_EBADF;
        case EAGAIN:            return EV_EAGAIN;
        case ENOMEM:            return EV_ENOMEM;
        case EACCES:            return EV_EACCES;
        case EFAULT:            return EV_EFAULT;
        case EBUSY:             return EV_EBUSY;
        case EEXIST:            return EV_EEXIST;
        case EXDEV:             return EV_EXDEV;
        case ENOTDIR:           return EV_ENOTDIR;
        case EISDIR:            return EV_EISDIR;
        case EINVAL:            return EV_EINVAL;
        case EMFILE:            return EV_EMFILE;
        case ENOSPC:            return EV_ENOSPC;
        case EROFS:             return EV_EROFS;
        case EPIPE:             return EV_EPIPE;
        case ENAMETOOLONG:      return EV_ENAMETOOLONG;
        case ENOTEMPTY:         return EV_ENOTEMPTY;
        case EADDRINUSE:        return EV_EADDRINUSE;
        case EADDRNOTAVAIL:     return EV_EADDRNOTAVAIL;
        case EAFNOSUPPORT:      return EV_EAFNOSUPPORT;
        case EALREADY:          return EV_EALREADY;
        case ECANCELED:         return EV_ECANCELED;
        case ECONNABORTED:      return EV_ECONNABORTED;
        case ECONNREFUSED:      return EV_ECONNREFUSED;
        case ECONNRESET:        return EV_ECONNRESET;
        case EHOSTUNREACH:      return EV_EHOSTUNREACH;
        case EINPROGRESS:       return EV_EINPROGRESS;
        case EISCONN:           return EV_EISCONN;
        case ELOOP:             return EV_ELOOP;
        case EMSGSIZE:          return EV_EMSGSIZE;
        case ENETUNREACH:       return EV_ENETUNREACH;
        case ENOBUFS:           return EV_ENOBUFS;
        case ENOTCONN:          return EV_ENOTCONN;
        case ENOTSOCK:          return EV_ENOTSOCK;
        case ENOTSUP:           return EV_ENOTSUP;
        case EPROTO:            return EV_EPROTO;
        case EPROTONOSUPPORT:   return EV_EPROTONOSUPPORT;
        case ETIMEDOUT:         return EV_ETIMEDOUT;
#if EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:       return EV_EAGAIN;
#endif
            /* Unknown */
        default:                break;
    }

    return syserr;
}

void ev_library_shutdown(void)
{
    // Do nothing
}
