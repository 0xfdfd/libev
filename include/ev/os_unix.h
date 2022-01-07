#ifndef __EV_OS_UNIX_H__
#define __EV_OS_UNIX_H__

#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int                     ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      (-1)

typedef int                     ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    (-1)

typedef pid_t                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((pid_t)(-1))

typedef pthread_t               ev_os_thread_t;
typedef pthread_key_t           ev_os_tls_t;
typedef pthread_mutex_t         ev_os_mutex_t;
typedef sem_t                   ev_os_sem_t;

#ifdef __cplusplus
}
#endif

#endif
