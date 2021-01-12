#include <assert.h>
#include "loop.h"

static void _ev_async_on_active(ev_iocp_t* req, size_t transferred)
{
	(void)transferred;
	ev_async_t* async = container_of(req, ev_async_t, backend.iocp);

	/**
	 * Do callback only when async is active, because user may exit async in
	 * the callback.
	 */
	if (ev__handle_is_active(&async->base)
		&& async->active_cb != NULL)
	{
		async->active_cb(async);
	}
}

static void _ev_async_on_close_win(ev_handle_t* handle)
{
	ev_async_t* async = container_of(handle, ev_async_t, base);
	if (async->close_cb != NULL)
	{
		async->close_cb(async);
	}
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
	ev__handle_init(loop, &handle->base, _ev_async_on_close_win);
	handle->active_cb = cb;

	ev__iocp_init(&handle->backend.iocp, _ev_async_on_active);

	ev__handle_active(&handle->base);

	return EV_SUCCESS;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
	assert(!ev__handle_is_closing(&handle->base));

	handle->close_cb = close_cb;
	ev__handle_exit(&handle->base);
}

void ev_async_weakup(ev_async_t* handle)
{
	ev_loop_t* loop = handle->base.loop;

	PostQueuedCompletionStatus(loop->backend.iocp,
		0, 0, &handle->backend.iocp.overlapped);
}
