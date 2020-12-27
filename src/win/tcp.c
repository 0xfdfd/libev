#include <assert.h>
#include <winsock2.h>
#include <mswsock.h>
#include "loop.h"
#include "tcp.h"

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

static int _tcp_setup_sock(ev_tcp_t* sock, int af, int with_iocp)
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
	ret = ev__translate_sys_error_win(WSAGetLastError());
	if (os_sock != INVALID_SOCKET)
	{
		closesocket(os_sock);
	}
	return ret;
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
	int ret;
	int flag_new_socket = 0;

	if (tcp->backend.sock == INVALID_SOCKET)
	{
		if ((ret = _tcp_setup_sock(tcp, addr->sa_family, 1)) != EV_SUCCESS)
		{
			return ret;
		}
		flag_new_socket = 1;
	}

	if ((ret = bind(tcp->backend.sock, addr, (int)addrlen)) == SOCKET_ERROR)
	{
		ret = ev__translate_sys_error_win(WSAGetLastError());
		goto err;
	}

	return EV_SUCCESS;

err:
	if (flag_new_socket)
	{
		_ev_tcp_close_socket(tcp);
	}
	return ret;
}

static void _ev_tcp_on_listen_win(ev_iocp_t* req)
{
	(void)req;
	assert(0);	// Should not go into here
}

static void _ev_setup_listen(ev_tcp_t* sock)
{
	ev_list_init(&sock->backend.u.listen.accept_queue);
	ev__iocp_init(&sock->backend.u.listen.io, _ev_tcp_on_listen_win);
}

int ev_tcp_listen(ev_tcp_t* tcp, int backlog)
{
	if (listen(tcp->backend.sock, backlog) == SOCKET_ERROR)
	{
		return ev__translate_sys_error_win(WSAGetLastError());
	}
	return EV_SUCCESS;
}

static void _ev_tcp_on_accept_win(ev_iocp_t* req)
{
	ev_tcp_t* conn = container_of(req, ev_tcp_t, backend.u.accept.io);
	ev_tcp_t* lisn = conn->backend.u.accept.listen;

	ev_list_erase(&lisn->backend.u.listen.accept_queue, &conn->backend.u.accept.node);
	lisn->backend.u.accept.cb(lisn, conn, EV_SUCCESS);
}

int ev_tcp_accept(ev_tcp_t* lisn, ev_tcp_t* conn, ev_accept_cb cb)
{
	int ret;
	int flag_new_sock = 0;
	if (conn->backend.sock == INVALID_SOCKET)
	{
		if ((ret = _tcp_setup_sock(conn, lisn->backend.af, 0)) != EV_SUCCESS)
		{
			goto err;
		}
		ev__iocp_init(&conn->backend.u.accept.io, _ev_tcp_on_accept_win);
		flag_new_sock = 1;
	}
	conn->backend.u.accept.listen = lisn;

	ret = AcceptEx(lisn->backend.sock, conn->backend.sock,
		NULL, 0, 0, 0, 0, &conn->backend.u.accept.io.overlapped);
	if (!ret)
	{
		ret = ev__translate_sys_error_win(WSAGetLastError());
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

void ev__tcp_init(void)
{
	int ret;
	WSADATA wsa_data;

	if ((ret = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0)
	{
		assert(ret == 0);
	}
}
