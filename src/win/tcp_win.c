#include "ev.h"
#include "handle.h"
#include "loop_win.h"
#include "winsock.h"
#include "misc_win.h"
#include "tcp_win.h"
#include <WinSock2.h>
#include <assert.h>

static void _ev_tcp_close_socket(ev_tcp_t* sock)
{
    if (sock->sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(sock->sock);
        sock->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_tcp_finialize_accept(ev_tcp_t* conn)
{
    ev_tcp_t* lisn = conn->backend.u.accept.listen;
    conn->backend.u.accept.listen = NULL;

    if (conn->backend.u.accept.stat == EV_EINPROGRESS)
    {
        conn->backend.u.accept.stat = EV_ECANCELED;
        ev_list_erase(&lisn->backend.u.listen.a_queue, &conn->backend.u.accept.node);
    }
    else
    {
        ev_list_erase(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);
    }

    ev__handle_event_dec(&lisn->base);
    ev__handle_event_dec(&conn->base);

    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb(lisn, conn, conn->backend.u.accept.stat);
}

static void _ev_tcp_cleanup_connection_in_listen(ev_tcp_t* conn)
{
    ev_tcp_t* lisn = conn->backend.u.accept.listen;

    ev__handle_event_dec(&lisn->base);
    ev__handle_event_dec(&conn->base);

    _ev_tcp_close_socket(conn);
    conn->base.data.flags &= ~EV_HANDLE_TCP_ACCEPTING;
    conn->backend.u.accept.cb(lisn, conn, EV_ECANCELED);
}

static void _ev_tcp_cleanup_listen(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue)) != NULL)
    {
        ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_cleanup_connection_in_listen(conn);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.listen.a_queue_done)) != NULL)
    {
        ev_tcp_t* conn = EV_CONTAINER_OF(it, ev_tcp_t, backend.u.accept.node);
        _ev_tcp_cleanup_connection_in_listen(conn);
    }
}

static void _ev_tcp_w_user_callback_win(ev_tcp_t* sock,
    ev_tcp_write_req_t* req, size_t size, int stat)
{
    ev__handle_event_dec(&sock->base);
    ev__write_exit(&req->base);
    req->user_callback(req, size, stat);
}

static void _ev_tcp_r_user_callbak_win(ev_tcp_t* sock,
    ev_tcp_read_req_t* req, size_t size, int stat)
{
    ev__handle_event_dec(&sock->base);
    ev__read_exit(&req->base);
    req->user_callback(req, size, stat);
}

static void _ev_tcp_cleanup_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        _ev_tcp_r_user_callbak_win(sock, req, req->base.data.size, req->backend.stat);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        _ev_tcp_r_user_callbak_win(sock, req, 0, EV_ECANCELED);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        _ev_tcp_w_user_callback_win(sock, req, req->base.size, req->backend.stat);
    }
    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        _ev_tcp_w_user_callback_win(sock, req, 0, EV_ECANCELED);
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

