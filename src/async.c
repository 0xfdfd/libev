#include "ev/async.h"
#include "ev-common.h"

static void _ev_async_on_close(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);

    if (async->data.close_cb != NULL)
    {
        async->data.close_cb(async);
    }
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    int ret;
    if ((ret = ev_mutex_init(&handle->data.mutex, 0)) != EV_SUCCESS)
    {
        return ret;
    }

    handle->data.active_cb = cb;
    handle->data.close_cb = NULL;
    handle->data.pending = 0;
    ev_queue_init(&handle->node);
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC, _ev_async_on_close);
    ev__handle_active(&handle->base);

    return EV_SUCCESS;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    ev_loop_t* loop = handle->base.data.loop;
    assert(!ev__handle_is_closing(&handle->base));

    handle->data.close_cb = close_cb;

    ev_mutex_enter(&loop->wakeup.async.mutex);
    {
        ev_queue_erase(&handle->node);
    }
    ev_mutex_leave(&loop->wakeup.async.mutex);

    ev_mutex_exit(&handle->data.mutex);
    ev__handle_exit(&handle->base, 0);
}

void ev_async_wakeup(ev_async_t* handle)
{
    ev_loop_t* loop = handle->base.data.loop;

    int pending;
    ev_mutex_enter(&handle->data.mutex);
    {
        pending = handle->data.pending;
        handle->data.pending = 1;
    }
    ev_mutex_leave(&handle->data.mutex);

    if (pending)
    {
        return;
    }

    ev_mutex_enter(&loop->wakeup.async.mutex);
    {
        ev_queue_push_back(&loop->wakeup.async.queue, &handle->node);
    }
    ev_mutex_leave(&loop->wakeup.async.mutex);

    ev__loop_wakeup(loop);
}
