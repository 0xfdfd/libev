#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "unix/loop.h"
#include "unix/tcp.h"
#include "unix/stream.h"
#include "pipe-common.h"
#include "handle.h"
#include "log.h"

typedef char ev_ipc_msghdr[CMSG_SPACE(sizeof(int))];

static void _ev_pipe_close_unix(ev_pipe_t* pipe)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        close(pipe->pipfd);
        pipe->pipfd = EV_OS_PIPE_INVALID;
    }
}

static void _ev_pipe_on_close_unix(ev_handle_t* handle)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(handle, ev_pipe_t, base);

    if (pipe_handle->close_cb != NULL)
    {
        pipe_handle->close_cb(pipe_handle);
    }
}

static void _ev_pipe_smart_deactive_unix(ev_pipe_t* pipe)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        int flag_have_write = 0;
        if (ev_list_size(&pipe->backend.ipc_mode.wio.wqueue) != 0
            || pipe->backend.ipc_mode.wio.curr.writing != NULL)
        {
            flag_have_write = 1;
        }

        int flag_have_read = 0;
        if (ev_list_size(&pipe->backend.ipc_mode.rio.rqueue) != 0
            || pipe->backend.ipc_mode.rio.curr.reading != NULL)
        {
            flag_have_read = 1;
        }

        if (!flag_have_write)
        {
            pipe->backend.ipc_mode.mask.wio_pending = 0;
            ev__nonblock_io_del(pipe->base.data.loop, &pipe->backend.ipc_mode.io, EPOLLOUT);
        }
        if (!flag_have_read)
        {
            pipe->backend.ipc_mode.mask.rio_pending = 0;
            ev__nonblock_io_del(pipe->base.data.loop, &pipe->backend.ipc_mode.io, EPOLLIN);
        }
        if (!flag_have_read && !flag_have_read)
        {
            ev__handle_deactive(&pipe->base);
        }
    }
    else
    {
        if (ev__nonblock_stream_size(&pipe->backend.data_mode.stream, EV_IO_IN | EV_IO_OUT) == 0)
        {
            ev__handle_deactive(&pipe->base);
        }
    }
}

static void _ev_pipe_w_user_callback_unix(ev_pipe_write_req_t* req, size_t size, int stat)
{
    ev__write_exit(&req->base);
    req->ucb(req, size, stat);
}

static void _ev_pipe_r_user_callback_unix(ev_pipe_read_req_t* req, size_t size, int stat)
{
    ev__read_exit(&req->base);
    req->ucb(req, size, stat);
}

static void _ev_pipe_on_data_mode_write_unix(ev_nonblock_stream_t* stream,
    ev_write_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(stream, ev_pipe_t, backend.data_mode.stream);
    _ev_pipe_smart_deactive_unix(pipe_handle);

    ev_pipe_write_req_t* w_req = EV_CONTAINER_OF(req, ev_pipe_write_req_t, base);
    _ev_pipe_w_user_callback_unix(w_req, size, stat);
}

static void _ev_pipe_on_data_mode_read_unix(ev_nonblock_stream_t* stream,
    ev_read_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe_handle = EV_CONTAINER_OF(stream, ev_pipe_t, backend.data_mode.stream);
    _ev_pipe_smart_deactive_unix(pipe_handle);

    ev_pipe_read_req_t* r_req = EV_CONTAINER_OF(req, ev_pipe_read_req_t, base);
    _ev_pipe_r_user_callback_unix(r_req, size, stat);
}

