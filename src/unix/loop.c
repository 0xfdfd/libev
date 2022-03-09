#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "ev-platform.h"
#include "ev.h"
#include "unix/loop.h"

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

static int _ev_cmp_io_unix(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_nonblock_io_t* io1 = container_of(key1, ev_nonblock_io_t, node);
    ev_nonblock_io_t* io2 = container_of(key2, ev_nonblock_io_t, node);
    return io1->data.fd - io2->data.fd;
}

static ev_nonblock_io_t* _ev_find_io(ev_loop_t* loop, int fd)
{
    ev_nonblock_io_t tmp;
    tmp.data.fd = fd;

    ev_map_node_t* it = ev_map_find(&loop->backend.io, &tmp.node);
    return it != NULL ? container_of(it, ev_nonblock_io_t, node) : NULL;
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

ssize_t ev__writev_unix(int fd, ev_buf_t* iov, int iovcnt)
{
    ssize_t write_size;
    do
    {
        write_size = writev(fd, (struct iovec*)iov, iovcnt);
    } while (write_size == -1 && errno == EINTR);

    if (write_size >= 0)
    {
        return write_size;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

static ssize_t _ev_stream_do_write_writev_unix(int fd, struct iovec* iov, int iovcnt, void* arg)
{
    (void)arg;
    return ev__writev_unix(fd, (ev_buf_t*)iov, iovcnt);
}

static int _ev_stream_do_write_once(ev_nonblock_stream_t* stream, ev_write_t* req)
{
    return ev__send_unix(stream->io.data.fd, req, _ev_stream_do_write_writev_unix, NULL);
}

static void _ev_stream_do_write(ev_nonblock_stream_t* stream)
{
    int ret;
    ev_list_node_t* it;
    ev_write_t* req;

    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        if ((ret = _ev_stream_do_write_once(stream, req)) == EV_SUCCESS)
        {
            stream->callbacks.w_cb(stream, req, req->data.size, EV_SUCCESS);
            continue;
        }

        /* Unsuccess operation should restore list */
        ev_list_push_front(&stream->pending.w_queue, it);

        if (ret == EV_EAGAIN)
        {
            break;
        }
        goto err;
    }

    return;

err:
    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        stream->callbacks.w_cb(stream, req, req->data.size, ret);
    }
}

ssize_t ev__readv_unix(int fd, ev_buf_t* iov, int iovcnt)
{
    ssize_t read_size;
    do
    {
        read_size = readv(fd, (struct iovec*)iov, iovcnt);
    } while (read_size == -1 && errno == EINTR);

    if (read_size > 0)
    {
        return read_size;
    }
    else if (read_size == 0)
    {
        return EV_EOF;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

static int _ev_stream_do_read_once(ev_nonblock_stream_t* stream, ev_read_t* req, size_t* size)
{
    int iovcnt = req->data.nbuf;
    if (iovcnt > g_ev_loop_unix_ctx.iovmax)
    {
        iovcnt = g_ev_loop_unix_ctx.iovmax;
    }

    ssize_t read_size;
    read_size = ev__readv_unix(stream->io.data.fd, req->data.bufs, iovcnt);
    if (read_size >= 0)
    {
        *size = read_size;
        return EV_SUCCESS;
    }
    return read_size;
}

static void _ev_stream_do_read(ev_nonblock_stream_t* stream)
{
    int ret;
    ev_list_node_t* it = ev_list_pop_front(&stream->pending.r_queue);
    ev_read_t* req = container_of(it, ev_read_t, node);

    size_t r_size = 0;
    ret = _ev_stream_do_read_once(stream, req, &r_size);
    req->data.size += r_size;

    if (ret == EV_SUCCESS)
    {
        stream->callbacks.r_cb(stream, req, req->data.size, EV_SUCCESS);
        return;
    }

    ev_list_push_front(&stream->pending.r_queue, it);

    if (ret == EV_EAGAIN)
    {
        return;
    }

    /* If error, cleanup all pending read requests */
    while ((it = ev_list_pop_front(&stream->pending.r_queue)) != NULL)
    {
        req = container_of(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, 0, ret);
    }
}

static void _ev_stream_cleanup_r(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.r_queue)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, 0, errcode);
    }
}

static void _ev_stream_cleanup_w(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        stream->callbacks.w_cb(stream, req, req->data.size, errcode);
    }
}

