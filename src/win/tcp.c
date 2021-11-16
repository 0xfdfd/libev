#include <assert.h>
#include <winsock2.h>
#include <mswsock.h>
#include "loop.h"
#include "tcp.h"

tcp_ctx_t g_tcp_ctx;

static void _ev_tcp_on_init(void)
{
    int ret;
    WSADATA wsa_data;
    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0)
    {
        assert(ret == 0);
    }

    if ((ret = ev_ipv4_addr("0.0.0.0", 0, &g_tcp_ctx.addr_any_ip4)) != EV_SUCCESS)
    {
        assert(ret == EV_SUCCESS);
    }

    if ((ret = ev_ipv6_addr("::", 0, &g_tcp_ctx.addr_any_ip6)) != EV_SUCCESS)
    {
        assert(ret == EV_SUCCESS);
    }
}

static void _ev_tcp_close_socket(ev_tcp_t* sock)
{
    closesocket(sock->sock);
    sock->sock = EV_OS_SOCKET_INVALID;
}

static void _ev_tcp_cleanup_accept(ev_tcp_t* sock)
{
    ev_tcp_t* lisn = sock->backend.u.accept.listen;
    sock->backend.u.accept.listen = NULL;

    if (sock->backend.u.accept.stat == EV_EINPROGRESS)
    {
        sock->backend.u.accept.stat = EV_ECANCELED;
        ev_list_erase(&lisn->backend.u.listen.a_queue, &sock->backend.u.accept.node);
    }
    else
    {
        ev_list_erase(&lisn->backend.u.listen.a_queue_done, &sock->backend.u.accept.node);
    }

    sock->base.flags &= ~EV_TCP_ACCEPTING;
    ev__tcp_deactive(sock);

    sock->backend.u.accept.cb(lisn, sock, sock->backend.u.accept.stat);
}

static void _ev_tcp_cleanup_listen(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue)) != NULL)
    {
        ev_tcp_t* conn = container_of(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_close_socket(conn);
        conn->base.flags &= ~EV_TCP_ACCEPTING;
        ev__tcp_deactive(conn);
        conn->backend.u.accept.cb(sock, conn, EV_ECANCELED);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue_done)) != NULL)
    {
        ev_tcp_t* conn = container_of(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_close_socket(conn);
        conn->base.flags &= ~EV_TCP_ACCEPTING;
        ev__tcp_deactive(conn);
        conn->backend.u.accept.cb(sock, conn, conn->backend.u.accept.stat);
    }
}

static void _ev_tcp_cleanup_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        req->data.cb(req, req->backend.size, req->backend.stat);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        req->data.cb(req, 0, EV_ECANCELED);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        req->data.cb(req, req->backend.size, req->backend.stat);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        req->data.cb(req, 0, EV_ECANCELED);
    }
}

static void _ev_tcp_cleanup_connect(ev_tcp_t* sock)
{
    if (sock->backend.u.client.stat == EV_EINPROGRESS)
    {
        sock->backend.u.client.stat = EV_ECANCELED;
    }
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
    ev_tcp_t* sock = container_of(handle, ev_tcp_t, base);
    if (sock->sock != INVALID_SOCKET)
    {
        _ev_tcp_close_socket(sock);
    }

    if (sock->base.flags & EV_TCP_LISTING)
    {
        sock->base.flags &= ~EV_TCP_LISTING;
        _ev_tcp_cleanup_listen(sock);
    }
    if (sock->base.flags & EV_TCP_ACCEPTING)
    {
        sock->base.flags &= ~EV_TCP_ACCEPTING;
        _ev_tcp_cleanup_accept(sock);
    }
    if (sock->base.flags & EV_TCP_STREAMING)
    {
        sock->base.flags &= ~EV_TCP_STREAMING;
        _ev_tcp_cleanup_stream(sock);
    }
    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        sock->base.flags &= ~EV_TCP_CONNECTING;
        _ev_tcp_cleanup_connect(sock);
    }

    if (sock->close_cb != NULL)
    {
        sock->close_cb(sock);
    }
}