static int _ev_pipe_on_ipc_mode_io_read_remain(ev_pipe_t* pipe)
{
    assert(pipe->backend.ipc_mode.rio.curr.reading != NULL);

    ev_buf_t bufs[EV_IOV_MAX];
    size_t target_size = pipe->backend.ipc_mode.rio.curr.data_remain_size;
    ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.curr.reading;

    size_t buf_idx = pipe->backend.ipc_mode.rio.curr.buf_idx;
    size_t buf_pos = pipe->backend.ipc_mode.rio.curr.buf_pos;

    size_t idx;
    for (idx = 0;
        idx < ARRAY_SIZE(bufs) && target_size > 0 && buf_idx < req->base.data.nbuf;
        idx++, buf_idx++)
    {
        bufs[idx].data = (uint8_t*)req->base.data.bufs[buf_idx].data + buf_pos;
        bufs[idx].size = req->base.data.bufs[buf_idx].size - buf_pos;

        bufs[idx].size = EV_MIN(bufs[idx].size, target_size);
        target_size -= bufs[idx].size;

        buf_pos = 0;
    }

    ssize_t read_size = ev__readv_unix(pipe->pipfd, bufs, idx);
    if (read_size < 0)
    {
        return read_size;
    }

    pipe->backend.ipc_mode.rio.curr.data_remain_size -= read_size;
    req->base.data.size += read_size;

    /* no data remain */
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size == 0)
    {
        goto callback;
    }

    /* move cursor */
    while (read_size > 0 && pipe->backend.ipc_mode.rio.curr.buf_idx < req->base.data.nbuf)
    {
        size_t left_size = req->base.data.bufs[pipe->backend.ipc_mode.rio.curr.buf_idx].size
            - pipe->backend.ipc_mode.rio.curr.buf_pos;

        if (left_size > (size_t)read_size)
        {
            pipe->backend.ipc_mode.rio.curr.buf_pos += read_size;
            break;
        }

        read_size -= left_size;
        pipe->backend.ipc_mode.rio.curr.buf_idx++;
        pipe->backend.ipc_mode.rio.curr.buf_pos = 0;
        continue;
    }

    /* Buffer is full */
    if (pipe->backend.ipc_mode.rio.curr.buf_idx >= req->base.data.nbuf)
    {
        goto callback;
    }

    return EV_SUCCESS;

callback:
    pipe->backend.ipc_mode.rio.curr.reading = NULL;
    _ev_pipe_r_user_callback_unix(req, req->base.data.size, EV_SUCCESS);
    return EV_SUCCESS;
}

static ssize_t _ev_pipe_recvmsg_unix(ev_pipe_t* pipe, struct msghdr* msg)
{
    struct cmsghdr* cmsg;
    ssize_t rc;
    int* pfd;
    int* end;
    int fd = pipe->pipfd;
#if defined(__linux__)
    if (!pipe->backend.ipc_mode.mask.no_cmsg_cloexec)
    {
        rc = recvmsg(fd, msg, MSG_CMSG_CLOEXEC);
        if (rc != -1)
        {
            return rc;
        }
        if ((rc = errno) != EINVAL)
        {
            return ev__translate_sys_error(errno);
        }
        rc = recvmsg(fd, msg, 0);
        if (rc == -1)
        {
            return ev__translate_sys_error(errno);
        }
        pipe->backend.ipc_mode.mask.no_cmsg_cloexec = 1;
    }
    else
    {
        rc = recvmsg(fd, msg, 0);
    }
#else
    rc = recvmsg(fd, msg, 0);
#endif
    if (rc == -1)
    {
        return ev__translate_sys_error(errno);
    }
    if (msg->msg_controllen == 0)
    {
        return rc;
    }
    for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg))
    {
        if (cmsg->cmsg_type == SCM_RIGHTS)
        {
            for (pfd = (int*)CMSG_DATA(cmsg),
                end = (int*)((char*)cmsg + cmsg->cmsg_len);
                pfd < end;
                pfd += 1)
            {
                ev__cloexec(*pfd, 1);
            }
        }
    }
    return rc;
}

static void _ev_stream_do_read_parser_msghdr(ev_pipe_read_req_t* req, struct msghdr* msg)
{
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(msg);
    if (cmsg == NULL)
    {
        return;
    }

    void* pv = CMSG_DATA(cmsg);
    int* pi = pv;
    req->handle.os_socket = *pi;

    assert(CMSG_NXTHDR(msg, cmsg) == NULL);
}

