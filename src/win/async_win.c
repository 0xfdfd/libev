#include "ev/errno.h"
#include "async_win.h"
#include "handle.h"
#include "loop_win.h"
#include <assert.h>

static void _async_on_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(iocp, ev_async_t, backend.io);

    handle->backend.async_sent = 0;
    handle->active_cb(handle);
}

static void _ev_async_on_close_win(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);

    if (async->close_cb != NULL)
    {
        async->close_cb(async);
    }
}

static void _ev_asyc_exit_win(ev_async_t* handle, ev_async_cb close_cb, int is_force)
{
    handle->close_cb = close_cb;
    ev__handle_exit(&handle->base, is_force);
}

void ev__async_exit_force(ev_async_t* handle)
{
    _ev_asyc_exit_win(handle, NULL, 1);
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    handle->active_cb = cb;
    handle->close_cb = NULL;
    handle->backend.async_sent = 0;

    ev__iocp_init(&handle->backend.io, _async_on_iocp_win, NULL);
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC, _ev_async_on_close_win);
    ev__handle_active(&handle->base);

    return EV_SUCCESS;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    _ev_asyc_exit_win(handle, close_cb, 0);
}

void ev_async_wakeup(ev_async_t* handle)
{
    if (!InterlockedOr(&handle->backend.async_sent, 1))
    {
        ev__iocp_post(handle->base.data.loop, &handle->backend.io);
    }
}