static int _ev_tcp_get_connectex(ev_tcp_t* sock, LPFN_CONNECTEX* fn)
{
    int ret;
    DWORD bytes;
    GUID wsaid = WSAID_CONNECTEX;

    ret = WSAIoctl(sock->sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &wsaid, sizeof(wsaid), fn, sizeof(*fn), &bytes, NULL, NULL);
    return ret == SOCKET_ERROR ? EV_UNKNOWN : EV_SUCCESS;
}

static int _ev_tcp_setup_sock(ev_tcp_t* sock, int af, int with_iocp)
{
    int ret;

    SOCKET os_sock = socket(af, SOCK_STREAM, 0);
    if (os_sock == INVALID_SOCKET)
    {
        goto err;
    }

    u_long yes = 1;
    if (ioctlsocket(os_sock, FIONBIO, &yes) == SOCKET_ERROR)
    {
        goto err;
    }

    if (!SetHandleInformation((HANDLE)os_sock, HANDLE_FLAG_INHERIT, 0))
    {
        goto err;
    }

    HANDLE iocp = sock->base.loop->backend.iocp;
    if (with_iocp && CreateIoCompletionPort((HANDLE)os_sock, iocp, os_sock, 0) == 0)
    {
        goto err;
    }

    sock->sock = os_sock;
    sock->backend.af = af;

    return EV_SUCCESS;

err:
    ret = ev__translate_sys_error(WSAGetLastError());
    if (os_sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(os_sock);
    }
    return ret;
}

static void _ev_tcp_setup_listen_win(ev_tcp_t* sock)
{
    ev_list_init(&sock->backend.u.listen.a_queue);
    ev_list_init(&sock->backend.u.listen.a_queue_done);
    sock->base.flags |= EV_TCP_LISTING;
}

static void _ev_tcp_setup_accept_win(ev_tcp_t* lisn, ev_tcp_t* conn, ev_accept_cb cb)
{
    conn->backend.u.accept.cb = cb;
    conn->backend.u.accept.listen = lisn;
    conn->backend.u.accept.stat = EV_EINPROGRESS;
    conn->base.flags |= EV_TCP_ACCEPTING;
}

static int _ev_tcp_setup_client_win(ev_tcp_t* sock, ev_connect_cb cb)
{
    int ret;
    if ((ret = _ev_tcp_get_connectex(sock, &sock->backend.u.client.fn_connectex)) != EV_SUCCESS)
    {
        return ret;
    }

    sock->backend.u.client.stat = EV_EINPROGRESS;
    sock->backend.u.client.cb = cb;
    sock->base.flags |= EV_TCP_CONNECTING;

    return EV_SUCCESS;
}

static void _ev_tcp_setup_stream_win(ev_tcp_t* sock)
{
    ev_list_init(&sock->backend.u.stream.r_queue);
    ev_list_init(&sock->backend.u.stream.r_queue_done);
    ev_list_init(&sock->backend.u.stream.w_queue);
    ev_list_init(&sock->backend.u.stream.w_queue_done);
    sock->base.flags |= EV_TCP_STREAM_INIT;
}

static void _ev_tcp_deactive_stream(ev_tcp_t* sock)
{
    if (ev_list_size(&sock->backend.u.stream.r_queue) == 0
        && ev_list_size(&sock->backend.u.stream.r_queue_done) == 0
        && ev_list_size(&sock->backend.u.stream.w_queue) == 0
        && ev_list_size(&sock->backend.u.stream.w_queue_done) == 0)
    {
        sock->base.flags &= ~EV_TCP_STREAMING;
        ev__tcp_deactive(sock);
    }
}

static void _ev_tcp_process_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;

    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        req->data.cb(req, req->backend.size, req->backend.stat);
    }

    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        req->data.cb(req, req->backend.size, req->backend.stat);
    }

    _ev_tcp_deactive_stream(sock);
}

