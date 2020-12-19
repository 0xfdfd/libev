#include <assert.h>
#include "loop.h"

/* Frequency of the high-resolution clock. */
static uint64_t hrtime_frequency_ = 0;

static void _ev_time_init_win(void)
{
	LARGE_INTEGER perf_frequency;

	/* Retrieve high-resolution timer frequency
	 * and precompute its reciprocal.
	 */
	if (QueryPerformanceFrequency(&perf_frequency))
	{
		hrtime_frequency_ = perf_frequency.QuadPart;
	}
	else
	{
		ABORT();
	}
}

static uint64_t _ev_hrtime_win(unsigned int scale)
{
	LARGE_INTEGER counter;
	double scaled_freq;
	double result;

	assert(hrtime_frequency_ != 0);
	assert(scale != 0);
	if (!QueryPerformanceCounter(&counter))
	{
		ABORT();
	}
	assert(counter.QuadPart != 0);

	/* Because we have no guarantee about the order of magnitude of the
	 * performance counter interval, integer math could cause this computation
	 * to overflow. Therefore we resort to floating point math.
	 */
	scaled_freq = (double)hrtime_frequency_ / scale;
	result = (double)counter.QuadPart / scaled_freq;
	return (uint64_t)result;
}

static void _ev_update_time_win(ev_loop_t* loop)
{
	static ev_once_t s_guard = EV_ONCE_INIT;
	ev_once_execute(&s_guard, _ev_time_init_win);

	loop->hwtime = _ev_hrtime_win(1000);
}

static int _ev_translate_sys_error_win(int err)
{
	if (err <= 0)
	{
		return err;
	}

	assert(0);
	return err;
}

static int _ev_cmp_timer(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
	(void)arg;
	ev_timer_t* t1 = container_of(key1, ev_timer_t, node);
	ev_timer_t* t2 = container_of(key2, ev_timer_t, node);

	if (t1->data.active == t2->data.active)
	{
		if (t1 == t2)
		{
			return 0;
		}
		return t1 < t2 ? -1 : 1;
	}

	return t1->data.active < t2->data.active ? -1 : 1;
}

static int _ev_loop_alive(ev_loop_t* loop)
{
	return loop->active_handles || ev_list_size(&loop->todo.queue);
}

static int _ev_loop_active_timer(ev_loop_t* loop)
{
	int ret = 0;

	ev_map_node_t* it;
	while ((it = ev_map_begin(&loop->timer.heap)) != NULL)
	{
		ev_timer_t* timer = container_of(it, ev_timer_t, node);
		if (timer->data.active > loop->hwtime)
		{
			break;
		}

		ev_timer_stop(timer);
		if (timer->attr.repeat != 0)
		{
			ev_timer_start(timer, timer->attr.cb, timer->attr.repeat, timer->attr.repeat);
		}
		timer->attr.cb(timer);
		ret++;
	}

	return ret;
}

static void _ev_pool_win_handle_req(OVERLAPPED_ENTRY* overlappeds, ULONG count)
{
	ULONG i;
	for (i = 0; i < count; i++)
	{
		if (overlappeds[i].lpOverlapped)
		{
			ev_iocp_t* req = container_of(overlappeds[i].lpOverlapped, ev_iocp_t, overlapped);
			req->cb(req);
		}
	}
}

static void _ev_poll_win(ev_loop_t* loop, uint32_t timeout)
{
	int repeat;
	BOOL success;
	ULONG count;
	OVERLAPPED_ENTRY overlappeds[128];

	uint64_t timeout_time = loop->hwtime + timeout;

	for (repeat = 0;; repeat++)
	{
		success = GetQueuedCompletionStatusEx(loop->backend.iocp, overlappeds,
			ARRAY_SIZE(overlappeds), &count, timeout, FALSE);

		/* If success, handle all IOCP request */
		if (success)
		{
			_ev_pool_win_handle_req(overlappeds, count);
			return;
		}

		/* Cannot handle any other error */
		if (GetLastError() != WAIT_TIMEOUT)
		{
			ABORT();
		}

		if (timeout == 0)
		{
			return;
		}

		/**
		 * GetQueuedCompletionStatusEx() can occasionally return a little early.
		 * Make sure that the desired timeout target time is reached.
		 */
		_ev_update_time_win(loop);

		if (timeout_time <= loop->hwtime)
		{
			break;
		}

		timeout = (uint32_t)(timeout_time - loop->hwtime);
		timeout += repeat ? (1U << (repeat - 1)) : 0;
	}
}

static uint32_t _ev_backend_timeout_timer(ev_loop_t* loop)
{
	ev_map_node_t* it = ev_map_begin(&loop->timer.heap);
	if (it == NULL)
	{
		return 0;
	}

	ev_timer_t* timer = container_of(it, ev_timer_t, node);
	if (timer->data.active <= loop->hwtime)
	{
		return 0;
	}
	uint64_t dif = timer->data.active - loop->hwtime;
	return dif > UINT32_MAX ? UINT32_MAX : (uint32_t)dif;
}