static int _ev_pipe_on_ipc_mode_io_read_first(ev_pipe_t* pipe)
{
    void* buffer = (uint8_t*)pipe->backend.ipc_mode.rio.buffer + pipe->backend.ipc_mode.rio.curr.head_read_size;
    size_t buffer_size = sizeof(pipe->backend.ipc_mode.rio.buffer) - pipe->backend.ipc_mode.rio.curr.head_read_size;

    if (pipe->backend.ipc_mode.rio.curr.head_read_size == 0)
    {
        struct msghdr msg;
        ev_ipc_msghdr cmsg_space;
        struct iovec iov = { buffer, buffer_size };

        /* ipc uses recvmsg */
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        /* Set up to receive a descriptor even if one isn't in the message */
        msg.msg_controllen = sizeof(cmsg_space);
        msg.msg_control = cmsg_space;

        ssize_t read_size = _ev_pipe_recvmsg_unix(pipe, &msg);
        if (read_size <= 0)
        {/* Error or EOF */
            return read_size;
        }
        assert(read_size <= (ssize_t)sizeof(pipe->backend.ipc_mode.rio.buffer));

        _ev_stream_do_read_parser_msghdr(pipe->backend.ipc_mode.rio.curr.reading, &msg);
        pipe->backend.ipc_mode.rio.curr.head_read_size += read_size;
    }
    else
    {
        ssize_t read_size = read(pipe->pipfd, buffer, buffer_size);
        if (read_size == 0)
        {
            return EV_EOF;
        }
        if (read_size < 0)
        {
            int err = errno;
            if (err == EAGAIN)
            {/* try again */
                return EV_SUCCESS;
            }
            return ev__translate_sys_error(err);
        }
        pipe->backend.ipc_mode.rio.curr.head_read_size += read_size;
    }

    /* If frame header not read complete, try again */
    if (pipe->backend.ipc_mode.rio.curr.head_read_size < sizeof(ev_ipc_frame_hdr_t))
    {
        return EV_SUCCESS;
    }

    /* A invalid frame header means something wrong in the transmission link */
    if (!ev__ipc_check_frame_hdr(pipe->backend.ipc_mode.rio.buffer, sizeof(ev_ipc_frame_hdr_t)))
    {
        return EV_EPIPE;
    }

    ev_ipc_frame_hdr_t* hdr = (ev_ipc_frame_hdr_t*)pipe->backend.ipc_mode.rio.buffer;
    pipe->backend.ipc_mode.rio.curr.data_remain_size = hdr->hdr_dtsz;

    /* No data remain to read means peer send a empty package */
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size == 0)
    {
        ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.curr.reading;
        pipe->backend.ipc_mode.rio.curr.reading = NULL;
        _ev_pipe_r_user_callback_unix(req, req->base.data.size, EV_SUCCESS);
    }

    /* Process to read body */
    return _ev_pipe_on_ipc_mode_io_read_remain(pipe);
}

static int _ev_pipe_on_ipc_mode_io_read_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.rio.curr.data_remain_size != 0)
    {
        return _ev_pipe_on_ipc_mode_io_read_remain(pipe);
    }

    assert(pipe->backend.ipc_mode.rio.curr.reading == NULL);

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.rqueue);
    assert(it != NULL);

    pipe->backend.ipc_mode.rio.curr.reading = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    pipe->backend.ipc_mode.rio.curr.head_read_size = 0;
    pipe->backend.ipc_mode.rio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.rio.curr.buf_pos = 0;

    return _ev_pipe_on_ipc_mode_io_read_first(pipe);
}

