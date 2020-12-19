#include <unistd.h>
#include <assert.h>
#include "loop.h"

static void _ev_async_clear_recv_buffer(ev_io_t* io)
{
	char buffer[64];

	ssize_t r;
	for (;;)
	{
		r = read(io->data.fd, buffer, sizeof(buffer));
		if (r == sizeof(buffer))
		{
			continue;
		}

		if (r != -1)
		{
			break;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			break;
		}

		if (errno == EINTR)
		{
			continue;
		}

		ABORT();
	}
}

static void _ev_async_on_close_unix(ev_handle_t* handle)
{
	ev_async_t* async = container_of(handle, ev_async_t, base);

	close(async->backend.io_read.data.fd);
	async->backend.io_read.data.fd = -1;

	close(async->backend.fd_write);
	async->backend.fd_write = -1;

	if (async->close_cb != NULL)
	{
		async->close_cb(async);
	}
}

static void _ev_async_on_io_active(ev_io_t* io, unsigned evts)
{
	(void)evts;
	_ev_async_clear_recv_buffer(io);

	ev_async_t* async = container_of(io, ev_async_t, backend.io_read);
	if (async->active_cb != NULL)
	{
		async->active_cb(async);
	}
}

static int _ev_async_set_cloexec_nonblock(int pipefd[2])
{
	int ret;

	if ((ret = ev__cloexec(pipefd[0], 1)) != EV_SUCCESS)
	{
		return ret;
	}
	if ((ret = ev__cloexec(pipefd[1], 1)) != EV_SUCCESS)
	{
		return ret;
	}
	if ((ret = ev__nonblock(pipefd[0], 1)) != EV_SUCCESS)
	{
		return ret;
	}
	if ((ret = ev__nonblock(pipefd[1], 1)) != EV_SUCCESS)
	{
		return ret;
	}

	return EV_SUCCESS;
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
	int ret;
	int pipefd[2];

	ev__handle_init(loop, &handle->base, _ev_async_on_close_unix);
	handle->active_cb = cb;

	if (pipe(pipefd) != 0)
	{
		return errno;
	}
	if ((ret = _ev_async_set_cloexec_nonblock(pipefd)) != EV_SUCCESS)
	{
		goto err;
	}

	ev__io_init(&handle->backend.io_read, pipefd[0], _ev_async_on_io_active);
	handle->backend.fd_write = pipefd[1];

	ev__io_add(loop, &handle->backend.io_read, EV_IO_IN);
	ev__handle_active(&handle->base);

	return EV_SUCCESS;

err:
	close(pipefd[0]);
	close(pipefd[1]);
	return ret;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
	assert(!ev__handle_is_closing(&handle->base));

	handle->close_cb = close_cb;
	ev__io_del(handle->base.loop, &handle->backend.io_read, EV_IO_IN);
	ev__handle_exit(&handle->base);
}

void ev_async_weakup(ev_async_t* handle)
{
	ssize_t r;
	do 
	{
		r = write(handle->backend.fd_write, "", 1);
	} while ((r == -1) && (errno == EINTR));

	if (r == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
	{
		ABORT();
	}
}
