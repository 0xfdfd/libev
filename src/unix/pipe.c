#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "unix/loop.h"
#include "unix/tcp.h"

static void _ev_pipe_on_close(ev_handle_t* handle)
{
    ev_pipe_t* pipe_handle = container_of(handle, ev_pipe_t, base);

    if (handle->data.flags & EV_PIPE_STREAMING)
    {
        ev__nonblock_stream_exit(&pipe_handle->backend.stream);
        handle->data.flags &= ~EV_PIPE_STREAMING;
    }

    if (pipe_handle->pipfd != EV_OS_PIPE_INVALID)
    {
        close(pipe_handle->pipfd);
        pipe_handle->pipfd = EV_OS_PIPE_INVALID;
    }

    if (pipe_handle->close_cb != NULL)
    {
        pipe_handle->close_cb(pipe_handle);
    }
}

static void _ev_pipe_smart_deactive(ev_pipe_t* pipe)
{
    if (ev__nonblock_stream_size(&pipe->backend.stream, EV_IO_IN | EV_IO_OUT) == 0)
    {
        ev__handle_deactive(&pipe->base);
    }
}

static void _ev_pipe_on_write(ev_nonblock_stream_t* stream, ev_write_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe_handle = container_of(stream, ev_pipe_t, backend.stream);
    _ev_pipe_smart_deactive(pipe_handle);

    req->data.cb(req, size, stat);
}

static void _ev_pipe_on_read(ev_nonblock_stream_t* stream, ev_read_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe_handle = container_of(stream, ev_pipe_t, backend.stream);
    _ev_pipe_smart_deactive(pipe_handle);
    req->data.cb(req, size, stat);
}

int ev_pipe_make(ev_os_pipe_t fds[2])
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    if (ev__cloexec(fds[0], 1) != EV_SUCCESS
        || ev__cloexec(fds[1], 1) != EV_SUCCESS
        || ev__nonblock(fds[0], 1)
        || ev__nonblock(fds[1], 1))
    {
        int err = errno;
        close(fds[0]);
        close(fds[1]);
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE, _ev_pipe_on_close);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;

    if (ipc)
    {
        pipe->base.data.flags |= EV_PIPE_IPC;
    }

    return EV_SUCCESS;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        return EV_EEXIST;
    }

    int mode = ev__getfl(handle);
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
    pipe->base.data.flags |= EV_PIPE_STREAMING;

    int is_ipc = pipe->base.data.flags & EV_PIPE_IPC;
    ev__nonblock_stream_init(pipe->base.data.loop, &pipe->backend.stream, handle,
        is_ipc, _ev_pipe_on_write, _ev_pipe_on_read);

    return EV_SUCCESS;
}

int ev_pipe_write(ev_pipe_t* pipe, ev_write_t* req)
{
    ev__handle_active(&pipe->base);

    ev__write_init_unix(req);
    return ev__nonblock_stream_write(&pipe->backend.stream, req);
}

int ev_pipe_read(ev_pipe_t* pipe, ev_read_t* req)
{
    ev__handle_active(&pipe->base);

    ev__read_init_unix(req);
    return ev__nonblock_stream_read(&pipe->backend.stream, req);
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_read_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    if (!(pipe->base.data.flags & EV_PIPE_IPC)
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
