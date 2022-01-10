#include "ev-common.h"
#include "pipe-common.h"

static void _ev_pipe_on_write(ev_write_t* req, size_t size, int stat)
{
    ev_pipe_write_req_t* real_req = container_of(req, ev_pipe_write_req_t, base);
    real_req->ucb(real_req, size, stat);
}

static void _ev_pipe_on_read(ev_read_t* req, size_t size, int stat)
{
    ev_pipe_read_req_t* real_req = container_of(req, ev_pipe_read_req_t, base);
    real_req->ucb(real_req, size, stat);
}

int ev_pipe_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_write_cb cb)
{
    return ev_pipe_write_ex(pipe, req, bufs, nbuf, EV_ROLE_UNKNOWN, NULL, 0, NULL, 0, cb);
}

int ev__pipe_read_init(ev_pipe_read_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_read_cb cb)
{
    int ret;
    if ((ret = ev_read_init(&req->base, bufs, nbuf, _ev_pipe_on_read)) != EV_SUCCESS)
    {
        return ret;
    }
    req->ucb = cb;

    req->handle.os_socket = EV_OS_SOCKET_INVALID;
    return EV_SUCCESS;
}

int ev__pipe_write_init(ev_pipe_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_pipe_write_cb cb)
{
    return ev__pipe_write_init_ext(req, cb, bufs, nbuf, NULL, 0, EV_ROLE_UNKNOWN, NULL, 0);
}

int ev__pipe_write_init_ext(ev_pipe_write_req_t* req, ev_pipe_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    int ret;
    if ((ret = ev_write_init_ext(&req->base, _ev_pipe_on_write, bufs, nbuf, iov_bufs, iov_size)) != EV_SUCCESS)
    {
        return ret;
    }
    req->ucb = callback;

    req->handle.role = handle_role;
    switch (handle_role)
    {
        /* no handle need to send */
    case EV_ROLE_UNKNOWN:
        break;

    case EV_ROLE_EV_TCP:
        if (handle_size != sizeof(ev_tcp_t))
        {
            return EV_EINVAL;
        }
        req->handle.u.os_socket = ((ev_tcp_t*)handle_addr)->sock;
        break;

        /* not support other type */
    default:
        return EV_EINVAL;
    }

    return EV_SUCCESS;
}