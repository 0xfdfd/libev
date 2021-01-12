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
	closesocket(sock->backend.sock);
	sock->backend.sock = INVALID_SOCKET;
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
	ev_tcp_t* sock = container_of(handle, ev_tcp_t, base);
	if (sock->backend.sock != INVALID_SOCKET)
	{
		_ev_tcp_close_socket(sock);
	}
	if (sock->close_cb != NULL)
	{
		sock->close_cb(sock);
	}
}

static int _ev_tcp_init_backend(ev_tcp_t* tcp)
{
	tcp->backend.sock	= INVALID_SOCKET;
	tcp->backend.af		= AF_INET6;
	return EV_SUCCESS;
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

	sock->backend.sock = os_sock;
	sock->backend.af = af;

	return EV_SUCCESS;

err:
	ret = ev__translate_sys_error(WSAGetLastError());
	if (os_sock != INVALID_SOCKET)
	{
		closesocket(os_sock);
	}
	return ret;
}

static void _ev_tcp_on_listen_win(ev_iocp_t* req, size_t transferred)
{
	(void)req;
	(void)transferred;
	assert(0);	// Should not go into here
}

static void _ev_setup_listen(ev_tcp_t* sock)
{
	ev_list_init(&sock->backend.u.listen.accept_queue);
	ev__iocp_init(&sock->backend.u.listen.io, _ev_tcp_on_listen_win);
}

static void _ev_tcp_on_accept_win(ev_iocp_t* req, size_t transferred)
{
	(void)transferred;
	ev_tcp_t* conn = container_of(req, ev_tcp_t, backend.u.accept.io);
	ev_tcp_t* lisn = conn->backend.u.accept.listen;

	ev_list_erase(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.node);
	lisn->backend.u.accept.cb(lisn, conn, EV_SUCCESS);
}

static int _ev_tcp_get_connectex(ev_tcp_t* sock, LPFN_CONNECTEX* fn)
{
	int ret;
	DWORD bytes;
	GUID wsaid = WSAID_CONNECTEX;

	ret = WSAIoctl(sock->sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &wsaid, sizeof(wsaid), fn, sizeof(*fn), &bytes, NULL, NULL);
	return ret == SOCKET_ERROR ? EV_UNKNOWN : EV_SUCCESS;
}

static void _ev_tcp_on_connect_win(ev_iocp_t* req, size_t transferred)
{
	(void)transferred;

	int ret = EV_SUCCESS;
	ev_tcp_t* sock = container_of(req, ev_tcp_t, backend.io);

	if (NT_SUCCESS(req->overlapped.Internal))
	{
		if (setsockopt(sock->sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) != 0)
		{
			ret = WSAGetLastError();
		}
	}
	else
	{
		ret = ev__ntstatus_to_winsock_error((NTSTATUS)req->overlapped.Internal);
	}

	ret = ev__translate_sys_error(ret);
	sock->backend.u.conn.cb(sock, ret);
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

static void _ev_tcp_on_stream_done(ev_todo_t* todo)
{
	ev_list_node_t* it;
	ev_tcp_t* sock = container_of(todo, ev_tcp_t, backend.u.stream.token);
	sock->backend.u.stream.mask.todo_pending = 0;

	while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue_done)) != NULL)
	{
		ev_write_t* req = container_of(it, ev_write_t, node);
		req->data.cb(req, req->backend.size, req->backend.stat);
	}
}

static void _ev_tcp_submit_stream_todo(ev_tcp_t* sock)
{
	if (sock->backend.u.stream.mask.todo_pending)
	{
		return;
	}

	ev__todo(sock->base.loop, &sock->backend.u.stream.token, _ev_tcp_on_stream_done);
	sock->backend.u.stream.mask.todo_pending = 1;
}

static void _ev_tcp_on_stream_write_done(ev_iocp_t* iocp, size_t transferred)
{
	ev_write_t* wreq = container_of(iocp, ev_write_t, backend.io);
	wreq->backend.size = transferred;
	wreq->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
		EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

	ev_tcp_t* sock = wreq->backend.owner;

	ev_list_erase(&sock->backend.u.stream.w_queue, &wreq->node);
	ev_list_push_back(&sock->backend.u.stream.w_queue_done, &wreq->node);

	_ev_tcp_submit_stream_todo(sock);
}

