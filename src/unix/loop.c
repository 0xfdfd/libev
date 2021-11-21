#include <sys/ioctl.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ev-platform.h"
#include "ev.h"
#include "loop.h"

typedef enum stream_flags
{
    STREAM_REG_IO = 1,
}stream_flags_t;

static clockid_t g_hwtime_clock_id = CLOCK_MONOTONIC;

static void _ev_init_hwtime(void)
{
    struct timespec t;
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &t) != 0)
    {
        return;
    }
    if (t.tv_nsec > 1 * 1000 * 1000)
    {
        return;
    }
    g_hwtime_clock_id = CLOCK_MONOTONIC_COARSE;
}

static int _ev_cmp_io_unix(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_io_t* io1 = container_of(key1, ev_io_t, node);
    ev_io_t* io2 = container_of(key2, ev_io_t, node);
    return io1->data.fd - io2->data.fd;
}

static ev_io_t* _ev_find_io(ev_loop_t* loop, int fd)
{
    ev_io_t tmp;
    tmp.data.fd = fd;

    ev_map_node_t* it = ev_map_find(&loop->backend.io, &tmp.node);
    return it != NULL ? container_of(it, ev_io_t, node) : NULL;
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
        ev_io_t* io = _ev_find_io(loop, events[i].data.fd);
        io->data.cb(io, events[i].events);
    }

    return nfds;
}

static int _ev_stream_do_write_once(ev_stream_t* stream, ev_write_t* req)
{
    ssize_t w;
    struct iovec* iov;
    int iovcnt;

    do
    {
        iov = (struct iovec*)(req->data.bufs + req->backend.idx);
        iovcnt = req->data.nbuf - req->backend.idx;
        w = writev(stream->io.data.fd, iov, iovcnt);
    } while (w == -1 && errno == EINTR);

    if (w < 0)
    {
        return (errno == EAGAIN || errno == EWOULDBLOCK) ?
            EV_EAGAIN : ev__translate_sys_error(errno);
    }

    req->backend.len += w;
    while (w > 0)
    {
        if ((size_t)w < req->data.bufs[req->backend.idx].size)
        {
            req->data.bufs[req->backend.idx].data =
                (void*)((uint8_t*)(req->data.bufs[req->backend.idx].data) + w);
            req->data.bufs[req->backend.idx].size -= w;
            break;
        }

        w -= req->data.bufs[req->backend.idx].size;
        req->backend.idx++;
    }

    return req->backend.idx < req->data.nbuf ? EV_EAGAIN : EV_SUCCESS;
}

static void _ev_stream_do_write(ev_stream_t* stream)
{
    int ret;
    ev_list_node_t* it;
    ev_write_t* req;

    while ((it = ev_list_pop_front(&stream->w_queue)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        if ((ret = _ev_stream_do_write_once(stream, req)) == EV_SUCCESS)
        {
            stream->w_cb(stream, req, req->backend.len, EV_SUCCESS);
            continue;
        }

        /* Unsuccess operation should restore list */
        ev_list_push_front(&stream->w_queue, it);

        if (ret == EV_EAGAIN)
        {
            break;
        }
        goto err;
    }

    return;

err:
    while ((it = ev_list_pop_front(&stream->w_queue)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        stream->w_cb(stream, req, req->backend.len, ret);
    }
}

static int _ev_stream_do_read_once(ev_stream_t* stream, ev_read_t* req, size_t* size)
{
    ssize_t r;
    do
    {
        r = readv(stream->io.data.fd, (struct iovec*)req->data.bufs, req->data.nbuf);
    } while (r == -1 && errno == EINTR);

    if (r == 0)
    {/* Peer close */
        return EV_EOF;
    }

    if (r < 0)
    {
        return (errno == EAGAIN || errno == EWOULDBLOCK) ?
            EV_EAGAIN : ev__translate_sys_error(errno);
    }

    *size = r;
    return EV_SUCCESS;
}

static void _ev_stream_do_read(ev_stream_t* stream)
{
    int ret;
    ev_list_node_t* it = ev_list_pop_front(&stream->r_queue);
    ev_read_t* req = container_of(it, ev_read_t, node);

    size_t r_size = 0;
    if ((ret = _ev_stream_do_read_once(stream, req, &r_size)) == EV_SUCCESS)
    {
        stream->r_cb(stream, req, r_size, EV_SUCCESS);
        return;
    }

    ev_list_push_front(&stream->r_queue, it);

    if (ret == EV_EAGAIN)
    {
        return;
    }

    /* If error, cleanup all pending read requests */
    while ((it = ev_list_pop_front(&stream->r_queue)) != NULL)
    {
        req = container_of(it, ev_read_t, node);
        stream->r_cb(stream, req, 0, ret);
    }
}

static void _ev_stream_cleanup_r(ev_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->r_queue)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        stream->r_cb(stream, req, 0, errcode);
    }
}

