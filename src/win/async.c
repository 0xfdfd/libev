#include "loop.h"

static void _ev_async_on_active(ev_iocp_t* req)
{
	ev_async_t* async = container_of(req, ev_async_t, iocp_req);
	if (async->cb != NULL)
	{
		async->cb(async);
	}
}

static void _ev_async_on_exit(ev_todo_t* handle)
{
	ev_async_t* async = container_of(handle, ev_async_t, close_token);
	if (async->close_cb != NULL)
	{
		async->close_cb(async);
	}
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
	ev__handle_init(loop, &handle->base);
	ev__iocp_init(&handle->iocp_req, _ev_async_on_active);
	handle->cb = cb;

	loop->active_handles++;

	return EV_ESUCCESS;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
	handle->close_cb = close_cb;
	ev__todo(handle->base.loop, &handle->close_token, _ev_async_on_exit);

	handle->base.loop->active_handles--;
	ev__handle_exit(&handle->base);
}

void ev_async_weakup(ev_async_t* handle)
{
	ev_loop_t* loop = handle->base.loop;

	PostQueuedCompletionStatus(loop->backend.iocp,
		0, 0, &handle->iocp_req.overlapped);
}
