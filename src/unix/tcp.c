#include <sys/uio.h>
#include <assert.h>
#include <unistd.h>
#include "loop.h"

static void _ev_tcp_on_event(ev_io_t* io, unsigned evts);

static void _ev_tcp_close_fd(ev_tcp_t* sock)
{
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        close(sock->sock);
        sock->sock = -1;
    }
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
    ev_tcp_t* sock = container_of(handle, ev_tcp_t, base);

    sock->base.flags &= ~(EV_TCP_LISTING | EV_TCP_ACCEPTING | EV_TCP_CONNECTING | EV_TCP_STREAMING);
    ev__io_del(sock->base.loop, &sock->backend.io, EV_IO_IN | EV_IO_OUT);
    _ev_tcp_close_fd(sock);

    if (sock->close_cb != NULL)
    {
        sock->close_cb(sock);
    }
}

static void _ev_tcp_on_connect(ev_tcp_t* sock)
{
    int ret;
    socklen_t result_len = sizeof(ret);

    ev__io_del(sock->base.loop, &sock->backend.io, EV_IO_OUT);
    sock->base.flags &= ~EV_TCP_CONNECTING;

    /* Get connect result */
    if (getsockopt(sock->sock, SOL_SOCKET, SO_ERROR, &ret, &result_len) < 0)
    {
        sock->backend.u.client.stat = ev__translate_sys_error(errno);
        goto fin;
    }

    /* Result is in `result` */
    sock->backend.u.client.stat = ev__translate_sys_error(ret);

fin:
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_accept(ev_tcp_t* acpt)
{
    ev_list_node_t* it = ev_list_pop_front(&acpt->backend.u.listen.accept_queue);
    if (it == NULL)
    {
        ABORT();
    }

    ev_tcp_t* conn = container_of(it, ev_tcp_t, backend.u.accept.accept_node);
    _ev_tcp_close_fd(conn);

    do
    {
        conn->sock = accept(acpt->sock, NULL, NULL);
    } while (conn->sock == -1 && errno == EINTR);

    conn->base.flags &= ~EV_TCP_ACCEPTING;
    ev__tcp_deactive(conn);

    int ret = conn->sock >= 0 ? EV_SUCCESS : ev__translate_sys_error(errno);
    if (ret == EV_SUCCESS)
    {/* Set non-block mode */
        if ((ret = ev__nonblock(conn->sock, 1)) != EV_SUCCESS)
        {
            _ev_tcp_close_fd(conn);
        }
    }

    ev__io_init(&conn->backend.io, conn->sock, _ev_tcp_on_event);
    conn->backend.u.accept.cb(acpt, conn, ret);

    if (ev_list_size(&acpt->backend.u.listen.accept_queue) == 0)
    {
        ev__io_del(acpt->base.loop, &acpt->backend.io, EV_IO_IN);
    }
}

static void _ev_tcp_cleanup_all_read_request(ev_tcp_t* sock, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        req->data.cb(req, 0, err);
    }
}

static void _ev_tcp_do_read(ev_tcp_t* sock)
{
    ev_list_node_t* it = ev_list_pop_front(&sock->backend.u.stream.r_queue);
    assert(it != NULL);
    ev_read_t* req = container_of(it, ev_read_t, node);

    ssize_t r;
    do
    {
        r = readv(sock->sock, (struct iovec*)req->data.bufs, req->data.nbuf);
    } while (r == -1 && errno == EINTR);

    /* Peer close */
    if (r == 0)
    {
        req->data.cb(req, 0, EV_EOF);
        goto fin;
    }

    /* Handle error */
    if (r < 0)
    {
        ev_list_push_front(&sock->backend.u.stream.r_queue, it);
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {/* Try again */
            return;
        }

        _ev_tcp_cleanup_all_read_request(sock, ev__translate_sys_error(errno));
        goto fin;
    }

    req->data.cb(req, r, EV_SUCCESS);

fin:
    if (ev_list_size(&sock->backend.u.stream.r_queue) == 0)
    {
        ev__io_del(sock->base.loop, &sock->backend.io, EV_IO_IN);
    }
}

