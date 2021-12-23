#ifndef __EV_OS_WIN_H__
#define __EV_OS_WIN_H__

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef HANDLE                  ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      INVALID_HANDLE_VALUE

typedef SOCKET                  ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    INVALID_SOCKET

typedef HANDLE                  ev_os_thread_t;
#define EV_OS_THREAD_INVALID    NULL

typedef CRITICAL_SECTION        ev_os_mutex_t;
typedef HANDLE                  ev_os_sem_t;

#ifdef __cplusplus
}
#endif

#endif
