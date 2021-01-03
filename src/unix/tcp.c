#include <sys/uio.h>
#include <assert.h>
#include <unistd.h>
#include "loop.h"

static void _ev_tcp_close_fd(ev_tcp_t* sock)
{
	if (sock->backend.sock != EV_INVALID_FD)
	{
		close(sock->backend.sock);
		sock->backend.sock = -1;
	}
}

static void _ev_tcp_on_close(ev_handle_t* handle)
{
	ev_tcp_t* sock = container_of(handle, ev_tcp_t, base);

	_ev_tcp_close_fd(sock);

	if (sock->close_cb != NULL)
	{
		sock->close_cb(sock);
	}
}

static int _ev_tcp_setup_fd(ev_tcp_t* tcp, int domain, int* new_fd)
{
	*new_fd = 0;
	if (tcp->backend.sock != EV_INVALID_FD)
	{
		return EV_SUCCESS;
	}
	if ((tcp->backend.sock = socket(domain, SOCK_STREAM, 0)) == -1)
	{
		return ev__translate_sys_error(errno);
	}

	int ret;
	if ((ret = ev__nonblock(tcp->backend.sock, 1)) != EV_SUCCESS)
	{
		goto err_nonblock;
	}

	*new_fd = 1;
	return EV_SUCCESS;

err_nonblock:
	close(tcp->backend.sock);
	tcp->backend.sock = EV_INVALID_FD;
	return ret;
}

static int _ev_tcp_is_listening(ev_tcp_t* sock)
{
	return sock->base.flags & EV_TCP_LISTING;
}

static void _ev_tcp_on_accept(ev_io_t* io, unsigned evts)
{
	(void)evts;
	ev_tcp_t* acpt = container_of(io, ev_tcp_t, backend.u.listen.io);

	ev_list_node_t* it = ev_list_pop_front(&acpt->backend.u.listen.accept_queue);
	if (it == NULL)
	{
		ABORT();
	}

	ev_tcp_t* conn = container_of(it, ev_tcp_t, backend.u.accept.accept_node);
	_ev_tcp_close_fd(conn);

	do 
	{
		conn->backend.sock = accept(acpt->backend.sock,
			(struct sockaddr*)&conn->backend.u.accept.peeraddr,
			&conn->backend.u.accept.addrlen);
	} while (conn->backend.sock == -1 && errno == EINTR);

	conn->base.flags &= ~EV_TCP_ACCEPTING;

	int ret = conn->backend.sock >= 0 ? EV_SUCCESS : ev__translate_sys_error(errno);
	conn->backend.u.accept.cb(acpt, conn, ret);

	if (ev_list_size(&acpt->backend.u.listen.accept_queue) == 0)
	{
		ev__io_del(acpt->base.loop, io, EV_IO_IN);
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

static void _ev_tcp_cleanup_all_write_request(ev_tcp_t* sock, int err)
{
	ev_list_node_t* it;
	while ((it = ev_list_pop_front(&sock->backend.u.stream.w_queue)) != NULL)
	{
		ev_write_t* req = container_of(it, ev_write_t, node);
		req->data.cb(req, req->info.len, err);
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
		r = readv(sock->backend.sock, (struct iovec*)req->data.bufs, req->data.nbuf);
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
		ev__io_del(sock->base.loop, &sock->backend.u.stream.io, EV_IO_IN);
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
		w = writev(sock->backend.sock,
			(struct iovec*)(req->data.bufs + req->info.idx), req->data.nbuf - req->info.idx);
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

	req->info.len += w;
	while (w > 0)
	{
		if ((size_t)w < req->data.bufs[req->info.idx].size)
		{
			req->data.bufs[req->info.idx].data =
				(void*)((uint8_t*)(req->data.bufs[req->info.idx].data) + w);
			req->data.bufs[req->info.idx].size -= w;
			break;
		}

		w -= req->data.bufs[req->info.idx].size;
		req->info.idx++;
		continue;
	}

	/* Write complete */
	if (req->info.idx == req->data.nbuf)
	{
		ev_list_erase(&sock->backend.u.stream.w_queue, it);
		req->data.cb(req, req->info.len, EV_SUCCESS);
	}

fin:
	if (ev_list_size(&sock->backend.u.stream.w_queue) == 0)
	{
		ev__io_del(sock->base.loop, &sock->backend.u.stream.io, EV_IO_OUT);
	}
}

static void _ev_tcp_on_stream(ev_io_t* io, unsigned evts)
{
	ev_tcp_t* sock = container_of(io, ev_tcp_t, backend.u.stream.io);
	if (evts & EV_IO_IN)
	{
		_ev_tcp_do_read(sock);
	}
	if (evts & EV_IO_OUT)
	{
		_ev_tcp_do_write(sock);
	}
}

static void _ev_tcp_setup_stream(ev_tcp_t* sock)
{
	ev__io_init(&sock->backend.u.stream.io, sock->backend.sock, _ev_tcp_on_stream);
	ev_list_init(&sock->backend.u.stream.w_queue);
	ev_list_init(&sock->backend.u.stream.r_queue);
}

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp)
{
	ev__handle_init(loop, &tcp->base, _ev_tcp_on_close);
	tcp->close_cb = NULL;
	tcp->backend.sock = EV_INVALID_FD;

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

	if ((ret = bind(tcp->backend.sock, addr, addrlen)) != 0)
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
	if ((ret = listen(tcp->backend.sock, backlog)) != 0)
	{
		return ev__translate_sys_error(errno);
	}

	ev_list_init(&tcp->backend.u.listen.accept_queue);
	ev__io_init(&tcp->backend.u.listen.io, tcp->backend.sock, _ev_tcp_on_accept);
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

	conn->backend.u.accept.cb = cb;
	ev_list_push_back(&lisn->backend.u.listen.accept_queue,
		&conn->backend.u.accept.accept_node);
	conn->base.flags |= EV_TCP_ACCEPTING;
	ev__io_add(lisn->base.loop, &lisn->backend.u.listen.io, EV_IO_IN);

	return EV_SUCCESS;
}

int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
	assert(sizeof(ev_buf_t) == sizeof(struct iovec));

	if (!(sock->base.flags & EV_TCP_STREAMING))
	{
		_ev_tcp_setup_stream(sock);
		sock->base.flags |= EV_TCP_STREAMING;
	}

	req->data.cb = cb;
	req->data.bufs = bufs;
	req->data.nbuf = nbuf;
	req->info.idx = 0;
	req->info.len = 0;
	ev_list_push_back(&sock->backend.u.stream.w_queue, &req->node);

	ev__io_add(sock->base.loop, &sock->backend.u.stream.io, EV_IO_OUT);

	return EV_SUCCESS;
}

int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb)
{
	if (!(sock->base.flags & EV_TCP_STREAMING))
	{
		_ev_tcp_setup_stream(sock);
		sock->base.flags |= EV_TCP_STREAMING;
	}

	req->data.cb = cb;
	req->data.bufs = bufs;
	req->data.nbuf = nbuf;
	ev_list_push_back(&sock->backend.u.stream.r_queue, &req->node);

	ev__io_add(sock->base.loop, &sock->backend.u.stream.io, EV_IO_IN);

	return EV_SUCCESS;
}