static ssize_t _ev_pipe_sendmsg_unix(int fd, int fd_to_send, struct iovec* iov, int iovcnt)
{
    struct msghdr msg;
    struct cmsghdr* cmsg;

    ev_ipc_msghdr msg_ctrl_hdr;
    memset(&msg_ctrl_hdr, 0, sizeof(msg_ctrl_hdr));

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = iovcnt;
    msg.msg_flags = 0;

    msg.msg_control = msg_ctrl_hdr;
    msg.msg_controllen = sizeof(msg_ctrl_hdr);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));

    /* silence aliasing warning */
    {
        void* pv = CMSG_DATA(cmsg);
        int* pi = pv;
        *pi = fd_to_send;
    }

    ssize_t n;
    do
    {
        n = sendmsg(fd, &msg, 0);
    } while (n == -1 && errno == EINTR);

    if (n >= 0)
    {
        return n;
    }

    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK)
    {
        return 0;
    }
    return ev__translate_sys_error(err);
}

static int _ev_pipe_on_ipc_mode_io_write_remain_body_unix(ev_pipe_t* pipe)
{
    ev_buf_t bufs[EV_IOV_MAX];
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.curr.writing;
    assert(req != NULL);

    size_t buf_idx = pipe->backend.ipc_mode.wio.curr.buf_idx;
    size_t buf_pos = pipe->backend.ipc_mode.wio.curr.buf_pos;

    size_t idx;
    for (idx = 0; idx < ARRAY_SIZE(bufs) && buf_idx < req->base.nbuf; idx++, buf_idx++)
    {
        bufs[idx].data = (uint8_t*)req->base.bufs[buf_idx].data + buf_pos;
        bufs[idx].size = req->base.bufs[buf_idx].size - buf_pos;
        buf_pos = 0;
    }

    ssize_t write_size = ev__writev_unix(pipe->pipfd, bufs, idx);
    if (write_size < 0)
    {
        return write_size;
    }

    req->base.size += write_size;
    while (write_size > 0)
    {
        size_t left_size = req->base.bufs[pipe->backend.ipc_mode.wio.curr.buf_idx].size -
            pipe->backend.ipc_mode.wio.curr.buf_pos;
        if (left_size > (size_t)write_size)
        {
            pipe->backend.ipc_mode.wio.curr.buf_pos += write_size;
            write_size = 0;
            break;
        }

        write_size -= left_size;
        pipe->backend.ipc_mode.wio.curr.buf_idx++;
        pipe->backend.ipc_mode.wio.curr.buf_pos = 0;
    }

    /* send finish */
    if (pipe->backend.ipc_mode.wio.curr.buf_idx >= req->base.nbuf)
    {
        pipe->backend.ipc_mode.wio.curr.writing = NULL;
        _ev_pipe_w_user_callback_unix(req, req->base.size, EV_SUCCESS);
    }

    return EV_SUCCESS;
}

static int _ev_pipe_on_ipc_mode_io_write_remain_head_unix(ev_pipe_t* pipe)
{
    EV_LOG_TRACE("pipe(%p) send remain header", pipe);

    const size_t buffer_pos = pipe->backend.ipc_mode.wio.curr.head_send_size;
    void* buffer = pipe->backend.ipc_mode.wio.buffer + buffer_pos;
    size_t buffer_size = pipe->backend.ipc_mode.wio.curr.head_send_capacity - buffer_pos;

    ssize_t send_size = ev__write_unix(pipe->pipfd, buffer, buffer_size);
    if (send_size < 0)
    {
        return send_size;
    }
    pipe->backend.ipc_mode.wio.curr.head_send_size += send_size;

    /* try again */
    if ((size_t)send_size < buffer_size)
    {
        return EV_SUCCESS;
    }

    /* Write body */
    return _ev_pipe_on_ipc_mode_io_write_remain_body_unix(pipe);
}

static int _ev_pipe_on_ipc_mode_io_write_remain_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.wio.curr.head_send_size < pipe->backend.ipc_mode.wio.curr.head_send_capacity)
    {
        return _ev_pipe_on_ipc_mode_io_write_remain_head_unix(pipe);
    }

    return _ev_pipe_on_ipc_mode_io_write_remain_body_unix(pipe);
}

