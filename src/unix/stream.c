#include "stream.h"

static ssize_t _ev_stream_do_write_writev_unix(int fd, struct iovec* iov, int iovcnt, void* arg)
{
    (void)arg;
    return ev__writev_unix(fd, (ev_buf_t*)iov, iovcnt);
}

static int _ev_stream_do_write_once(ev_nonblock_stream_t* stream, ev_write_t* req)
{
    return ev__send_unix(stream->io.data.fd, req, _ev_stream_do_write_writev_unix, NULL);
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

static void _ev_stream_do_write(ev_nonblock_stream_t* stream)
{
    int ret;
    ev_list_node_t* it;
    ev_write_t* req;

    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_write_t, node);
        if ((ret = _ev_stream_do_write_once(stream, req)) == EV_SUCCESS)
        {
            stream->callbacks.w_cb(stream, req, req->size, EV_SUCCESS);
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
        req = EV_CONTAINER_OF(it, ev_write_t, node);
        stream->callbacks.w_cb(stream, req, req->size, ret);
    }
}

static void _ev_stream_do_read(ev_nonblock_stream_t* stream)
{
    int ret;
    ev_list_node_t* it = ev_list_pop_front(&stream->pending.r_queue);
    ev_read_t* req = EV_CONTAINER_OF(it, ev_read_t, node);

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
        req = EV_CONTAINER_OF(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, 0, ret);
    }
}

static void _ev_stream_cleanup_r(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.r_queue)) != NULL)
    {
        ev_read_t* req = EV_CONTAINER_OF(it, ev_read_t, node);
        stream->callbacks.r_cb(stream, req, 0, errcode);
    }
}

static void _ev_stream_cleanup_w(ev_nonblock_stream_t* stream, int errcode)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&stream->pending.w_queue)) != NULL)
    {
        ev_write_t* req = EV_CONTAINER_OF(it, ev_write_t, node);
        stream->callbacks.w_cb(stream, req, req->size, errcode);
    }
}

static void _ev_nonblock_stream_on_io(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;
    ev_nonblock_stream_t* stream = EV_CONTAINER_OF(io, ev_nonblock_stream_t, io);

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
