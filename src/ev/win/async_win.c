#include <assert.h>

static void _async_on_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(iocp, ev_async_t, backend.io);

    handle->backend.async_sent = 0;
    handle->activate_cb(handle, handle->activate_arg);
}

static void _ev_async_on_close_win(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);
    ev_async_cb close_cb = async->close_cb;
    void* close_arg = async->close_arg;
    ev_free(async);

    if (close_cb != NULL)
    {
        close_cb(async, close_arg);
    }
}

static void _ev_asyc_exit_win(ev_async_t* handle, ev_async_cb close_cb,
    void* close_arg)
{
    handle->close_cb = close_cb;
    handle->close_arg = close_arg;
    ev__handle_deactive(&handle->base);
    if (close_cb != NULL)
    {
        ev__handle_exit(&handle->base, _ev_async_on_close_win);
    }
    else
    {
        ev__handle_exit(&handle->base, NULL);
        ev_free(handle);
    }
}

EV_LOCAL void ev__async_exit_force(ev_async_t* handle)
{
    _ev_asyc_exit_win(handle, NULL, NULL);
}

int ev_async_init(ev_loop_t *loop, ev_async_t **handle,
    ev_async_cb activate_cb, void *activate_arg)
{
    ev_async_t* new_handle = ev_malloc(sizeof(ev_async_t));
    if (new_handle == NULL)
    {
        return EV_ENOMEM;
    }

    new_handle->activate_cb = activate_cb;
    new_handle->activate_arg = activate_arg;
    new_handle->close_cb = NULL;
    new_handle->close_arg = NULL;
    new_handle->backend.async_sent = 0;

    ev__iocp_init(&new_handle->backend.io, _async_on_iocp_win, NULL);
    ev__handle_init(loop, &new_handle->base, EV_ROLE_EV_ASYNC);
    ev__handle_active(&new_handle->base);

    *handle = new_handle;
    return 0;
}

void ev_async_exit(ev_async_t *handle, ev_async_cb close_cb,
    void *close_arg)
{
    _ev_asyc_exit_win(handle, close_cb, close_arg);
}

void ev_async_wakeup(ev_async_t* handle)
{
    if (!InterlockedOr(&handle->backend.async_sent, 1))
    {
        ev__iocp_post(handle->base.loop, &handle->backend.io);
    }
}