static void _ev_stream_cleanup_w(ev_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->w_queue)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        stream->w_cb(stream, req, req->backend.len, errcode);
    }
}

static void _ev_stream_on_io(ev_io_t* io, unsigned evts)
{
    ev_stream_t* stream = container_of(io, ev_stream_t, io);

    if (evts & EPOLLOUT)
    {
        _ev_stream_do_write(stream);
        if (ev_list_size(&stream->w_queue) == 0)
        {
            ev__io_del(stream->loop, &stream->io, EV_IO_OUT);
        }
    }

    else if (evts & (EPOLLIN | EPOLLHUP))
    {
        _ev_stream_do_read(stream);
        if (ev_list_size(&stream->r_queue) == 0)
        {
            ev__io_del(stream->loop, &stream->io, EV_IO_IN);
        }
    }
}

void ev__loop_update_time(ev_loop_t* loop)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_init_hwtime);

    struct timespec t;
    if (clock_gettime(g_hwtime_clock_id, &t) != 0)
    {
        return;
    }

    loop->hwtime = t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
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
        return errno;
    }

    return EV_SUCCESS;
#else
    int flags;

    int r = ev__getfd(fd);
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
        return errno;
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

    int r = ev__getfl(fd);
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

int ev__getfl(int fd)
{
    int mode;
    do
    {
        mode = fcntl(fd, F_GETFL);
    } while (mode == -1 && errno == EINTR);
    return mode;
}

int ev__getfd(int fd)
{
    int flags;

    do
    {
        flags = fcntl(fd, F_GETFD);
    } while (flags == -1 && errno == EINTR);

    return flags;
}

int ev__loop_init_backend(ev_loop_t* loop)
{
    ENSURE_LAYOUT(ev_buf_t, struct iovec, data, iov_base, size, iov_len);

    ev_map_init(&loop->backend.io, _ev_cmp_io_unix, NULL);

    if ((loop->backend.pollfd = epoll_create1(EPOLL_CLOEXEC)) != -1)
    {
        return EV_SUCCESS;
    }

    if (errno != ENOSYS && errno != EINVAL)
    {
        return errno;
    }

    if ((loop->backend.pollfd = epoll_create(256)) == -1)
    {
        return errno;
    }

    int ret;
    if ((ret = ev__cloexec(loop->backend.pollfd, 1)) != 0)
    {
        close(loop->backend.pollfd);
        loop->backend.pollfd = -1;
        return ret;
    }

    return EV_SUCCESS;
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
    close(loop->backend.pollfd);
    loop->backend.pollfd = -1;
}

void ev__io_init(ev_io_t* io, int fd, ev_io_cb cb)
{
    io->data.fd = fd;
    io->data.c_events = 0;
    io->data.n_events = 0;
    io->data.cb = cb;
}

void ev__io_add(ev_loop_t* loop, ev_io_t* io, unsigned evts)
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
        ABORT();
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_ADD)
    {
        ev_map_insert(&loop->backend.io, &io->node);
    }
}

