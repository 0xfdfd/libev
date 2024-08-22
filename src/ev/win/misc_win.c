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
    case ERROR_PROC_NOT_FOUND:              return EV_ENOENT;
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

size_t ev_os_page_size(void)
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwPageSize;
}

size_t ev_os_mmap_offset_granularity(void)
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwAllocationGranularity;
}