static void _ev_tcp_cleanup_all_write_request(ev_tcp_t* sock, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        req->data.cb(req, req->backend.len, err);
    }
}

static void _ev_tcp_do_write(ev_tcp_t* sock)
{
    ev_list_node_t* it = ev_list_begin(&sock->backend.u.stream.w_queue);
    assert(it != NULL);
    ev_write_t* req = container_of(it, ev_write_t, node);

    // TODO write data
    ssize_t w;
    do
    {
        w = writev(sock->sock,
            (struct iovec*)(req->data.bufs + req->backend.idx), req->data.nbuf - req->backend.idx);
    } while (w == -1 && errno == EINTR);

    /* Handle error */
    if (w < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {/* Try again */
            return;
        }

        _ev_tcp_cleanup_all_write_request(sock, ev__translate_sys_error(errno));
        goto fin;
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
        continue;
    }

    /* Write complete */
    if (req->backend.idx == req->data.nbuf)
    {
        ev_list_erase(&sock->backend.u.stream.w_queue, it);
        req->data.cb(req, req->backend.len, EV_SUCCESS);
    }

fin:
    if (ev_list_size(&sock->backend.u.stream.w_queue) == 0)
    {
        ev__io_del(sock->base.loop, &sock->backend.io, EV_IO_OUT);
    }
}

static void _ev_tcp_on_event(ev_io_t* io, unsigned evts)
{
    ev_tcp_t* sock = container_of(io, ev_tcp_t, backend.io);

    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        _ev_tcp_on_connect(sock);
        goto fin;
    }

    if (sock->base.flags & EV_TCP_LISTING)
    {
        _ev_tcp_on_accept(sock);
        goto fin;
    }

    if (sock->base.flags & EV_TCP_STREAMING)
    {
        if (evts & EV_IO_IN)
        {
            _ev_tcp_do_read(sock);
        }
        if (evts & EV_IO_OUT)
        {
            _ev_tcp_do_write(sock);
        }
        if (ev_list_size(&sock->backend.u.stream.w_queue) == 0
            && ev_list_size(&sock->backend.u.stream.r_queue) == 0)
        {
            sock->base.flags &= ~EV_TCP_STREAMING;
        }
    }

fin:
    ev__tcp_deactive(sock);
}

static int _ev_tcp_setup_fd(ev_tcp_t* sock, int domain, int* new_fd)
{
    int ret = EV_SUCCESS;
    int tmp_new_fd = 0;
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        goto fin;
    }

    if ((sock->sock = socket(domain, SOCK_STREAM, 0)) == EV_OS_SOCKET_INVALID)
    {
        ret = ev__translate_sys_error(errno);
        goto fin;
    }

    if ((ret = ev__nonblock(sock->sock, 1)) != EV_SUCCESS)
    {
        goto err_nonblock;
    }

    tmp_new_fd = 1;
    ev__io_init(&sock->backend.io, sock->sock, _ev_tcp_on_event);

    goto fin;

err_nonblock:
    _ev_tcp_close_fd(sock);
fin:
    if (new_fd != NULL)
    {
        *new_fd = tmp_new_fd;
    }
    return ret;
}

static int _ev_tcp_is_listening(ev_tcp_t* sock)
{
    return sock->base.flags & EV_TCP_LISTING;
}

static void _ev_tcp_setup_stream(ev_tcp_t* sock)
{
    sock->base.flags |= EV_TCP_STREAMING;
    ev_list_init(&sock->backend.u.stream.w_queue);
    ev_list_init(&sock->backend.u.stream.r_queue);
}

static void _ev_tcp_to_connect(ev_todo_t* todo)
{
    ev_tcp_t* sock = container_of(todo, ev_tcp_t, backend.u.client.token);

    sock->base.flags &= ~EV_TCP_CONNECTING;
    sock->backend.u.client.cb(sock, EV_SUCCESS);
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* sock)
{
    ev__handle_init(loop, &sock->base, _ev_tcp_on_close);
    sock->close_cb = NULL;
    sock->sock = EV_OS_SOCKET_INVALID;

    return EV_SUCCESS;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    sock->close_cb = cb;
    ev__handle_exit(&sock->base);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_fd;
    if ((ret = _ev_tcp_setup_fd(tcp, addr->sa_family, &flag_new_fd)) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = bind(tcp->sock, addr, addrlen)) != 0)
    {
        ret = ev__translate_sys_error(errno);
        goto err_bind;
    }

    return EV_SUCCESS;