static void _ev_tcp_process_accept(ev_tcp_t* conn)
{
    ev_tcp_t* lisn = conn->backend.u.accept.listen;
    ev_list_erase(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);

    conn->base.flags &= ~EV_TCP_ACCEPTING;
    ev__tcp_deactive(conn);

    conn->backend.u.accept.cb(lisn, conn, conn->backend.u.accept.stat);
}

static void _ev_tcp_process_connect(ev_tcp_t* sock)
{
    int ret;
    if (sock->backend.u.client.stat == EV_SUCCESS)
    {
        if ((ret = setsockopt(sock->sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0)) == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            sock->backend.u.client.stat = ev__translate_sys_error(ret);
        }
    }

    sock->base.flags &= ~EV_TCP_CONNECTING;
    ev__tcp_deactive(sock);

    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_task_done(ev_todo_t* todo)
{
    ev_tcp_t* sock = container_of(todo, ev_tcp_t, backend.token);
    sock->backend.mask.todo_pending = 0;

    if (sock->base.flags & EV_TCP_STREAMING)
    {
        _ev_tcp_process_stream(sock);
    }
    if (sock->base.flags & EV_TCP_ACCEPTING)
    {
        _ev_tcp_process_accept(sock);
    }
    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        _ev_tcp_process_connect(sock);
    }
}

static void _ev_tcp_submit_stream_todo(ev_tcp_t* sock)
{
    if (sock->backend.mask.todo_pending)
    {
        return;
    }

    ev__todo(sock->base.loop, &sock->backend.token, _ev_tcp_on_task_done);
    sock->backend.mask.todo_pending = 1;
}

static void _ev_tcp_on_accept_win(ev_tcp_t* conn, size_t transferred)
{
    (void)transferred;
    ev_tcp_t* lisn = conn->backend.u.accept.listen;

    ev_list_erase(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);
    ev_list_push_back(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);

    conn->backend.u.accept.stat = NT_SUCCESS(conn->backend.io.overlapped.Internal) ?
        EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)conn->backend.io.overlapped.Internal));
    _ev_tcp_submit_stream_todo(conn);
}

static void _ev_tcp_on_connect_win(ev_tcp_t* sock, size_t transferred)
{
    (void)transferred;

    sock->backend.u.client.stat = NT_SUCCESS(sock->backend.io.overlapped.Internal) ?
        EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)sock->backend.io.overlapped.Internal));
    _ev_tcp_submit_stream_todo(sock);
}

static int _ev_tcp_bind_any_addr(ev_tcp_t* sock, int af)
{
    size_t name_len;
    const struct sockaddr* bind_addr;

    switch (af)
    {
    case AF_INET:
        bind_addr = (struct sockaddr*)&g_tcp_ctx.addr_any_ip4;
        name_len = sizeof(g_tcp_ctx.addr_any_ip4);
        break;

    case AF_INET6:
        bind_addr = (struct sockaddr*)&g_tcp_ctx.addr_any_ip6;
        name_len = sizeof(g_tcp_ctx.addr_any_ip6);
        break;

    default:
        return EV_EINVAL;
    }

    return ev_tcp_bind(sock, bind_addr, name_len);
}