static int _ev_pipe_ipc_mode_write_new_frame_unix(ev_pipe_t* pipe)
{
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.curr.writing;

    ev_ipc_frame_hdr_t frame_hdr;
    ev__ipc_init_frame_hdr(&frame_hdr, 0, 0, req->base.capacity);
    memcpy(pipe->backend.ipc_mode.wio.buffer, &frame_hdr, sizeof(frame_hdr));
    pipe->backend.ipc_mode.wio.curr.head_send_capacity = sizeof(frame_hdr);

    ssize_t send_size;
    if (req->handle.role == EV_ROLE_EV_TCP)
    {
        struct iovec iov = {
            pipe->backend.ipc_mode.wio.buffer,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity
        };

        send_size = _ev_pipe_sendmsg_unix(pipe->pipfd, req->handle.u.os_socket, &iov, 1);
        if (send_size >= 0)
        {
            req->handle.role = EV_ROLE_UNKNOWN;
            req->handle.u.os_socket = EV_OS_SOCKET_INVALID;
        }
    }
    else
    {
        send_size = ev__write_unix(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity);
    }

    if (send_size == 0)
    {/* If data not send, try again */
        EV_LOG_TRACE("pipe(%p) data not send, try again", pipe);
        ev_list_push_front(&pipe->backend.ipc_mode.wio.wqueue, &req->base.node);
        pipe->backend.ipc_mode.wio.curr.writing = NULL;
        return EV_SUCCESS;
    }
    else if (send_size < 0)
    {/* send_size is error code */
        EV_LOG_ERROR("pipe(%p) data send failed, err:%d", pipe, send_size);
        return send_size;
    }

    pipe->backend.ipc_mode.wio.curr.head_send_size = send_size;

    /* try again to send frame header */
    if ((size_t)send_size < pipe->backend.ipc_mode.wio.curr.head_send_capacity)
    {
        EV_LOG_TRACE("pipe(%p) frame header remain %zu bytes", pipe,
            pipe->backend.ipc_mode.wio.curr.head_send_capacity - send_size);
        return EV_SUCCESS;
    }

    return _ev_pipe_on_ipc_mode_io_write_remain_unix(pipe);
}

static void _ev_pipe_ipc_mode_reset_wio_curr_cnt(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.wio.curr.head_send_size = 0;
    pipe->backend.ipc_mode.wio.curr.head_send_capacity = 0;
    pipe->backend.ipc_mode.wio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.wio.curr.buf_pos = 0;
}

static void _ev_pipe_ipc_mode_reset_rio_curr_cnt(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.rio.curr.head_read_size = 0;
    pipe->backend.ipc_mode.rio.curr.data_remain_size = 0;
    pipe->backend.ipc_mode.rio.curr.buf_idx = 0;
    pipe->backend.ipc_mode.rio.curr.buf_pos = 0;
}

static int _ev_pipe_on_ipc_mode_io_write_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.wio.curr.writing != NULL)
    {
        return _ev_pipe_on_ipc_mode_io_write_remain_unix(pipe);
    }

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.wqueue);
    assert(it != NULL);
    pipe->backend.ipc_mode.wio.curr.writing =
        EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
    _ev_pipe_ipc_mode_reset_wio_curr_cnt(pipe);

    return _ev_pipe_ipc_mode_write_new_frame_unix(pipe);
}

static void _ev_pipe_ipc_mode_cancel_all_rio_unix(ev_pipe_t* pipe, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.rqueue)) != NULL)
    {
        ev_pipe_read_req_t* req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_unix(req, req->base.data.size, stat);
    }
}

static void _ev_pipe_ipc_mode_cancel_all_wio_unix(ev_pipe_t* pipe, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.wqueue)) != NULL)
    {
        ev_pipe_write_req_t* req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_unix(req, req->base.size, stat);
    }
}