static void _ev_tcp_on_close_win(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);

    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_LISTING;
        _ev_tcp_cleanup_listen(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_finialize_accept(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_STREAMING;
        _ev_tcp_cleanup_stream(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
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

    ret = WSAIoctl(sock->sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &wsaid, sizeof(wsaid), fn, sizeof(*fn), &bytes, NULL, NULL);
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
    sock->base.data.flags |= EV_HANDLE_TCP_LISTING;
}

static void _ev_tcp_setup_accept_win(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    conn->backend.u.accept.cb = cb;
    conn->backend.u.accept.listen = lisn;
    conn->backend.u.accept.stat = EV_EINPROGRESS;
    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
}

static int _ev_tcp_setup_client_win(ev_tcp_t* sock, ev_tcp_connect_cb cb)
{
    int ret;
    if ((ret = _ev_tcp_get_connectex(sock, &sock->backend.u.client.fn_connectex)) != EV_SUCCESS)
    {
        return ret;
    }

    sock->backend.u.client.stat = EV_EINPROGRESS;
    sock->backend.u.client.cb = cb;
    sock->base.data.flags |= EV_HANDLE_TCP_CONNECTING;

    return EV_SUCCESS;
}

static void _ev_tcp_setup_stream_win(ev_tcp_t* sock)
{
    ev_list_init(&sock->backend.u.stream.r_queue);
    ev_list_init(&sock->backend.u.stream.r_queue_done);
    ev_list_init(&sock->backend.u.stream.w_queue);
    ev_list_init(&sock->backend.u.stream.w_queue_done);
    sock->base.data.flags |= EV_HANDLE_TCP_STREAMING;
}

static void _ev_tcp_process_stream(ev_tcp_t* sock)
{
    ev_list_node_t* it;

    while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
    {
        ev_tcp_write_req_t* req = EV_CONTAINER_OF(it, ev_tcp_write_req_t, base.node);
        size_t write_size = req->base.size;
        int write_stat = req->backend.stat;

        _ev_tcp_w_user_callback_win(sock, req, write_size, write_stat);
    }

    while ((it = ev_list_pop_front(&sock->backend.u.stream.r_queue_done)) != NULL)
    {
        ev_tcp_read_req_t* req = EV_CONTAINER_OF(it, ev_tcp_read_req_t, base.node);
        size_t read_size = req->base.data.size;
        int read_stat = req->backend.stat;

        _ev_tcp_r_user_callbak_win(sock, req, read_size, read_stat);
    }
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

    ev__handle_event_dec(&sock->base);

    sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
    sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_task_done(ev_handle_t* handle)
{
    ev_tcp_t* sock = EV_CONTAINER_OF(handle, ev_tcp_t, base);
    sock->backend.mask.todo_pending = 0;

    if (sock->base.data.flags & EV_HANDLE_TCP_STREAMING)
    {
        _ev_tcp_process_stream(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_finialize_accept(sock);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
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

    ev__backlog_submit(&sock->base, _ev_tcp_on_task_done);
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
        bind_addr = (struct sockaddr*)&ev_addr_ip4_any_;
        name_len = sizeof(ev_addr_ip4_any_);
        break;

    case AF_INET6:
        bind_addr = (struct sockaddr*)&ev_addr_ip6_any_;
        name_len = sizeof(ev_addr_ip6_any_);
        break;

    default:
        return EV_EINVAL;
    }

    return ev_tcp_bind(sock, bind_addr, name_len);
}

static void _ev_tcp_on_stream_write_done(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_tcp_write_req_t* req = arg;
    ev_tcp_t* sock = req->backend.owner;

    req->base.size = transferred;
    req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
        EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

    ev_list_erase(&sock->backend.u.stream.w_queue, &req->base.node);
    ev_list_push_back(&sock->backend.u.stream.w_queue_done, &req->base.node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_stream_read_done(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_tcp_read_req_t* req = arg;
    ev_tcp_t* sock = req->backend.owner;

    req->base.data.size = transferred;
    if (transferred == 0)
    {/* Zero recv means peer close */
        req->backend.stat = EV_EOF;
    }
    else
    {
        req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
            EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
    }

    ev_list_erase(&sock->backend.u.stream.r_queue, &req->base.node);
    ev_list_push_back(&sock->backend.u.stream.r_queue_done, &req->base.node);

    _ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_on_iocp(ev_iocp_t* req, size_t transferred, void* arg)
{
    (void)req;
    ev_tcp_t* sock = arg;

    if (sock->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
    {
        _ev_tcp_on_accept_win(sock, transferred);
    }
    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
    {
        _ev_tcp_on_connect_win(sock, transferred);
    }
}

static int _ev_tcp_init_write_req_win(ev_tcp_t* sock, ev_tcp_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb)
{
    int ret;
    if ((ret = ev__write_init(&req->base, bufs, nbuf)) != EV_SUCCESS)
    {
        return ret;
    }

    req->user_callback = cb;
    req->backend.owner = sock;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_write_done, req);

    return EV_SUCCESS;
}

static int _ev_tcp_init_read_req_win(ev_tcp_t* sock, ev_tcp_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    int ret;

    if ((ret = ev__read_init(&req->base, bufs, nbuf)) != EV_SUCCESS)
    {
        return ret;
    }

    req->user_callback = cb;
    req->backend.owner = sock;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_read_done, req);

    return EV_SUCCESS;
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp)
{
    ev__handle_init(loop, &tcp->base, EV_ROLE_EV_TCP);
    tcp->close_cb = NULL;
    tcp->sock = EV_OS_SOCKET_INVALID;

    tcp->backend.af = AF_INET6;
    ev__iocp_init(&tcp->backend.io, _ev_tcp_on_iocp, tcp);
    memset(&tcp->backend.mask, 0, sizeof(tcp->backend.mask));

    return EV_SUCCESS;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
    sock->close_cb = cb;

    /* Close socket to avoid IOCP conflict with exiting process */
    _ev_tcp_close_socket(sock);

    /**
     * From this point, any complete IOCP operations should be in done_queue,
     * and any pending IOCP operations is canceled.
     */

    /* Ready to close socket */
    ev__handle_exit(&sock->base, _ev_tcp_on_close_win);
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
    int ret;
    int flag_new_socket = 0;

    if (tcp->base.data.flags & EV_HABDLE_TCP_BOUND)
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
    tcp->base.data.flags |= EV_HABDLE_TCP_BOUND;

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
    if (sock->base.data.flags & EV_HANDLE_TCP_LISTING)
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

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_tcp_accept_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (conn->base.data.flags & EV_HANDLE_TCP_ACCEPTING)
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

    ev__handle_event_add(&lisn->base);
    ev__handle_event_add(&conn->base);

    DWORD bytes = 0;
    ret = AcceptEx(lisn->sock, conn->sock,
        conn->backend.u.accept.buffer, 0, sizeof(struct sockaddr_storage), sizeof(struct sockaddr_storage),
        &bytes, &conn->backend.io.overlapped);

    /* Accept success */
    if (ret)
    {
        conn->backend.u.accept.stat = EV_SUCCESS;
        ev_list_push_back(&lisn->backend.u.listen.a_queue_done, &conn->backend.u.accept.node);
        _ev_tcp_submit_stream_todo(conn);
        return EV_SUCCESS;
    }

    if ((ret = WSAGetLastError()) != WSA_IO_PENDING)
    {
        ev__handle_event_dec(&lisn->base);
        ev__handle_event_dec(&conn->base);
        conn->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        return ev__translate_sys_error(ret);
    }
    conn->base.data.flags |= EV_HANDLE_TCP_ACCEPTING;
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

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_tcp_connect_cb cb)
{
    int ret;
    int flag_new_sock = 0;

    if (sock->base.data.flags & EV_HANDLE_TCP_CONNECTING)
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

    if (!(sock->base.data.flags & EV_HABDLE_TCP_BOUND))
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
    ev__handle_event_add(&sock->base);

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
        sock->base.data.flags &= ~EV_HANDLE_TCP_CONNECTING;
        ev__handle_event_dec(&sock->base);
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

int ev_tcp_write(ev_tcp_t* sock, ev_tcp_write_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_tcp_write_cb cb)
{
    int ret;
    if ((ret = _ev_tcp_init_write_req_win(sock, req, bufs, nbuf, cb)) != EV_SUCCESS)
    {
        return ret;
    }

    if (!(sock->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    ev_list_push_back(&sock->backend.u.stream.w_queue, &req->base.node);
    ev__handle_event_add(&sock->base);

    ret = WSASend(sock->sock, (WSABUF*)req->base.bufs, (DWORD)req->base.nbuf,
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
        ev__handle_event_dec(&sock->base);
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_tcp_read(ev_tcp_t* sock, ev_tcp_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb)
{
    int ret;

    if ((ret = _ev_tcp_init_read_req_win(sock, req, bufs, nbuf, cb)) != EV_SUCCESS)
    {
        return ret;
    }

    if (!(sock->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(sock);
    }

    ev_list_push_back(&sock->backend.u.stream.r_queue, &req->base.node);
    ev__handle_event_add(&sock->base);

    DWORD flags = 0;
    ret = WSARecv(sock->sock, (WSABUF*)req->base.data.bufs, (DWORD)req->base.data.nbuf,
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
        ev__handle_event_dec(&sock->base);
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

API_LOCAL int ev__tcp_open_win(ev_tcp_t* tcp, SOCKET fd)
{
    tcp->sock = fd;
    if (!(tcp->base.data.flags & EV_HANDLE_TCP_STREAMING))
    {
        _ev_tcp_setup_stream_win(tcp);
    }

    return EV_SUCCESS;
}
