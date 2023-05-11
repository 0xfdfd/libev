#include "ev.h"
#include "timer.h"
#include "handle.h"
#include "loop.h"
#include <string.h>

static int _ev_cmp_timer(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_timer_t* t1 = EV_CONTAINER_OF(key1, ev_timer_t, node);
    ev_timer_t* t2 = EV_CONTAINER_OF(key2, ev_timer_t, node);

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

static void _ev_timer_on_close(ev_handle_t* handle)
{
    ev_timer_t* timer = EV_CONTAINER_OF(handle, ev_timer_t, base);
    if (timer->close_cb != NULL)
    {
        timer->close_cb(timer);
    }
}

EV_LOCAL void ev__init_timer(ev_loop_t* loop)
{
    ev_map_init(&loop->timer.heap, _ev_cmp_timer, NULL);
}

EV_LOCAL void ev__process_timer(ev_loop_t* loop)
{
    ev_map_node_t* it;
    while ((it = ev_map_begin(&loop->timer.heap)) != NULL)
    {
        ev_timer_t* timer = EV_CONTAINER_OF(it, ev_timer_t, node);
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
    }
}

int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle)
{
    memset(handle, 0, sizeof(*handle));

    ev__handle_init(loop, &handle->base, EV_ROLE_EV_TIMER);
    return 0;
}

void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb)
{
    handle->close_cb = close_cb;

    ev_timer_stop(handle);
    ev__handle_exit(&handle->base, _ev_timer_on_close);
}

int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat)
{
    ev_loop_t* loop = handle->base.loop;
    if (ev__handle_is_active(&handle->base))
    {
        ev_timer_stop(handle);
    }

    handle->attr.cb = cb;
    handle->attr.timeout = timeout;
    handle->attr.repeat = repeat;
    handle->data.active = loop->hwtime + timeout;

    if (ev_map_insert(&loop->timer.heap, &handle->node) != 0)
    {
        EV_ABORT("duplicate timer");
    }
    ev__handle_event_add(&handle->base);

    return 0;
}

void ev_timer_stop(ev_timer_t* handle)
{
    if (!ev__handle_is_active(&handle->base))
    {
        return;
    }

    ev__handle_event_dec(&handle->base);
    ev_map_erase(&handle->base.loop->timer.heap, &handle->node);
}