static void _ev_tcp_setup_stream_win(ev_tcp_t* sock)
{
	ev_list_init(&sock->backend.u.stream.r_queue);
	ev_list_init(&sock->backend.u.stream.r_queue_done);
	ev_list_init(&sock->backend.u.stream.w_queue);
	ev_list_init(&sock->backend.u.stream.w_queue_done);
	memset(&sock->backend.u.stream.mask, 0, sizeof(sock->backend.u.stream.mask));
	sock->base.flags |= EV_TCP_STREAMING;
}

static void _ev_tcp_process_direct_write_success(ev_tcp_t* sock, ev_write_t* req, ev_buf_t bufs[], size_t nbuf)
{
	req->backend.size = 0;

	size_t i;
	for (i = 0; i < nbuf; i++)
	{
		req->backend.size += bufs[i].size;
	}
	req->backend.stat = EV_SUCCESS;

	ev_list_push_back(&sock->backend.u.stream.w_queue_done, &req->node);
	_ev_tcp_submit_stream_todo(sock);
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
	return _ev_tcp_init_backend(tcp);
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
	sock->close_cb = cb;
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

	if (tcp->backend.sock == INVALID_SOCKET)
	{
		if ((ret = _ev_tcp_setup_sock(tcp, addr->sa_family, 1)) != EV_SUCCESS)
		{
			return ret;
		}
		flag_new_socket = 1;
	}

	if ((ret = bind(tcp->backend.sock, addr, (int)addrlen)) == SOCKET_ERROR)
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

int ev_tcp_listen(ev_tcp_t* tcp, int backlog)
{
	if (tcp->base.flags & EV_TCP_LISTING)
	{
		return EV_EADDRINUSE;
	}

	int ret;
	if ((ret = listen(tcp->backend.sock, backlog)) == SOCKET_ERROR)
	{
		return ev__translate_sys_error(WSAGetLastError());
	}
	tcp->base.flags |= EV_TCP_LISTING;

	return EV_SUCCESS;
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_accept_cb cb)
{
	int ret;
	int flag_new_sock = 0;
	if (conn->backend.sock == INVALID_SOCKET)
	{
		if ((ret = _ev_tcp_setup_sock(conn, lisn->backend.af, 1)) != EV_SUCCESS)
		{
			goto err;
		}
		ev__iocp_init(&conn->backend.u.accept.io, _ev_tcp_on_accept_win);
		flag_new_sock = 1;
	}
	conn->backend.u.accept.listen = lisn;

	DWORD bytes = 0;
	ret = AcceptEx(lisn->backend.sock, conn->backend.sock,
		NULL, 0, 0, 0, &bytes, &conn->backend.u.accept.io.overlapped);
	if (!ret)
	{
		ret = ev__translate_sys_error(WSAGetLastError());
		goto err;
	}

	conn->backend.u.accept.cb = cb;
	ev_list_push_back(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.node);
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

	if ((ret = getsockname(sock->backend.sock, name, &socklen)) == SOCKET_ERROR)
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

	if ((ret = getpeername(sock->backend.sock, name, &socklen)) == SOCKET_ERROR)
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

	if (sock->sock == INVALID_SOCKET)
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

	ev__iocp_init(&sock->backend.io, _ev_tcp_on_connect_win);

	sock->backend.u.conn.cb = cb;
	if ((ret = _ev_tcp_get_connectex(sock, &sock->backend.u.conn.fn_connectex)) != EV_SUCCESS)
	{
		goto err;
	}

	DWORD bytes;
	if (!sock->backend.u.conn.fn_connectex(sock->sock, addr, (int)size,
		NULL, 0, &bytes, &sock->backend.io.overlapped))
	{
		ret = ev__translate_sys_error(WSAGetLastError());
		goto err;
	}

	sock->base.flags |= EV_TCP_CONNECTING;

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

	if (!(sock->base.flags & EV_TCP_STREAMING))
	{
		_ev_tcp_setup_stream_win(sock);
	}

	req->data.cb = cb;
	req->data.bufs = bufs;
	req->data.nbuf = nbuf;
	req->backend.owner = sock;
	ev__iocp_init(&req->backend.io, _ev_tcp_on_stream_write_done);

	DWORD sent_count;
	ret = WSASend(sock->sock, (WSABUF*)bufs, (DWORD)nbuf,
		&sent_count, 0, &req->backend.io.overlapped, NULL);
	if (ret == 0)
	{
		_ev_tcp_process_direct_write_success(sock, req, bufs, nbuf);
		return EV_SUCCESS;
	}

	ret = WSAGetLastError();
	if (ret != WSA_IO_PENDING)
	{
		return ev__translate_sys_error(ret);
	}
	ev_list_push_back(&sock->backend.u.stream.w_queue, &req->node);

	return EV_SUCCESS;
}
