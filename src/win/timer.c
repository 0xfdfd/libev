#include "loop.h"

int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle)
{
	memset(handle, 0, sizeof(*handle));

	ev__handle_init(loop, &handle->base);
	return EV_ESUCCESS;
}

static void _ev_timer_to_close(ev_todo_t* handle)
{
	ev_timer_t* timer = container_of(handle, ev_timer_t, clse.token);
	if (timer->clse.cb != NULL)
	{
		timer->clse.cb(timer);
	}
}

void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb)
{
	handle->clse.cb = close_cb;
	ev__todo(handle->base.loop, &handle->clse.token, _ev_timer_to_close);

	ev_timer_stop(handle);
	ev__handle_exit(&handle->base);
}

int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat)
{
	ev_loop_t* loop = handle->base.loop;
	if (handle->mask.b_start)
	{
		ev_timer_stop(handle);
	}

	handle->attr.cb = cb;
	handle->attr.timeout = timeout;
	handle->attr.repeat = repeat;
	handle->data.active = loop->hwtime + timeout;

	if (ev_map_insert(&loop->timer.heap, &handle->node) < 0)
	{
		ABORT();
	}
	handle->mask.b_start = 1;
	handle->base.loop->active_handles++;

	return EV_ESUCCESS;
}

void ev_timer_stop(ev_timer_t* handle)
{
	if (!handle->mask.b_start)
	{
		return;
	}

	handle->mask.b_start = 0;
	ev_map_erase(&handle->base.loop->timer.heap, &handle->node);
	handle->base.loop->active_handles--;
}
