#include "ev-common.h"

int ev_pipe_write_init(ev_pipe_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_write_cb cb)
{
    return ev_pipe_write_init_ext(req, cb, bufs, nbuf, NULL, 0, EV_ROLE_UNKNOWN, NULL, 0);
}

int ev_pipe_write_init_ext(ev_pipe_write_req_t* req, ev_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    int ret;
    if ((ret = ev_write_init_ext(&req->base, callback, bufs, nbuf, iov_bufs, iov_size)) != EV_SUCCESS)
    {
        return ret;
    }

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