static void _ev_tcp_on_stream_write_done(ev_iocp_t* iocp, size_t transferred)
{
    ev_write_t* req = container_of(iocp, ev_write_t, backend.io);
    ev_tcp_t* sock = req->backend.owner;

    req->backend.size = transferred;
    req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
        EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

    ev_list_erase(&sock->backend.u.stream.w_queue, &req->node);
    ev_list_push_back(&sock->backend.u.stream.w_queue_done, &req->node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_stream_read_done(ev_iocp_t* iocp, size_t transferred)
{
    ev_read_t* req = container_of(iocp, ev_read_t, backend.io);
    ev_tcp_t* sock = req->backend.owner;

    req->backend.size = transferred;
    if (transferred == 0)
    {/* Zero recv means peer close */
        req->backend.stat = EV_EOF;
    }
    else
    {
        req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
            EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
    }

    ev_list_erase(&sock->backend.u.stream.r_queue, &req->node);
    ev_list_push_back(&sock->backend.u.stream.r_queue_done, &req->node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_iocp(ev_iocp_t* req, size_t transferred)
{
    ev_tcp_t* sock = container_of(req, ev_tcp_t, backend.io);

    if (sock->base.flags & EV_TCP_ACCEPTING)
    {
        _ev_tcp_on_accept_win(sock, transferred);
    }
    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        _ev_tcp_on_connect_win(sock, transferred);
    }
}

void ev__tcp_init(void)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_tcp_on_init);
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp)
{
    ev__handle_init(loop, &tcp->base, _ev_tcp_on_close);
    tcp->close_cb = NULL;
    tcp->sock = EV_OS_SOCKET_INVALID;

    tcp->backend.af = AF_INET6;
    ev__iocp_init(&tcp->backend.io, _ev_tcp_on_iocp);
    memset(&tcp->backend.mask, 0, sizeof(tcp->backend.mask));

    return EV_SUCCESS;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    sock->close_cb = cb;

    /* Close socket to avoid IOCP conflict with exiting process */
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        _ev_tcp_close_socket(sock);
    }

    /**
     * From this point, any complete IOCP operations should be in done_queue,
     * and any pending IOCP operations is canceled.
     */

    /* Ready to close socket */
    ev__handle_exit(&sock->base);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_socket = 0;

    if (tcp->base.flags & EV_TCP_BOUND)
    {
        return EV_EALREADY;
    }

    if (tcp->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(tcp, addr->sa_family, 1)) != EV_SUCCESS)
        {
            return ret;
        }
        flag_new_socket = 1;
    }

    if ((ret = bind(tcp->sock, addr, (int)addrlen)) == SOCKET_ERROR)
    {
        ret = ev__translate_sys_error(WSAGetLastError());
        goto err;
    }
    tcp->base.flags |= EV_TCP_BOUND;

    return EV_SUCCESS;

err:
    if (flag_new_socket)
    {
        _ev_tcp_close_socket(tcp);
    }
    return ret;
}

int ev_tcp_listen(ev_tcp_t* sock, int backlog)
{
    if (sock->base.flags & EV_TCP_LISTING)
    {
        return EV_EADDRINUSE;
    }

    int ret;
    if ((ret = listen(sock->sock, backlog)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }
    _ev_tcp_setup_listen_win(sock);

    return EV_SUCCESS;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_accept_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (conn->base.flags & EV_TCP_ACCEPTING)
    {
        return EV_EINPROGRESS;
    }

    if (conn->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(conn, lisn->backend.af, 1)) != EV_SUCCESS)
        {
            goto err;
        }
        flag_new_sock = 1;
    }
    _ev_tcp_setup_accept_win(lisn, conn, cb);
    ev__tcp_active(conn);

    DWORD bytes = 0;
    ret = AcceptEx(lisn->sock, conn->sock,
        conn->backend.u.accept.buffer, 0, sizeof(struct sockaddr_storage), sizeof(struct sockaddr_storage),
        &bytes, &conn->backend.io.overlapped);

    /* Accept success */
    if (ret)
    {
        conn->backend.u.accept.stat = EV_SUCCESS;
        ev_list_push_back(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);
        _ev_tcp_submit_stream_todo(lisn);
        return EV_SUCCESS;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        conn->base.flags &= ~EV_TCP_CONNECTING;
        ev__tcp_deactive(conn);
        return ev__translate_sys_error(ret);
    }
    conn->base.flags |= EV_TCP_ACCEPTING;
    ev_list_push_back(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);

    return EV_SUCCESS;

err:
    if (flag_new_sock)
    {
        _ev_tcp_close_socket(conn);
    }
    return ret;
}