void ev__io_del(ev_loop_t* loop, ev_io_t* io, unsigned evts)
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
        ABORT();
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
            ABORT();
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
    case EPROTONOSUPPORT:   return EV_EPROTONOSUPPORT;
    case ETIMEDOUT:         return EV_ETIMEDOUT;
    /* Unknown */
    default:                break;
    }

    return syserr;
}

int ev__write_init(ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
    req->data.cb = cb;
    req->data.nbuf = nbuf;
    req->backend.idx = 0;
    req->backend.len = 0;

    if (nbuf <= ARRAY_SIZE(req->data.bufsml))
    {
        req->data.bufs = req->data.bufsml;
    }
    else
    {
        if ((req->data.bufs = malloc(sizeof(ev_buf_t) * nbuf)) == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);
    return EV_SUCCESS;
}

void ev__write_exit(ev_write_t* req)
{
    if (req->data.bufs != req->data.bufsml)
    {
        free(req->data.bufs);
    }
    req->data.bufs = NULL;
}

int ev__read_init(ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb)
{
    req->data.cb = cb;
    req->data.nbuf = nbuf;

    if (nbuf <= ARRAY_SIZE(req->data.bufsml))
    {
        req->data.bufs = req->data.bufsml;
    }
    else
    {
        if ((req->data.bufs = malloc(sizeof(ev_buf_t) * nbuf)) == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);
    return  EV_SUCCESS;
}

void ev__read_exit(ev_read_t* req)
{
    if (req->data.bufs != req->data.bufsml)
    {
        free(req->data.bufs);
    }
    req->data.bufs = NULL;
}

void ev__stream_init(ev_loop_t* loop, ev_stream_t* stream, int fd, ev_stream_write_cb wcb, ev_stream_read_cb rcb)
{
    stream->loop = loop;
    stream->flags = STREAM_REG_IO;
    ev__io_init(&stream->io, fd, _ev_stream_on_io);
    ev_list_init(&stream->w_queue);
    ev_list_init(&stream->r_queue);
    stream->w_cb = wcb;
    stream->r_cb = rcb;
}

void ev__stream_exit(ev_stream_t* stream)
{
    ev__stream_abort(stream, EV_IO_IN | EV_IO_OUT);
    ev__stream_cleanup(stream, EV_IO_IN | EV_IO_OUT);
    stream->loop = NULL;
    stream->w_cb = NULL;
    stream->r_cb = NULL;
}

int ev__stream_write(ev_stream_t* stream, ev_write_t* req)
{
    if (!(stream->flags & STREAM_REG_IO))
    {
        return EV_EPERM;
    }

    ev_list_push_back(&stream->w_queue, &req->node);
    ev__io_add(stream->loop, &stream->io, EV_IO_OUT);
    return EV_SUCCESS;
}

int ev__stream_read(ev_stream_t* stream, ev_read_t* req)
{
    if (!(stream->flags & STREAM_REG_IO))
    {
        return EV_EPERM;
    }

    ev_list_push_back(&stream->r_queue, &req->node);
    ev__io_add(stream->loop, &stream->io, EV_IO_IN);
    return EV_SUCCESS;
}

size_t ev__stream_size(ev_stream_t* stream, unsigned evts)
{
    size_t ret = 0;
    if (evts & EV_IO_IN)
    {
        ret += ev_list_size(&stream->r_queue);
    }
    if (evts & EV_IO_OUT)
    {
        ret += ev_list_size(&stream->w_queue);
    }
    return ret;
}

void ev__stream_abort(ev_stream_t* stream, unsigned evts)
{
    if (stream->flags & STREAM_REG_IO)
    {
        ev__io_del(stream->loop, &stream->io, evts);
        stream->flags &= ~STREAM_REG_IO;
    }
}

void ev__stream_cleanup(ev_stream_t* stream, unsigned evts)
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
