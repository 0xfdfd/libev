#include "ev/errno.h"
#include "loop.h"
#include "handle.h"
#include <assert.h>

/**
 * @brief Set handle as inactive
 * @see ev__handle_event_dec()
 * @param[in] handle    handler
 */
static void ev__handle_deactive(ev_handle_t* handle)
{
    if (!(handle->data.flags & EV_HANDLE_ACTIVE))
    {
        return;
    }
    handle->data.flags &= ~EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->loop;
    ev_list_erase(&loop->handles.active_list, &handle->handle_queue);
    ev_list_push_back(&loop->handles.idle_list, &handle->handle_queue);
}

/**
 * @brief Force set handle as active, regardless the active event counter.
 * @see ev__handle_event_add()
 * @param[in] handle    handler
 */
static void ev__handle_active(ev_handle_t* handle)
{
    if (handle->data.flags & EV_HANDLE_ACTIVE)
    {
        return;
    }
    handle->data.flags |= EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->loop;
    ev_list_erase(&loop->handles.idle_list, &handle->handle_queue);
    ev_list_push_back(&loop->handles.active_list, &handle->handle_queue);
}

static void _ev_to_close_handle(ev_handle_t* handle)
{
    ev__handle_event_dec(handle);

    /**
     * Deactive but not reset #ev_handle_t::data::active_events, for debug
     * purpose.
     * The #ev_handle_t::data::active_events should be zero by now.
     */
    ev__handle_deactive(handle);

    handle->data.flags |= EV_HANDLE_CLOSED;
    ev_list_erase(&handle->loop->handles.idle_list, &handle->handle_queue);

    handle->endgame.close_cb(handle);
}

void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role)
{
    handle->loop = loop;
    ev_list_push_back(&loop->handles.idle_list, &handle->handle_queue);

    handle->data.role = role;
    handle->data.flags = 0;
    handle->data.active_events = 0;

    handle->backlog.status = EV_ENOENT;
    handle->backlog.cb = NULL;
    handle->backlog.node = (ev_list_node_t)EV_LIST_NODE_INIT;

    handle->endgame.close_cb = NULL;
    handle->endgame.node = (ev_list_node_t)EV_LIST_NODE_INIT;
}

void ev__handle_exit(ev_handle_t* handle, ev_handle_cb close_cb)
{
    assert(!ev__handle_is_closing(handle));

    handle->data.flags |= EV_HANDLE_CLOSING;
    handle->endgame.close_cb = close_cb;

    if (close_cb != NULL)
    {
        ev__handle_event_add(handle);
        ev_list_push_back(&handle->loop->endgame_queue, &handle->endgame.node);
    }
    else
    {
        ev__handle_deactive(handle);
        handle->data.flags |= EV_HANDLE_CLOSED;
        ev_list_erase(&handle->loop->handles.idle_list, &handle->handle_queue);
    }
}

void ev__handle_event_add(ev_handle_t* handle)
{
    handle->data.active_events++;

    if (handle->data.active_events != 0)
    {
        ev__handle_active(handle);
    }
}

void ev__handle_event_dec(ev_handle_t* handle)
{
    assert(handle->data.active_events != 0);

    handle->data.active_events--;

    if (handle->data.active_events == 0)
    {
        ev__handle_deactive(handle);
    }
}

int ev__handle_is_active(ev_handle_t* handle)
{
    return handle->data.flags & EV_HANDLE_ACTIVE;
}

int ev__handle_is_closing(ev_handle_t* handle)
{
    return handle->data.flags & (EV_HANDLE_CLOSING | EV_HANDLE_CLOSED);
}

int ev__backlog_submit(ev_handle_t* handle, ev_handle_cb callback)
{
    if (handle->backlog.status != EV_ENOENT)
    {
        return EV_EEXIST;
    }

    handle->backlog.status = EV_EEXIST;
    handle->backlog.cb = callback;
    ev__handle_event_add(handle);

    ev_list_push_back(&handle->loop->backlog_queue, &handle->backlog.node);

    return EV_SUCCESS;
}

void ev__process_backlog(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->backlog_queue)) != NULL)
    {
        ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, backlog.node);

        ev__handle_event_dec(handle);
        handle->backlog.status = EV_ENOENT;

        handle->backlog.cb(handle);
    }
}

void ev__process_endgame(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->endgame_queue)) != NULL)
    {
        ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, endgame.node);
        _ev_to_close_handle(handle);
    }
}