err_bind:
    if (flag_new_fd)
    {
        _ev_tcp_close_fd(tcp);
    }
    return ret;
}

int ev_tcp_listen(ev_tcp_t* tcp, int backlog)
{
    if (_ev_tcp_is_listening(tcp))
    {
        return EV_EADDRINUSE;
    }

    int ret;
    if ((ret = listen(tcp->sock, backlog)) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    ev_list_init(&tcp->backend.u.listen.accept_queue);
    tcp->base.flags |= EV_TCP_LISTING;

    return EV_SUCCESS;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_accept_cb cb)
{
    if (conn->base.flags & EV_TCP_ACCEPTING)
    {
        return EV_EINPROGRESS;
    }
    assert(cb != NULL);

    conn->base.flags |= EV_TCP_ACCEPTING;
    conn->backend.u.accept.cb = cb;
    ev_list_push_back(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.accept_node);
    ev__io_add(lisn->base.loop, &lisn->backend.io, EV_IO_IN);

    ev__tcp_active(lisn);
    ev__tcp_active(conn);

    return EV_SUCCESS;
}

int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
    if (sock->base.flags & (EV_TCP_LISTING | EV_TCP_ACCEPTING | EV_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    if (!(sock->base.flags & EV_TCP_STREAMING))
    {
        _ev_tcp_setup_stream(sock);
    }

    ev__write_init(req, bufs, nbuf, cb);
    ev_list_push_back(&sock->backend.u.stream.w_queue, &req->node);

    ev__io_add(sock->base.loop, &sock->backend.io, EV_IO_OUT);

    return EV_SUCCESS;
}

int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb)
{
    if (sock->base.flags & (EV_TCP_LISTING | EV_TCP_ACCEPTING | EV_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    if (!(sock->base.flags & EV_TCP_STREAMING))
    {
        _ev_tcp_setup_stream(sock);
    }

    ev__read_init(req, bufs, nbuf, cb);
    ev_list_push_back(&sock->backend.u.stream.r_queue, &req->node);

    ev__io_add(sock->base.loop, &sock->backend.io, EV_IO_IN);

    return EV_SUCCESS;
}

int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    socklen_t socklen = *len;
    if (getsockname(sock->sock, name, &socklen) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    *len = (size_t)socklen;
    return EV_SUCCESS;
}

int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    socklen_t socklen = *len;

    if ((ret = getpeername(sock->sock, name, &socklen)) != 0)
    {
        return ev__translate_sys_error(errno);
    }

    *len = socklen;
    return EV_SUCCESS;
}

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb)
{
    int ret;
    ev_loop_t* loop = sock->base.loop;

    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        return EV_EINPROGRESS;
    }
    if (sock->base.flags & (EV_TCP_LISTING | EV_TCP_ACCEPTING | EV_TCP_STREAMING))
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_tcp_setup_fd(sock, addr->sa_family, NULL)) != EV_SUCCESS)
    {
        return ret;
    }

    sock->backend.u.client.cb = cb;
    sock->base.flags |= EV_TCP_CONNECTING;
    ev__tcp_active(sock);

    if ((ret = connect(sock->sock, addr, size)) == 0)
    {/* Connect success immediately */
        sock->backend.u.client.stat = EV_SUCCESS;
        ev__todo(loop, &sock->backend.u.client.token, _ev_tcp_to_connect);
        return EV_SUCCESS;
    }

    if (errno != EINPROGRESS)
    {
        sock->base.flags &= ~EV_TCP_CONNECTING;
        ev__tcp_deactive(sock);
        ret = ev__translate_sys_error(errno);
        goto err;
    }

    ev__io_add(loop, &sock->backend.io, EV_IO_OUT);

    return EV_SUCCESS;

err:
    _ev_tcp_close_fd(sock);
    return ret;
}