static void _ev_nonblock_stream_on_io(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;
    ev_nonblock_stream_t* stream = container_of(io, ev_nonblock_stream_t, io);

    if (evts & EPOLLOUT)
    {
        _ev_stream_do_write(stream);
        if (ev_list_size(&stream->pending.w_queue) == 0)
        {
            ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_OUT);
            stream->flags.io_reg_w = 0;
        }
    }

    else if (evts & (EPOLLIN | EPOLLHUP))
    {
        _ev_stream_do_read(stream);
        if (ev_list_size(&stream->pending.r_queue) == 0)
        {
            ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_IN);
            stream->flags.io_reg_r = 0;
        }
    }
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
}

static void _ev_wakeup_clear_eventfd(ev_loop_t* loop)
{
    uint64_t cnt = 0;

    for (;;)
    {
        ssize_t read_size = read(loop->backend.wakeup.fd, &cnt, sizeof(cnt));
        if (read_size >= 0)
        {
            continue;
        }

        int err = errno;
        if (err == EINTR)
        {
            continue;
        }

        break;
    }
}

static void _ev_loop_on_wakeup_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts;
    ev_loop_t* loop = container_of(io, ev_loop_t, backend.wakeup.io);
    ev_loop_on_wakeup_cb wakeup_cb = arg;

    _ev_wakeup_clear_eventfd(loop);
    wakeup_cb(loop);
}

/**
 * @brief Initialize #ev_loop_t::backend::wakeup
 * @param[out] loop     Event loop
 * @return              #ev_errno_t
 */
static int _ev_wakeup_init_loop_unix(ev_loop_t* loop, ev_loop_on_wakeup_cb wakeup_cb)
{
    int err;
    loop->backend.wakeup.fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (loop->backend.wakeup.fd < 0)
    {
        err = errno;
        return ev__translate_sys_error(err);
    }

    ev__nonblock_io_init(&loop->backend.wakeup.io, loop->backend.wakeup.fd, _ev_loop_on_wakeup_unix, wakeup_cb);
    ev__nonblock_io_add(loop, &loop->backend.wakeup.io, EPOLLIN);

    return EV_SUCCESS;
}

/**
 * @brief Destroy #ev_loop_t::backend::wakeup
 * @param[in] loop  Event loop
 */
static void _ev_wakeup_exit_loop_unix(ev_loop_t* loop)
{
    ev__nonblock_io_del(loop, &loop->backend.wakeup.io, EPOLLIN);
    if (loop->backend.wakeup.fd != -1)
    {
        close(loop->backend.wakeup.fd);
        loop->backend.wakeup.fd = -1;
    }
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

int ev__cloexec(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
    int r;

    do
    {
        r = ioctl(fd, set ? FIOCLEX : FIONCLEX);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return EV_SUCCESS;
#else
    int flags;

    int r = ev__fcntl_getfd_unix(fd);
    if (r == -1)
    {
        return errno;
    }

    /* Bail out now if already set/clear. */
    if (!!(r & FD_CLOEXEC) == !!set)
    {
        return EV_SUCCESS;
    }

    if (set)
    {
        flags = r | FD_CLOEXEC;
    }
    else
    {
        flags = r & ~FD_CLOEXEC;
    }

    do
    {
        r = fcntl(fd, F_SETFD, flags);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return EV_SUCCESS;
#endif
}

int ev__nonblock(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
    int r;

    do
    {
        r = ioctl(fd, FIONBIO, &set);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return EV_SUCCESS;
#else
    int flags;

    int r = ev__fcntl_getfl_unix(fd);
    if (r == -1)
    {
        return ev__translate_sys_error(errno);
    }

    /* Bail out now if already set/clear. */
    if (!!(r & O_NONBLOCK) == !!set)
    {
        return EV_SUCCESS;
    }

    if (set)
    {
        flags = r | O_NONBLOCK;
    }
    else
    {
        flags = r & ~O_NONBLOCK;
    }

    do
    {
        r = fcntl(fd, F_SETFL, flags);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        return ev__translate_sys_error(errno);
    }

    return EV_SUCCESS;
#endif
}

int ev__reuse_unix(int fd)
{
    int yes;
    yes = 1;

#if defined(SO_REUSEPORT) && defined(__MVS__)
    struct sockaddr_in sockfd;
    unsigned int sockfd_len = sizeof(sockfd);
    if (getsockname(fd, (struct sockaddr*)&sockfd, &sockfd_len) == -1)
    {
        goto err;
    }
    if (sockfd.sin_family == AF_UNIX)
    {
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
        {
            goto err;
        }
    }
    else
    {
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)))
        {
            goto err;
        }
    }
#elif defined(SO_REUSEPORT) && !defined(__linux__)
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)))
    {
        goto err;
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
    {
        goto err;
    }
#endif

    return 0;

err:
    yes = errno;
    return ev__translate_sys_error(yes);
}

