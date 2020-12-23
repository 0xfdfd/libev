#include <sys/uio.h>
#include <assert.h>
#include <unistd.h>
#include "loop.h"

static void _ev_tcp_close_fd(ev_tcp_t* sock)
{
	if (sock->fd != EV_INVALID_FD)
	{
		close(sock->fd);
		sock->fd = -1;
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

int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp, int flags)
{
	ev__handle_init(loop, &tcp->base, _ev_tcp_on_close);
	tcp->close_cb = NULL;
	tcp->flags = flags;
	tcp->fd = EV_INVALID_FD;

	return EV_SUCCESS;
}

void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb)
{
	sock->close_cb = cb;
	ev__handle_exit(&sock->base);
}

static int _ev_tcp_flags_to_domain(int flags)
{
	switch (flags)
	{
	case EV_TCP_IPV4:
		return AF_INET;
	case EV_TCP_IPV6:
		return AF_INET6;
	case 0:
	default:
		return AF_INET6;
	}
}

static int _ev_tcp_setup_fd(ev_tcp_t* tcp, int* new_fd)
{
	*new_fd = 0;
	if (tcp->fd != EV_INVALID_FD)
	{
		return EV_SUCCESS;
	}
	if ((tcp->fd = socket(_ev_tcp_flags_to_domain(tcp->flags), SOCK_STREAM, 0)) == -1)
	{
		return errno;
	}

	int ret;
	if ((ret = ev__nonblock(tcp->fd, 1)) != EV_SUCCESS)
	{
		goto err_nonblock;
	}

	*new_fd = 1;
	return EV_SUCCESS;

err_nonblock:
	close(tcp->fd);
	tcp->fd = EV_INVALID_FD;
	return ret;
}

int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen)
{
	int ret;
	int flag_new_fd;
	if ((ret = _ev_tcp_setup_fd(tcp, &flag_new_fd)) != EV_SUCCESS)
	{
		return ret;
	}

	if ((ret = bind(tcp->fd, addr, addrlen)) != 0)
	{
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

static int _ev_tcp_is_listening(ev_tcp_t* sock)
{
	return sock->base.flags & EV_TCP_LISTING;
}

static void _ev_tcp_on_accept(ev_io_t* io, unsigned evts)
{
	(void)evts;
	ev_tcp_t* acpt = container_of(io, ev_tcp_t, u.listen.io);

	ev_list_node_t* it = ev_list_pop_front(&acpt->u.listen.accept_queue);
	if (it == NULL)
	{
		ABORT();
	}

	ev_tcp_t* conn = container_of(it, ev_tcp_t, u.accept.accept_node);
	_ev_tcp_close_fd(conn);

	do 
	{
		conn->fd = accept(acpt->fd, (struct sockaddr*)&conn->u.accept.peeraddr, &conn->u.accept.addrlen);
	} while (conn->fd == -1 && errno == EINTR);

	conn->base.flags &= ~EV_TCP_ACCEPTING;

	int ret = conn->fd >= 0 ? EV_SUCCESS : errno;
	conn->u.accept.cb(acpt, conn, ret);

	if (ev_list_size(&acpt->u.listen.accept_queue) == 0)
	{
		ev__io_del(acpt->base.loop, io, EV_IO_IN);
	}
}

int ev_tcp_accept(ev_tcp_t* acpt, ev_tcp_t* conn, ev_accept_cb cb)
{
	int ret;
	if (conn->base.flags & EV_TCP_ACCEPTING)
	{
		return EV_EINPROGRESS;
	}
	assert(cb != NULL);

	if (!_ev_tcp_is_listening(acpt))
	{
		if ((ret = listen(acpt->fd, 1024)) != 0)
		{
			return ret;
		}

		ev_list_init(&acpt->u.listen.accept_queue);
		acpt->base.flags |= EV_TCP_LISTING;
		ev__io_init(&acpt->u.listen.io, acpt->fd, _ev_tcp_on_accept);
		ev__io_add(acpt->base.loop, &acpt->u.listen.io, EV_IO_IN);
	}

	conn->u.accept.cb = cb;
	ev_list_push_back(&acpt->u.listen.accept_queue, &conn->u.accept.accept_node);
	conn->base.flags |= EV_TCP_ACCEPTING;

	return EV_SUCCESS;
}

static void _ev_tcp_cleanup_all_read_request(ev_tcp_t* sock, int err)
{
	ev_list_node_t* it;
	while ((it = ev_list_pop_front(&sock->u.stream.r_queue)) != NULL)
	{
		ev_read_t* req = container_of(it, ev_read_t, node);
		req->data.cb(req, 0, err);
	}
}

static void _ev_tcp_do_read(ev_tcp_t* sock)
{
	ev_list_node_t* it = ev_list_pop_front(&sock->u.stream.r_queue);
	assert(it != NULL);
	ev_read_t* req = container_of(it, ev_read_t, node);

	ssize_t r;
	do 
	{
		r = readv(sock->fd, (struct iovec*)req->data.bufs, req->data.nbuf);
	} while (r == -1 && errno == EINTR);

	/* Peer close */
	if (r == 0)
	{
		req->data.cb(req, 0, EV_EOF);
		goto fin;
	}

	/* Try again */
	if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		ev_list_push_front(&sock->u.stream.r_queue, it);
		return;
	}

	if (r < 0)
	{
		_ev_tcp_cleanup_all_read_request(sock, errno);
		return;
	}

	req->data.cb(req, r, EV_SUCCESS);

fin:
	if (ev_list_size(&sock->u.stream.r_queue) == 0)
	{
		ev__io_del(sock->base.loop, &sock->u.stream.io, EV_IO_IN);
	}
}

static void _ev_tcp_do_write(ev_tcp_t* sock)
{
	ev_list_node_t* it = ev_list_pop_front(&sock->u.stream.w_queue);
	assert(it != NULL);
	ev_write_t* req = container_of(it, ev_write_t, node);

	// TODO write data
}

static void _ev_tcp_on_stream(ev_io_t* io, unsigned evts)
{
	ev_tcp_t* sock = container_of(io, ev_tcp_t, u.stream.io);
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
	ev__io_init(&sock->u.stream.io, sock->fd, _ev_tcp_on_stream);
	ev_list_init(&sock->u.stream.w_queue);
	ev_list_init(&sock->u.stream.r_queue);
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
	ev_list_push_back(&sock->u.stream.w_queue, &req->node);

	ev__io_add(sock->base.loop, &sock->u.stream.io, EV_IO_OUT);

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
	ev_list_push_back(&sock->u.stream.r_queue, &req->node);

	ev__io_add(sock->base.loop, &sock->u.stream.io, EV_IO_IN);

	return EV_SUCCESS;
}
