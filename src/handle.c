#include "handle.h"
#include <assert.h>

static void _ev_to_close(ev_todo_token_t* todo)
{
    ev_handle_t* handle = EV_CONTAINER_OF(todo, ev_handle_t, data.close_queue);

    handle->data.flags &= ~EV_HANDLE_CLOSING;
    handle->data.flags |= EV_HANDLE_CLOSED;

    ev_list_erase(&handle->data.loop->handles.idle_list, &handle->node);
    handle->data.close_cb(handle);
}

void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role, ev_close_cb close_cb)
{
    ev_list_push_back(&loop->handles.idle_list, &handle->node);

    handle->data.loop = loop;
    handle->data.role = role;
    handle->data.flags = 0;

    handle->data.close_cb = close_cb;
}

void ev__handle_exit(ev_handle_t* handle, int force)
{
    assert(!ev__handle_is_closing(handle));

    /* Stop if necessary */
    ev__handle_deactive(handle);

    if (handle->data.close_cb != NULL && force == 0)
    {
        handle->data.flags |= EV_HANDLE_CLOSING;
        ev_todo_submit(handle->data.loop, &handle->data.close_queue, _ev_to_close);
    }
    else
    {
        handle->data.flags |= EV_HANDLE_CLOSED;
        ev_list_erase(&handle->data.loop->handles.idle_list, &handle->node);
    }
}

void ev__handle_active(ev_handle_t* handle)
{
    if (handle->data.flags & EV_HANDLE_ACTIVE)
    {
        return;
    }
    handle->data.flags |= EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->data.loop;
    ev_list_erase(&loop->handles.idle_list, &handle->node);
    ev_list_push_back(&loop->handles.active_list, &handle->node);
}

void ev__handle_deactive(ev_handle_t* handle)
{
    if (!(handle->data.flags & EV_HANDLE_ACTIVE))
    {
        return;
    }
    handle->data.flags &= ~EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->data.loop;
    ev_list_erase(&loop->handles.active_list, &handle->node);
    ev_list_push_back(&loop->handles.idle_list, &handle->node);
}

int ev__handle_is_active(ev_handle_t* handle)
{
    return handle->data.flags & EV_HANDLE_ACTIVE;
}

int ev__handle_is_closing(ev_handle_t* handle)
{
    return handle->data.flags & (EV_HANDLE_CLOSING | EV_HANDLE_CLOSED);
}