static uint32_t _ev_backend_timeout(ev_loop_t* loop)
{
	if (loop->mask.b_stop)
	{
		return 0;
	}

	if (ev_list_size(&loop->todo.queue) != 0)
	{
		return 0;
	}

	return _ev_backend_timeout_timer(loop);
}

static void _ev_loop_active_todo(ev_loop_t* loop)
{
	ev_list_node_t* it;
	while ((it = ev_list_pop_front(&loop->todo.queue)) != NULL)
	{
		ev_todo_t* todo = container_of(it, ev_todo_t, node);
		todo->cb(todo);
	}
}

static void _ev_to_close(ev_todo_t* todo)
{
	ev_handle_t* handle = container_of(todo, ev_handle_t, close_queue);

	handle->flags &= ~EV_HANDLE_CLOSING;
	handle->flags |= EV_HANDLE_CLOSED;

	handle->close_cb(handle);
}

void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_close_cb close_cb)
{
	handle->loop = loop;
	handle->close_cb = close_cb;
	handle->close_queue = (ev_todo_t)EV_TODO_INIT;
	handle->flags = 0;
}

void ev__handle_exit(ev_handle_t* handle)
{
	if (ev__handle_is_closing(handle))
	{
		ABORT();
		return;
	}

	/* Stop if necessary */
	ev__handle_deactive(handle);

	handle->flags |= EV_HANDLE_CLOSING;

	ev__todo(handle->loop, &handle->close_queue, _ev_to_close);
	handle->loop = NULL;
}

void ev__handle_active(ev_handle_t* handle)
{
	if (handle->flags & EV_HANDLE_ACTIVE)
	{
		return;
	}

	handle->flags |= EV_HANDLE_ACTIVE;
	handle->loop->active_handles++;
}

void ev__handle_deactive(ev_handle_t* handle)
{
	if (!(handle->flags & EV_HANDLE_ACTIVE))
	{
		return;
	}

	handle->flags &= ~EV_HANDLE_ACTIVE;
	handle->loop->active_handles--;
}

int ev__handle_is_active(ev_handle_t* handle)
{
	return handle->flags & EV_HANDLE_ACTIVE;
}

int ev__handle_is_closing(ev_handle_t* handle)
{
	return handle->flags & (EV_HANDLE_CLOSING | EV_HANDLE_CLOSED);
}

void ev__todo(ev_loop_t* loop, ev_todo_t* todo, ev_todo_cb cb)
{
	todo->cb = cb;
	ev_list_push_back(&loop->todo.queue, &todo->node);
}

void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb)
{
	req->cb = cb;
}

int ev_loop_init(ev_loop_t* loop)
{
	loop->hwtime = 0;
	_ev_update_time_win(loop);

	loop->backend.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (loop->backend.iocp == NULL)
	{
		return _ev_translate_sys_error_win(GetLastError());
	}

	ev_map_init(&loop->timer.heap, _ev_cmp_timer, NULL);
	ev_list_init(&loop->todo.queue);
	loop->active_handles = 0;

	memset(&loop->mask, 0, sizeof(loop->mask));

	return EV_ESUCCESS;
}

void ev_loop_exit(ev_loop_t* loop)
{
	CloseHandle(loop->backend.iocp);
	loop->backend.iocp = NULL;
}

int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode)
{
	uint32_t timeout;

	int r = _ev_loop_alive(loop);

	while (r != 0 && !loop->mask.b_stop)
	{
		_ev_update_time_win(loop);

		_ev_loop_active_timer(loop);
		_ev_loop_active_todo(loop);

		/* Calculate timeout */
		timeout = mode != ev_loop_mode_nowait ?
			_ev_backend_timeout(loop) : 0;

		_ev_poll_win(loop, timeout);

		/**
		 * #ev_loop_mode_once implies forward progress: at least one callback must have
		 * been invoked when it returns. #_ev_poll_win() can return without doing
		 * I/O (meaning: no callbacks) when its timeout expires - which means we
		 * have pending timers that satisfy the forward progress constraint.
		 *
		 * #ev_loop_mode_nowait makes no guarantees about progress so it's omitted from
		 * the check.
		 */
		if (mode == ev_loop_mode_once)
		{
			_ev_update_time_win(loop);
			_ev_loop_active_timer(loop);
		}

		r = _ev_loop_alive(loop);
		if (mode != ev_loop_mode_default)
		{
			break;
		}
	}

	/* Prevent #ev_loop_t::mask::b_stop from compile optimization */
	if (loop->mask.b_stop)
	{
		loop->mask.b_stop = 0;
	}

	return r;
}

void ev_loop_stop(ev_loop_t* loop)
{
	loop->mask.b_stop = 1;
}
