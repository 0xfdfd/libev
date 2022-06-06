#include "ev/errno.h"
#include "loop_unix.h"
#include "stream_unix.h"
#include "misc_unix.h"
#include "tcp_unix.h"
#include "handle.h"
#include <sys/uio.h>
#include <assert.h>
#include <unistd.h>

static void _ev_tcp_close_fd(ev_tcp_t* sock)
{
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        close(sock->sock);
        sock->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_tcp_cleanup_listen_queue(ev_tcp_t* s_sock, int stat)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&s_sock->backend.u.listen.accept_queue)) != NULL)
    {
        ev_tcp_t* c_sock = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.accept_node);
        c_sock->backend.u.accept.cb(s_sock, c_sock, stat);
    }
}

static int _ev_tcp_is_listening_active(ev_tcp_t* sock)
{
    return (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
        && (ev_list_size(&sock->backend.u.listen.accept_queue) != 0);
}

static int _ev_tcp_is_streaming_active(ev_tcp_t* sock)
{
    return (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
        && (ev__nonblock_stream_size(&sock->backend.u.stream, EV_IO_IN | EV_IO_OUT) != 0);
}

/**
 * @brief De-active socket if no active event
 */
static void _ev_tcp_smart_deactive(ev_tcp_t* sock)
{
    int flag_have_events = 0;
    if (_ev_tcp_is_listening_active(sock))
    {
        flag_have_events = 1;
    }
    if (_ev_tcp_is_streaming_active(sock))
    {
        flag_have_events = 1;
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        flag_have_events = 1;
    }

    if (!flag_have_events)
    {
        ev__handle_deactive(&sock->base);
    }
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);

    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        ev__nonblock_stream_exit(&sock->backend.u.stream);
        sock->base.data.flags &= ~EV_HANDLE_TCP_STREAMING;
    }

    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        _ev_tcp_cleanup_listen_queue(sock, EV_ECANCELED);
        sock->base.data.flags &= ~EV_HANDLE_TCP_LISTING;
    }

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        sock->backend.u.client.cb(sock, EV_ECANCELED);
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    }

    _ev_tcp_smart_deactive(sock);
    if (sock->close_cb != NULL)
    {
        sock->close_cb(sock);
    }
}

static void _ev_tcp_on_connect(ev_tcp_t* sock)
{
    int ret;
    socklen_t result_len = sizeof(ret);

    ev__nonblock_io_del(sock->base.data.loop, &sock->backend.u.client.io, EV_IO_OUT);
    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;

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

static void _ev_tcp_w_user_callback_unix(ev_tcp_write_req_t* req, size_t size, int stat)
{
    ev__write_exit(&req->base);
    req->user_callback(req, size, stat);
}

static void _ev_tcp_r_user_callback_unix(ev_tcp_read_req_t* req, size_t size, int stat)
{
    ev__read_exit(&req->base);
    req->user_callback(req, size, stat);
}

static void _on_tcp_write_done(ev_nonblock_stream_t* stream, ev_write_t* req, size_t size, int stat)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(stream, ev_tcp_t, backend.u.stream);
    _ev_tcp_smart_deactive(sock);

    ev_tcp_write_req_t* w_req = EV_CONTAINER_OF(req, ev_tcp_write_req_t, base);
    _ev_tcp_w_user_callback_unix(w_req, size, stat);
}

static void _on_tcp_read_done(ev_nonblock_stream_t* stream, ev_read_t* req, size_t size, int stat)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(stream, ev_tcp_t, backend.u.stream);
    _ev_tcp_smart_deactive(sock);

    ev_tcp_read_req_t* r_req = EV_CONTAINER_OF(req, ev_tcp_read_req_t, base);
    _ev_tcp_r_user_callback_unix(r_req, size, stat);
}

static void _ev_tcp_on_accept(ev_tcp_t* acpt)
{
    ev_list_node_t* it = ev_list_pop_front(&acpt->backend.u.listen.accept_queue);
    if (it == NULL)
    {
        BREAK_ABORT();
    }

    ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.accept_node);
    _ev_tcp_close_fd(conn);

    do
    {
        conn->sock = accept(acpt->sock, NULL, NULL);
    } while (conn->sock == -1 && errno == EINTR);

    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;
    _ev_tcp_smart_deactive(conn);

    int ret = conn->sock >= 0 ? EV_SUCCESS : ev__translate_sys_error(errno);
    if (ret == EV_SUCCESS)
    {/* Set non-block mode */
        if ((ret = ev__nonblock(conn->sock, 1)) != EV_SUCCESS)
        {
            _ev_tcp_close_fd(conn);
        }
    }

    conn->backend.u.accept.cb(acpt, conn, ret);

    /* might be close in callback */
    if (ev__handle_is_closing(&acpt->base))
    {
        return;
    }
    if (ev_list_size(&acpt->backend.u.listen.accept_queue) == 0)
    {
        ev__nonblock_io_del(acpt->base.data.loop, &acpt->backend.u.listen.io, EV_IO_IN);
    }
}

static void _ev_tcp_on_server_event(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_tcp_t* sock = EV_CONTAINER_OF(io, ev_tcp_t, backend.u.listen.io);

    _ev_tcp_on_accept(sock);
    _ev_tcp_smart_deactive(sock);
}

static void _ev_tcp_on_client_event(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_tcp_t* sock = EV_CONTAINER_OF(io, ev_tcp_t, backend.u.client.io);

    _ev_tcp_on_connect(sock);
    _ev_tcp_smart_deactive(sock);
}