static void _ev_pipe_abort_unix(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ev__nonblock_io_del(pipe->base.data.loop, &pipe->backend.ipc_mode.io, EV_IO_IN | EV_IO_OUT);
        _ev_pipe_close_unix(pipe);

        _ev_pipe_ipc_mode_cancel_all_rio_unix(pipe, stat);
        _ev_pipe_ipc_mode_cancel_all_wio_unix(pipe, stat);
    }
    else
    {
        if (pipe->base.data.flags & EV_HANDLE_PIPE_STREAMING)
        {
            ev__nonblock_stream_exit(&pipe->backend.data_mode.stream);
            pipe->base.data.flags &= ~EV_HANDLE_PIPE_STREAMING;
        }
        _ev_pipe_close_unix(pipe);
    }
}

static void _ev_pipe_on_ipc_mode_io_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;

    int ret = EV_SUCCESS;
    ev_pipe_t* pipe = EV_CONTAINER_OF(io, ev_pipe_t, backend.ipc_mode.io);

    if (evts & (EPOLLIN | EPOLLHUP))
    {
        if ((ret = _ev_pipe_on_ipc_mode_io_read_unix(pipe)) != EV_SUCCESS)
        {
            goto err;
        }
    }
    if (evts & (EPOLLOUT | EPOLLERR))
    {
        if ((ret = _ev_pipe_on_ipc_mode_io_write_unix(pipe)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    _ev_pipe_smart_deactive_unix(pipe);
    return;

err:
    _ev_pipe_abort_unix(pipe, ret);
}

static void _ev_pipe_init_as_ipc_mode_unix(ev_pipe_t* pipe)
{
    ev__nonblock_io_init(&pipe->backend.ipc_mode.io, pipe->pipfd, _ev_pipe_on_ipc_mode_io_unix, NULL);
    memset(&pipe->backend.ipc_mode.mask, 0, sizeof(pipe->backend.ipc_mode.mask));

    _ev_pipe_ipc_mode_reset_rio_curr_cnt(pipe);
    pipe->backend.ipc_mode.rio.curr.reading = NULL;
    ev_list_init(&pipe->backend.ipc_mode.rio.rqueue);

    _ev_pipe_ipc_mode_reset_wio_curr_cnt(pipe);
    pipe->backend.ipc_mode.wio.curr.writing = NULL;
    ev_list_init(&pipe->backend.ipc_mode.wio.wqueue);
}

static void _ev_pipe_ipc_mode_want_write_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.mask.wio_pending)
    {
        return;
    }

    ev__nonblock_io_add(pipe->base.data.loop, &pipe->backend.ipc_mode.io, EV_IO_OUT);
    pipe->backend.ipc_mode.mask.wio_pending = 1;
}

static void _ev_pipe_ipc_mode_want_read_unix(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.mask.rio_pending)
    {
        return;
    }

    ev__nonblock_io_add(pipe->base.data.loop, &pipe->backend.ipc_mode.io, EV_IO_IN);
    pipe->backend.ipc_mode.mask.rio_pending = 1;
}

static int _ev_pipe_write_ipc_mode_unix(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    if (req->base.capacity > UINT32_MAX)
    {
        return EV_E2BIG;
    }

    ev_list_push_back(&pipe->backend.ipc_mode.wio.wqueue, &req->base.node);
    _ev_pipe_ipc_mode_want_write_unix(pipe);

    return EV_SUCCESS;
}

static int _ev_pipe_read_ipc_mode_unix(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    ev_list_push_back(&pipe->backend.ipc_mode.rio.rqueue, &req->base.node);
    _ev_pipe_ipc_mode_want_read_unix(pipe);
    return EV_SUCCESS;
}

