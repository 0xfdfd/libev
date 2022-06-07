#include "ev/errno.h"
#include "allocator.h"
#include "winapi.h"
#include "misc_win.h"
#include <assert.h>

ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src)
{
    int errcode;
    int pathw_len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (pathw_len == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    size_t buf_sz = pathw_len * sizeof(WCHAR);
    WCHAR* buf = ev__malloc(buf_sz);
    if (buf == NULL)
    {
        return EV_ENOMEM;
    }

    int r = MultiByteToWideChar(CP_UTF8, 0, src, -1, buf, pathw_len);
    assert(r == pathw_len);

    *dst = buf;

    return r;
}

ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src)
{
    int errcode;
    int target_len = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0,
                                         NULL, NULL);
    if (target_len == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    char* buf = ev__malloc(target_len);
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

int ev__ntstatus_to_winsock_error(NTSTATUS status)
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
                /* It's a windows error that has been previously mapped to an ntstatus
                 * code. */
                return (DWORD)(status & 0xffff);
            }
            else
            {
                /* The default fallback for unmappable ntstatus codes. */
                return WSAEINVAL;
            }
    }
}

int ev__translate_sys_error(int err)
{
    switch (err)
    {
        case 0:                                 return EV_SUCCESS;
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
        default:                                BREAK_ABORT(); return err;
    }
}

void ev_library_shutdown(void)
{

}