/**
 * @return #ev_errno_t
 */
static int _ev_tcp_setup_fd(ev_tcp_t* sock, int domain, int is_server, int* new_fd)
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
    if (is_server)
    {
        ev__nonblock_io_init(&sock->backend.u.listen.io, sock->sock, _ev_tcp_on_server_event, NULL);
    }
    else
    {
        ev__nonblock_io_init(&sock->backend.u.client.io, sock->sock, _ev_tcp_on_client_event, NULL);
    }

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
    return sock->base.data.flags & EV_HANDLE_TCP_LISTING;
}

static void _ev_tcp_to_connect(ev_todo_token_t* todo)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(todo, ev_tcp_t, backend.u.client.token);

    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    sock->backend.u.client.cb(sock, EV_SUCCESS);
}

static void _ev_tcp_setup_stream_once(ev_tcp_t* sock)
{
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        return;
    }
    ev__nonblock_stream_init(sock->base.data.loop, &sock->backend.u.stream, sock->sock,
        _on_tcp_write_done, _on_tcp_read_done);
    sock->base.data.flags |= EV_HANDLE_TCP_STREAMING;
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* sock)
{
    ev__handle_init(loop, &sock->base, EV_ROLE_EV_TCP, _ev_tcp_on_close);
    sock->close_cb = NULL;
    sock->sock = EV_OS_SOCKET_INVALID;

    return EV_SUCCESS;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    /* Stop all pending IO actions */
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        ev__nonblock_stream_abort(&sock->backend.u.stream);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        ev__nonblock_io_del(sock->base.data.loop, &sock->backend.u.listen.io, EV_IO_IN);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        ev__nonblock_io_del(sock->base.data.loop, &sock->backend.u.client.io, EV_IO_OUT);
    }

    /* Close fd */
    _ev_tcp_close_fd(sock);

    sock->close_cb = cb;
    ev__handle_exit(&sock->base, 0);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_fd;
    if ((ret = _ev_tcp_setup_fd(tcp, addr->sa_family, 1, &flag_new_fd)) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = bind(tcp->sock, addr, addrlen)) != 0)
    {
        ret = ev__translate_sys_error(errno);
        goto err_bind;
    }
    tcp->base.data.flags |= EV_HABDLE_TCP_BOUND;

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
    tcp->base.data.flags |= EV_HANDLE_TCP_LISTING;

    return EV_SUCCESS;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    if (conn->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        return EV_EINPROGRESS;
    }
    assert(cb != NULL);

    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb = cb;
    ev_list_push_back(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.accept_node);
    ev__nonblock_io_add(lisn->base.data.loop, &lisn->backend.u.listen.io, EV_IO_IN);

    ev__handle_active(&lisn->base);
    ev__handle_active(&conn->base);

    return EV_SUCCESS;
}

int ev_tcp_write(ev_tcp_t* sock, ev_tcp_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb)
{
    req->user_callback = cb;
    int ret = ev__write_init(&req->base, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    _ev_tcp_setup_stream_once(sock);

    ev__handle_active(&sock->base);
    return ev__nonblock_stream_write(&sock->backend.u.stream, &req->base);
}

int ev_tcp_read(ev_tcp_t* sock, ev_tcp_read_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_CONNECTING))
    {
        return EV_EINVAL;
    }

    req->user_callback = cb;
    int ret = ev__read_init(&req->base, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    _ev_tcp_setup_stream_once(sock);

    ev__handle_active(&sock->base);
    return ev__nonblock_stream_read(&sock->backend.u.stream, &req->base);
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

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_tcp_connect_cb cb)
{
    int ret;
    ev_loop_t* loop = sock->base.data.loop;

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        return EV_EINPROGRESS;
    }
    if (sock->base.data.flags & (EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING | EV_HANDLE_TCP_STREAMING))
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_tcp_setup_fd(sock, addr->sa_family, 0, NULL)) != EV_SUCCESS)
    {
        return ret;
    }

    sock->backend.u.client.cb = cb;
    sock->base.data.flags |= EV_HANDLE_TCP_CONNECTING;

    if ((ret = connect(sock->sock, addr, size)) == 0)
    {/* Connect success immediately */
        sock->backend.u.client.stat = EV_SUCCESS;
        ev_todo_submit(loop, &sock->backend.u.client.token, _ev_tcp_to_connect);
        return EV_SUCCESS;
    }

    if (errno != EINPROGRESS)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        ret = ev__translate_sys_error(errno);
        goto err;
    }

    ev__handle_active(&sock->base);
    ev__nonblock_io_add(loop, &sock->backend.u.client.io, EV_IO_OUT);

    return EV_SUCCESS;

err:
    _ev_tcp_close_fd(sock);
    return ret;
}

int ev__tcp_open(ev_tcp_t* tcp, int fd)
{
    int busy_flags = EV_HANDLE_TCP_LISTING | EV_HANDLE_TCP_ACCEPTING |
            EV_HANDLE_TCP_STREAMING | EV_HANDLE_TCP_CONNECTING | EV_HABDLE_TCP_BOUND;
    if (tcp->base.data.flags & busy_flags)
    {
        return EV_EBUSY;
    }

    tcp->sock = fd;
    _ev_tcp_setup_stream_once(tcp);

    return EV_SUCCESS;
}