int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len)
{
	int ret;
	socklen_t socklen = *len;

	if ((ret = getsockname(sock->backend.sock, name, &socklen)) != 0)
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

	if ((ret = getpeername(sock->backend.sock, name, &socklen)) != 0)
	{
		return ev__translate_sys_error(errno);
	}

	*len = socklen;
	return EV_SUCCESS;
}

static void _ev_tcp_active(ev_tcp_t* sock)
{
	ev__handle_active(&sock->base);
}

static void _ev_tcp_deactive(ev_tcp_t* sock)
{
	unsigned flags = sock->base.flags;

	if (flags & (EV_TCP_LISTING | EV_TCP_ACCEPTING | EV_TCP_STREAMING | EV_TCP_CONNECTING))
	{
		return;
	}

	ev__handle_deactive(&sock->base);
}

static void _ev_tcp_do_connect_callback(ev_tcp_t* sock)
{
	sock->base.flags &= ~EV_TCP_CONNECTING;
	_ev_tcp_deactive(sock);
	sock->backend.u.client.cb(sock, sock->backend.u.client.stat);
}

static void _ev_tcp_on_connect(ev_io_t* io, unsigned evts)
{
	(void)evts;

	int ret;
	socklen_t result_len = sizeof(ret);
	ev_tcp_t* sock = container_of(io, ev_tcp_t, backend.u.client.io);

	/* Get connect result */
	if (getsockopt(sock->backend.sock, SOL_SOCKET, SO_ERROR, &ret, &result_len) < 0)
	{
		sock->backend.u.client.stat = ev__translate_sys_error(errno);
		goto fin;
	}

	/* Result is in `result` */
	sock->backend.u.client.stat = ev__translate_sys_error(ret);

fin:
	_ev_tcp_do_connect_callback(sock);
}

static void _ev_tcp_to_connect(ev_todo_t* todo)
{
	ev_tcp_t* sock = container_of(todo, ev_tcp_t, backend.u.client.token);
	_ev_tcp_do_connect_callback(sock);
}

int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb)
{
	int ret;
	ev_loop_t* loop = sock->base.loop;

	if (sock->base.flags & EV_TCP_CONNECTING)
	{
		return EV_EINPROGRESS;
	}

	sock->backend.u.client.cb = cb;

	if ((ret = connect(sock->backend.sock, addr, size)) == 0)
	{/* Connect success immediately */
		sock->backend.u.client.stat = EV_SUCCESS;
		ev__todo(loop, &sock->backend.u.client.token, _ev_tcp_to_connect);
		return EV_SUCCESS;
	}

	if (errno != EINPROGRESS)
	{
		return ev__translate_sys_error(errno);
	}

	ev__io_init(&sock->backend.u.client.io, sock->backend.sock, _ev_tcp_on_connect);
	ev__io_add(loop, &sock->backend.u.client.io, EV_IO_OUT);
	_ev_tcp_active(sock);

	return EV_SUCCESS;
}