static int _ev_pipe_make_pipe(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    int errcode;
    fds[0] = EV_OS_PIPE_INVALID;
    fds[1] = EV_OS_PIPE_INVALID;

    if (pipe(fds) < 0)
    {
        errcode = errno;
        errcode = ev__translate_sys_error(errcode);
        goto err;
    }

    if ((errcode = ev__cloexec(fds[0], 1)) != EV_SUCCESS)
    {
        goto err;
    }
    if ((errcode = ev__cloexec(fds[1], 1)) != EV_SUCCESS)
    {
        goto err;
    }

    if (rflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[0], 1)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    if (wflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[1], 1)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    return EV_SUCCESS;

err:
    if (fds[0] != EV_OS_PIPE_INVALID)
    {
        close(fds[0]);
        fds[0] = EV_OS_PIPE_INVALID;
    }
    if (fds[1] != EV_OS_PIPE_INVALID)
    {
        close(fds[1]);
        fds[1] = EV_OS_PIPE_INVALID;
    }
    return errcode;
}

static int _ev_pipe_make_socketpair(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    int errcode;
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    if (rflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[0], 1)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    if (wflags & EV_PIPE_NONBLOCK)
    {
        if ((errcode = ev__nonblock(fds[1], 1)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    return EV_SUCCESS;

err:
    close(fds[0]);
    close(fds[1]);
    return errcode;
}

int ev_pipe_make(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    if ((rflags & EV_PIPE_IPC) != (wflags & EV_PIPE_IPC))
    {
        return EV_EINVAL;
    }

    int is_ipc = rflags & EV_PIPE_IPC;

    if (is_ipc)
    {
        return _ev_pipe_make_socketpair(fds, rflags, wflags);
    }

    return _ev_pipe_make_pipe(fds, rflags, wflags);
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE, _ev_pipe_on_close_unix);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;
    pipe->base.data.flags |= ipc ? EV_HANDLE_PIPE_IPC : 0;

    return EV_SUCCESS;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    pipe->close_cb = cb;
    _ev_pipe_abort_unix(pipe, EV_ECANCELED);
    ev__handle_exit(&pipe->base, 0);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        return EV_EEXIST;
    }

    int mode = ev__fcntl_getfl_unix(handle);
    if (mode == -1)
    {
        return ev__translate_sys_error(errno);
    }

    int ret;
    if ((ret = ev__nonblock(handle, 1)) != EV_SUCCESS)
    {
        return ret;
    }

    pipe->pipfd = handle;
    pipe->base.data.flags |= EV_HANDLE_PIPE_STREAMING;

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_init_as_ipc_mode_unix(pipe);
    }
    else
    {
        ev__nonblock_stream_init(pipe->base.data.loop, &pipe->backend.data_mode.stream, handle,
            _ev_pipe_on_data_mode_write_unix, _ev_pipe_on_data_mode_read_unix);
    }

    return EV_SUCCESS;
}

int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size,
    ev_pipe_write_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_write_init_ext(req, cb, bufs, nbuf,
        handle_role, handle_addr, handle_size);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_active(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_write_ipc_mode_unix(pipe, req);
    }
    else
    {
        ret = ev__nonblock_stream_write(&pipe->backend.data_mode.stream, &req->base);
    }

    if (ret != EV_SUCCESS && ret != EV_E2BIG)
    {
        _ev_pipe_abort_unix(pipe, ret);
    }

    return ret;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_read_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_read_init(req, bufs, nbuf, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_active(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_read_ipc_mode_unix(pipe, req);
    }
    else
    {
        ret = ev__nonblock_stream_read(&pipe->backend.data_mode.stream, &req->base);
    }

    if (ret != EV_SUCCESS)
    {
        _ev_pipe_abort_unix(pipe, ret);
    }

    return ret;
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    if (!(pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
        || handle_role != EV_ROLE_EV_TCP
        || handle_addr == NULL)
    {
        return EV_EINVAL;
    }
    if (req->handle.os_socket == EV_OS_SOCKET_INVALID)
    {
        return EV_ENOENT;
    }
    if (handle_size < sizeof(ev_buf_t))
    {
        return EV_ENOMEM;
    }

    ev_tcp_t* tcp = handle_addr;
    return ev__tcp_open(tcp, req->handle.os_socket);
}

void ev_pipe_close(ev_os_pipe_t fd)
{
    close(fd);
}