int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    int socklen = (int)*len;

    if ((ret = getsockname(sock->sock, name, &socklen)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }

    *len = socklen;
    return EV_SUCCESS;
}

int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
    int ret;
    int socklen = (int)*len;

    if ((ret = getpeername(sock->sock, name, &socklen)) == SOCKET_ERROR)
    {
        return ev__translate_sys_error(WSAGetLastError());
    }

    *len = socklen;
    return EV_SUCCESS;
}

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (sock->base.flags & EV_TCP_CONNECTING)
    {
        return EV_EINPROGRESS;
    }

    if (sock->sock == EV_OS_SOCKET_INVALID)
    {
        if ((ret = _ev_tcp_setup_sock(sock, addr->sa_family, 1)) != EV_SUCCESS)
        {
            goto err;
        }
        flag_new_sock = 1;
    }

    if (!(sock->base.flags & EV_TCP_BOUND))
    {
        if ((ret = _ev_tcp_bind_any_addr(sock, addr->sa_family)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    if ((ret = _ev_tcp_setup_client_win(sock, cb)) != EV_SUCCESS)
    {
        goto err;
    }
    ev__tcp_active(sock);

    DWORD bytes;
    ret = sock->backend.u.client.fn_connectex(sock->sock, addr, (int)size,
        NULL, 0, &bytes, &sock->backend.io.overlapped);
    if (ret)
    {
        sock->backend.u.client.stat = EV_SUCCESS;
        _ev_tcp_submit_stream_todo(sock);
        return EV_SUCCESS;
    }

    ret = WSAGetLastError();
    if (ret != WSA_IO_PENDING)
    {
        sock->base.flags &= ~EV_TCP_CONNECTING;
        ev__tcp_deactive(sock);
        ret = ev__translate_sys_error(ret);
        goto err;
    }

    return EV_SUCCESS;

err:
    if (flag_new_sock)
    {
        _ev_tcp_close_socket(sock);
    }
    return ret;
}

int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
    int ret;
    ENSURE_LAYOUT(ev_buf_t, WSABUF, size, len, data, buf);

    if (!(sock->base.flags & EV_TCP_STREAM_INIT))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    req->data.cb = cb;
    req->data.bufs = bufs;
    req->data.nbuf = nbuf;
    req->backend.owner = sock;
    req->backend.size = 0;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_write_done);

    ev_list_push_back(&sock->backend.u.stream.w_queue, &req->node);
    sock->base.flags |= EV_TCP_STREAMING;
    ev__tcp_active(sock);

    ret = WSASend(sock->sock, (WSABUF*)bufs, (DWORD)nbuf,
        NULL, 0, &req->backend.io.overlapped, NULL);
    if (ret == 0)
    {
        /*
         * A result of zero means send successful, but the result will still go
         * through IOCP callback, so it is necessary to return directly.
         */
        return EV_SUCCESS;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        _ev_tcp_deactive_stream(sock);
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb)
{
    int ret;
    ENSURE_LAYOUT(ev_buf_t, WSABUF, size, len, data, buf);

    if (!(sock->base.flags & EV_TCP_STREAM_INIT))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    req->data.cb = cb;
    req->data.bufs = bufs;
    req->data.nbuf = nbuf;
    req->backend.owner = sock;
    req->backend.size = 0;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_read_done);

    ev_list_push_back(&sock->backend.u.stream.r_queue, &req->node);
    sock->base.flags |= EV_TCP_STREAMING;
    ev__tcp_active(sock);

    DWORD flags = 0;
    ret = WSARecv(sock->sock, (WSABUF*)bufs, (DWORD)nbuf,
        NULL, &flags, &req->backend.io.overlapped, NULL);
    if (ret == 0)
    {
        /*
         * A result of zero means recv successful, but the result will still go
         * through IOCP callback, so it is necessary to return directly.
         */
        return EV_SUCCESS;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        _ev_tcp_deactive_stream(sock);
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}
