#ifndef __EV_OS_WIN_H__
#define __EV_OS_WIN_H__

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fcntl.h>
#include <sys/stat.h>

#if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#   define SSIZE_MAX INTPTR_MAX
#   define _SSIZE_T_
#   define _SSIZE_T_DEFINED
#endif

#define EV_FS_O_APPEND          _O_APPEND
#define EV_FS_O_CREAT           _O_CREAT
#define EV_FS_O_DSYNC           FILE_FLAG_WRITE_THROUGH
#define EV_FS_O_EXCL            _O_EXCL
#define EV_FS_O_SYNC            FILE_FLAG_WRITE_THROUGH
#define EV_FS_O_TRUNC           _O_TRUNC
#define EV_FS_O_RDONLY          _O_RDONLY
#define EV_FS_O_WRONLY          _O_WRONLY
#define EV_FS_O_RDWR            _O_RDWR

#define EV_FS_S_IRUSR           _S_IREAD
#define EV_FS_S_IWUSR           _S_IWRITE
#define EV_FS_S_IXUSR           _S_IEXEC
#define EV_FS_S_IRWXU           (EV_FS_S_IRUSR | EV_FS_S_IWUSR | EV_FS_S_IXUSR)

#ifdef __cplusplus
extern "C" {
#endif

typedef HANDLE                  ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      INVALID_HANDLE_VALUE

typedef SOCKET                  ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    INVALID_SOCKET

typedef DWORD                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((DWORD)(-1))

typedef HANDLE                  ev_os_file_t;
#define EV_OS_FILE_INVALID      INVALID_HANDLE_VALUE

typedef HANDLE                  ev_os_thread_t;
typedef DWORD                   ev_os_tls_t;
typedef CRITICAL_SECTION        ev_os_mutex_t;
typedef HANDLE                  ev_os_sem_t;

#ifdef __cplusplus
}
#endif

#endif
