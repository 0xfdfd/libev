#include "ev/errno.h"
#include "ev/request.h"
#include "misc_unix.h"
#include "io_unix.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

static int _ev_io_finalize_send_req_unix(ev_write_t* req, size_t write_size)
{
    req->size += write_size;

    /* All data is sent */
    if (req->size == req->capacity)
    {
        req->nbuf = 0;
        return EV_SUCCESS;
    }
    assert(req->size < req->capacity);

    /* maintenance iovec */
    size_t idx;
    for (idx = 0; write_size > 0 && idx < req->nbuf; idx++)
    {
        if (write_size < req->bufs[idx].size)
        {
            req->bufs[idx].size -= write_size;
            req->bufs[idx].data = (uint8_t*)req->bufs[idx].data + write_size;
            break;
        }
        else
        {
            write_size -= req->bufs[idx].size;
        }
    }

    assert(idx < req->nbuf);

    memmove(&req->bufs[0], &req->bufs[idx], sizeof(req->bufs[0]) * (req->nbuf - idx));
    req->nbuf -= idx;

    return EV_EAGAIN;
}

static int _ev_cmp_io_unix(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_nonblock_io_t* io1 = EV_CONTAINER_OF(key1, ev_nonblock_io_t, node);
    ev_nonblock_io_t* io2 = EV_CONTAINER_OF(key2, ev_nonblock_io_t, node);
    return io1->data.fd - io2->data.fd;
}

void ev__init_io(ev_loop_t* loop)
{
    int err;
    ev_map_init(&loop->backend.io, _ev_cmp_io_unix, NULL);

    if ((loop->backend.pollfd = epoll_create(256)) == -1)
    {
        err = errno;
        EV_ABORT("errno:%d", err);
    }
    if ((err = ev__cloexec(loop->backend.pollfd, 1)) != 0)
    {
        err = errno;
        EV_ABORT("errno:%d", err);
    }
}

void ev__exit_io(ev_loop_t* loop)
{
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
    int errcode;
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
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_ADD)
    {
        ev_map_insert(&loop->backend.io, &io->node);
    }
}

void ev__nonblock_io_del(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts)
{
    int errcode;
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
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    io->data.c_events = io->data.n_events;
    if (op == EPOLL_CTL_DEL)
    {
        ev_map_erase(&loop->backend.io, &io->node);
    }
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

int ev__send_unix(int fd, ev_write_t* req,
    ssize_t(*do_write)(int fd, struct iovec* iov, int iovcnt, void* arg), void* arg)
{
    ev_buf_t* iov = req->bufs;
    int iovcnt = req->nbuf;
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

    return _ev_io_finalize_send_req_unix(req, (size_t)write_size);
}
