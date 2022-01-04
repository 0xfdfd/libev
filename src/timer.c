#include <string.h>
#include "ev-common.h"

static void _ev_timer_on_close(ev_handle_t* handle)
{
    ev_timer_t* timer = container_of(handle, ev_timer_t, base);
    if (timer->close_cb != NULL)
    {
        timer->close_cb(timer);
    }
}

int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle)
{
    memset(handle, 0, sizeof(*handle));

    ev__handle_init(loop, &handle->base, EV_ROLE_EV_TIMER, _ev_timer_on_close);
    return EV_SUCCESS;
}

void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb)
{
    handle->close_cb = close_cb;

    ev_timer_stop(handle);
    ev__handle_exit(&handle->base, 0);
}

int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat)
{
    ev_loop_t* loop = handle->base.data.loop;
    if (ev__handle_is_active(&handle->base))
    {
        ev_timer_stop(handle);
    }

    handle->attr.cb = cb;
    handle->attr.timeout = timeout;
    handle->attr.repeat = repeat;
    handle->data.active = loop->hwtime + timeout;

    if (ev_map_insert(&loop->timer.heap, &handle->node) < 0)
    {
        BREAK_ABORT();
    }
    ev__handle_active(&handle->base);

    return EV_SUCCESS;
}

void ev_timer_stop(ev_timer_t* handle)
{
    if (!ev__handle_is_active(&handle->base))
    {
        return;
    }

    ev__handle_deactive(&handle->base);
    ev_map_erase(&handle->base.data.loop->timer.heap, &handle->node);
}
