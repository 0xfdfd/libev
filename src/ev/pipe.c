
int ev_pipe_write(ev_pipe_t *pipe, ev_buf_t *bufs, size_t nbuf,
                  ev_pipe_write_cb cb, void *arg)
{
    return ev_pipe_write_ex(pipe, bufs, nbuf, EV_ROLE_UNKNOWN, NULL, cb, arg);
}

EV_LOCAL int ev__pipe_read_init(ev_pipe_read_req_t *req, ev_buf_t *bufs,
                                size_t nbuf, ev_pipe_read_cb cb)
{
    int ret;
    if ((ret = ev__read_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }
    req->ucb = cb;

    req->handle.os_socket = EV_OS_SOCKET_INVALID;
    return 0;
}

EV_LOCAL int ev__pipe_write_init(ev_pipe_write_req_t *req, ev_buf_t *bufs,
                                 size_t nbuf, ev_pipe_write_cb cb, void *arg)
{
    return ev__pipe_write_init_ext(req, cb, arg, bufs, nbuf, EV_ROLE_UNKNOWN,
                                   NULL);
}

EV_LOCAL int ev__pipe_write_init_ext(ev_pipe_write_req_t *req,
                                     ev_pipe_write_cb cb, void *arg,
                                     ev_buf_t *bufs, size_t nbuf,
                                     ev_role_t handle_role, void *handle_addr)
{
    int ret;
    if ((ret = ev__write_init(&req->base, bufs, nbuf)) != 0)
    {
        return ret;
    }
    req->ucb = cb;
    req->ucb_arg = arg;

    req->handle.role = handle_role;
    switch (handle_role)
    {
        /* no handle need to send */
    case EV_ROLE_UNKNOWN:
        break;

    case EV_ROLE_EV_TCP:
        if (((ev_tcp_t *)handle_addr)->base.data.role != EV_ROLE_EV_TCP)
        {
            return EV_EINVAL;
        }
        req->handle.u.os_socket = ((ev_tcp_t *)handle_addr)->sock;
        break;

        /* not support other type */
    default:
        return EV_EINVAL;
    }

    return 0;
}