int ev__fcntl_getfl_unix(int fd)
{
    int mode;
    do
    {
        mode = fcntl(fd, F_GETFL);
    } while (mode == -1 && errno == EINTR);
    return mode;
}

int ev__fcntl_getfd_unix(int fd)
{
    int flags;

    do
    {
        flags = fcntl(fd, F_GETFD);
    } while (flags == -1 && errno == EINTR);

    return flags;
}

void ev__init_once_unix(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_unix);
}

int ev__loop_init_backend(ev_loop_t* loop, ev_loop_on_wakeup_cb wakeup_cb)
{
    int err = EV_SUCCESS;
    ev__init_once_unix();

    ev_map_init(&loop->backend.io, _ev_cmp_io_unix, NULL);

    if ((loop->backend.pollfd = epoll_create(256)) == -1)
    {
        return ev__translate_sys_error(errno);
    }

    if ((err = ev__cloexec(loop->backend.pollfd, 1)) != 0)
    {
        goto err_cloexec;
    }

    if ((err = _ev_wakeup_init_loop_unix(loop, wakeup_cb)) != EV_SUCCESS)
    {
        goto err_cloexec;
    }

    return EV_SUCCESS;

err_cloexec:
    close(loop->backend.pollfd);
    loop->backend.pollfd = -1;
    return err;
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
    _ev_wakeup_exit_loop_unix(loop);

    if (loop->backend.pollfd != -1)
    {
        close(loop->backend.pollfd);
        loop->backend.pollfd = -1;
    }
}

void ev__nonblock_io_init(ev_nonblock_io_t* io, int fd, ev_nonblock_io_cb cb, void* arg)
{
    io->data.fd = fd;
    io->data.c_events = 0;
    io->data.n_events = 0;
    io->data.cb = cb;
    io->data.arg = arg;
}

void ev__nonblock_io_add(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts)
{
    struct epoll_event poll_event;

    io->data.n_events |= evts;
    if (io->data.n_events == io->data.c_events)
    {
        return;
    }

    memset(&poll_event, 0, sizeof(poll_event));
    poll_event.events = io->data.n_events;
    poll_event.data.fd = io->data.fd;

    int op = io->data.c_events == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
    {
        BREAK_ABORT();
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_ADD)
    {
        ev_map_insert(&loop->backend.io, &io->node);
    }
}

