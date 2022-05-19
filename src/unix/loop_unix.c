#include "loop_unix.h"
#include "io_unix.h"
#include "work.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/eventfd.h>

#if defined(__PASE__)
/* on IBMi PASE the control message length can not exceed 256. */
#   define EV__CMSG_FD_COUNT 60
#else
#   define EV__CMSG_FD_COUNT 64
#endif
#define EV__CMSG_FD_SIZE (EV__CMSG_FD_COUNT * sizeof(int))

ev_loop_unix_ctx_t g_ev_loop_unix_ctx;

static void _ev_init_hwtime(void)
{
    struct timespec t;
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &t) != 0)
    {
        goto err;
    }
    if (t.tv_nsec > 1 * 1000 * 1000)
    {
        goto err;
    }
    g_ev_loop_unix_ctx.hwtime_clock_id = CLOCK_MONOTONIC_COARSE;
    return;

err:
    g_ev_loop_unix_ctx.hwtime_clock_id = CLOCK_MONOTONIC;
}

static ev_nonblock_io_t* _ev_find_io(ev_loop_t* loop, int fd)
{
    ev_nonblock_io_t tmp;
    tmp.data.fd = fd;

    ev_map_node_t* it = ev_map_find(&loop->backend.io, &tmp.node);
    return it != NULL ? EV_CONTAINER_OF(it, ev_nonblock_io_t, node) : NULL;
}

static int _ev_poll_once(ev_loop_t* loop, struct epoll_event* events, int maxevents, int timeout)
{
    int nfds = epoll_wait(loop->backend.pollfd, events, maxevents, timeout);
    if (nfds < 0)
    {
        return nfds;
    }

    int i;
    for (i = 0; i < nfds; i++)
    {
        ev_nonblock_io_t* io = _ev_find_io(loop, events[i].data.fd);
        io->data.cb(io, events[i].events, io->data.arg);
    }

    return nfds;
}

static void _ev_init_iovmax(void)
{
#if defined(IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = IOV_MAX;
#elif defined(__IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = __IOV_MAX;
#elif defined(_SC_IOV_MAX)
    g_ev_loop_unix_ctx.iovmax = sysconf(_SC_IOV_MAX);
    if (g_ev_loop_unix_ctx.iovmax == -1)
    {
        g_ev_loop_unix_ctx.iovmax = 1;
    }
#else
    g_ev_loop_unix_ctx.iovmax = EV_IOV_MAX;
#endif
}

static void _ev_check_layout_unix(void)
{
    ENSURE_LAYOUT(ev_buf_t, data, size, struct iovec, iov_base, iov_len);
}

static void _ev_init_once_unix(void)
{
    _ev_check_layout_unix();
    _ev_init_hwtime();
    _ev_init_iovmax();
    ev__init_process_unix();
}

uint64_t ev__clocktime(void)
{
    struct timespec t;
    if (clock_gettime(g_ev_loop_unix_ctx.hwtime_clock_id, &t) != 0)
    {
        BREAK_ABORT();
    }

    return t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
}

void ev__init_once_unix(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_unix);
}

int ev__loop_init_backend(ev_loop_t* loop)
{
    ev__init_once_unix();
    ev__init_io(loop);
    ev__init_work(loop);

    return EV_SUCCESS;
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
    ev__exit_work(loop);
    ev__exit_io(loop);
}

void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
    int nevts;
    struct epoll_event events[128];

    /**
     * A bug in kernels < 2.6.37 makes timeouts larger than ~30 minutes
     * effectively infinite on 32 bits architectures.  To avoid blocking
     * indefinitely, we cap the timeout and poll again if necessary.
     *
     * Note that "30 minutes" is a simplification because it depends on
     * the value of CONFIG_HZ.  The magic constant assumes CONFIG_HZ=1200,
     * that being the largest value I have seen in the wild (and only once.)
     */
    const uint32_t max_safe_timeout = 1789569;

    /**
     * from libuv, this value gives the best throughput.
     */
    int max_performance_events = 49152;

    const uint64_t base_time = loop->hwtime;
    const uint32_t user_timeout = timeout;
    for (; max_performance_events != 0; max_performance_events--)
    {
        if (timeout > max_safe_timeout)
        {
            timeout = max_safe_timeout;
        }

        nevts = _ev_poll_once(loop, events, ARRAY_SIZE(events), timeout);

        if (nevts == ARRAY_SIZE(events))
        {/* Poll for more events but don't block this time. */
            timeout = 0;
            continue;
        }

        if (nevts >= 0)
        {
            break;
        }

        /* If errno is not EINTR, something must wrong in the program */
        if (errno != EINTR)
        {
            BREAK_ABORT();
        }

        ev__loop_update_time(loop);
        uint64_t pass_time = loop->hwtime - base_time;
        if (pass_time >= user_timeout)
        {
            break;
        }

        timeout = user_timeout - pass_time;
    }
}

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
