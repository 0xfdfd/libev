#ifndef __EV_OS_UNIX_H__
#define __EV_OS_UNIX_H__

#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#if defined(O_APPEND)
#   define EV_FS_O_APPEND       O_APPEND
#else
#   define EV_FS_O_APPEND       0
#endif

#if defined(O_CREAT)
#   define EV_FS_O_CREAT        O_CREAT
#else
#   define EV_FS_O_CREAT        0
#endif

#if defined(O_DSYNC)
#   define EV_FS_O_DSYNC        O_DSYNC
#else
#   define EV_FS_O_DSYNC        0
#endif

#if defined(O_EXCL)
#   define EV_FS_O_EXCL         O_EXCL
#else
#   define EV_FS_O_EXCL         0
#endif

#if defined(O_SYNC)
#   define EV_FS_O_SYNC         O_SYNC
#else
#   define EV_FS_O_SYNC         0
#endif

#if defined(O_TRUNC)
#   define EV_FS_O_TRUNC        O_TRUNC
#else
#   define EV_FS_O_TRUNC        0
#endif

#if defined(O_RDONLY)
#   define EV_FS_O_RDONLY       O_RDONLY
#else
#   define EV_FS_O_RDONLY       0
#endif

#if defined(O_WRONLY)
#   define EV_FS_O_WRONLY       O_WRONLY
#else
#   define EV_FS_O_WRONLY       0
#endif

#if defined(O_RDWR)
#   define EV_FS_O_RDWR         O_RDWR
#else
#   define EV_FS_O_RDWR         0
#endif

#define EV_FS_S_IRUSR           S_IRUSR
#define EV_FS_S_IWUSR           S_IWUSR
#define EV_FS_S_IXUSR           S_IXUSR
#define EV_FS_S_IRWXU           S_IRWXU

#define EV_FS_SEEK_BEG			SEEK_SET
#define EV_FS_SEEK_CUR			SEEK_CUR
#define EV_FS_SEEK_END			SEEK_END

#ifdef __cplusplus
extern "C" {
#endif

typedef pid_t                   ev_os_pid_t;
#define EV_OS_PID_INVALID       ((pid_t)-1)

typedef int                     ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      (-1)

typedef int                     ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    (-1)

typedef pid_t                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((pid_t)(-1))

typedef int                     ev_os_file_t;
#define EV_OS_FILE_INVALID      (-1)

typedef pthread_t               ev_os_thread_t;
#define EV_OS_THREAD_INVALID    ((pthread_t)(-1))

typedef pthread_key_t           ev_os_tls_t;
typedef pthread_mutex_t         ev_os_mutex_t;
typedef sem_t                   ev_os_sem_t;

#ifdef __cplusplus
}
#endif

#endif