void ev__nonblock_io_del(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts)
{
    struct epoll_event poll_event;
    io->data.n_events &= ~evts;
    if (io->data.n_events == io->data.c_events)
    {
        return;
    }

    memset(&poll_event, 0, sizeof(poll_event));
    poll_event.events = io->data.n_events;
    poll_event.data.fd = io->data.fd;

    int op = io->data.n_events == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
    {
        BREAK_ABORT();
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_DEL)
    {
        ev_map_erase(&loop->backend.io, &io->node);
    }
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

void ev__nonblock_stream_init(ev_loop_t* loop, ev_nonblock_stream_t* stream,
    int fd, ev_stream_write_cb wcb, ev_stream_read_cb rcb)
{
    stream->loop = loop;

    stream->flags.io_abort = 0;
    stream->flags.io_reg_r = 0;
    stream->flags.io_reg_w = 0;

    ev__nonblock_io_init(&stream->io, fd, _ev_nonblock_stream_on_io, NULL);

    ev_list_init(&stream->pending.w_queue);
    ev_list_init(&stream->pending.r_queue);

    stream->callbacks.w_cb = wcb;
    stream->callbacks.r_cb = rcb;
}

void ev__nonblock_stream_exit(ev_nonblock_stream_t* stream)
{
    ev__nonblock_stream_abort(stream);
    ev__nonblock_stream_cleanup(stream, EV_IO_IN | EV_IO_OUT);
    stream->loop = NULL;
    stream->callbacks.w_cb = NULL;
    stream->callbacks.r_cb = NULL;
}

int ev__nonblock_stream_write(ev_nonblock_stream_t* stream, ev_write_t* req)
{
    if (stream->flags.io_abort)
    {
        return EV_EBADF;
    }

    if (!stream->flags.io_reg_w)
    {
        ev__nonblock_io_add(stream->loop, &stream->io, EV_IO_OUT);
        stream->flags.io_reg_w = 1;
    }

    ev_list_push_back(&stream->pending.w_queue, &req->node);
    return EV_SUCCESS;
}

int ev__nonblock_stream_read(ev_nonblock_stream_t* stream, ev_read_t* req)
{
    if (stream->flags.io_abort)
    {
        return EV_EBADF;
    }

    if (!stream->flags.io_reg_r)
    {
        ev__nonblock_io_add(stream->loop, &stream->io, EV_IO_IN);
        stream->flags.io_reg_r = 1;
    }

    ev_list_push_back(&stream->pending.r_queue, &req->node);
    return EV_SUCCESS;
}

size_t ev__nonblock_stream_size(ev_nonblock_stream_t* stream, unsigned evts)
{
    size_t ret = 0;
    if (evts & EV_IO_IN)
    {
        ret += ev_list_size(&stream->pending.r_queue);
    }
    if (evts & EV_IO_OUT)
    {
        ret += ev_list_size(&stream->pending.w_queue);
    }
    return ret;
}

void ev__nonblock_stream_abort(ev_nonblock_stream_t* stream)
{
    if (!stream->flags.io_abort)
    {
        ev__nonblock_io_del(stream->loop, &stream->io, EV_IO_IN | EV_IO_OUT);
        stream->flags.io_abort = 1;
    }
}

void ev__nonblock_stream_cleanup(ev_nonblock_stream_t* stream, unsigned evts)
{
    if (evts & EV_IO_OUT)
    {
        _ev_stream_cleanup_w(stream, EV_ECANCELED);
    }

    if (evts & EV_IO_IN)
    {
        _ev_stream_cleanup_r(stream, EV_ECANCELED);
    }
}

ssize_t ev__write_unix(int fd, void* buffer, size_t size)
{
    ssize_t send_size;
    do
    {
        send_size = write(fd, buffer, size);
    } while (send_size == -1 && errno == EINTR);

    if (send_size >= 0)
    {
        return send_size;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }

    return ev__translate_sys_error(err);
}

void ev__loop_wakeup(ev_loop_t* loop)
{
    static const uint64_t val = 1;
    ssize_t write_size;
    do
    {
        write_size = write(loop->backend.wakeup.fd, &val, sizeof(val));
    } while (write_size < 0 && errno == EINTR);

    if (write_size == sizeof(val))
    {
        return;
    }

    int err = errno;
    if (write_size == -1)
    {
        if (err == EV_EAGAIN || err == EWOULDBLOCK)
        {
            return;
        }
    }
    abort();
}

static int _ev_finalize_send_req_unix(ev_write_t* req, size_t write_size)
{
    req->data.size += write_size;

    /* All data is sent */
    if (req->data.size == req->data.capacity)
    {
        req->data.nbuf = 0;
        return EV_SUCCESS;
    }
    assert(req->data.size < req->data.capacity);

    /* maintenance iovec */
    size_t idx;
    for (idx = 0; write_size > 0 && idx < req->data.nbuf; idx++)
    {
        if (write_size < req->data.bufs[idx].size)
        {
            req->data.bufs[idx].size -= write_size;
            req->data.bufs[idx].data = (uint8_t*)req->data.bufs[idx].data + write_size;
            break;
        }
        else
        {
            write_size -= req->data.bufs[idx].size;
        }
    }

    assert(idx < req->data.nbuf);
    assert(write_size > 0);

    memmove(&req->data.bufs[0], &req->data.bufs[idx], sizeof(req->data.bufs[0]) * (req->data.nbuf - idx));
    req->data.nbuf -= idx;

    return EV_EAGAIN;
}

int ev__send_unix(int fd, ev_write_t* req,
    ssize_t(*do_write)(int fd, struct iovec* iov, int iovcnt, void* arg), void* arg)
{
    ev_buf_t* iov = req->data.bufs;
    int iovcnt = req->data.nbuf;
    if (iovcnt > g_ev_loop_unix_ctx.iovmax)
    {
        iovcnt = g_ev_loop_unix_ctx.iovmax;
    }

    ssize_t write_size = do_write(fd, (struct iovec*)iov, iovcnt, arg);

    /* Check send result */
    if (write_size < 0)
    {
        if (write_size == EV_ENOBUFS)
        {
            write_size = EV_EAGAIN;
        }
        return write_size;
    }

    return _ev_finalize_send_req_unix(req, (size_t)write_size);
}
