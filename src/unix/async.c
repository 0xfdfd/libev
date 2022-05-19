#include "unix/io.h"
#include "async.h"
#include "handle.h"
#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

static void _async_on_wakeup_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(io, ev_async_t, backend.io);
    handle->active_cb(handle);
}

static void _ev_async_on_close(ev_handle_t* handle)
{
    ev_async_t* async = EV_CONTAINER_OF(handle, ev_async_t, base);

    if (async->close_cb != NULL)
    {
        async->close_cb(async);
    }
}

static void _async_close_pipe(ev_async_t* handle)
{
    if (handle->backend.pipfd[0] != -1)
    {
        close(handle->backend.pipfd[0]);
        handle->backend.pipfd[0] = -1;
    }
    if (handle->backend.pipfd[1] != -1)
    {
        close(handle->backend.pipfd[1]);
        handle->backend.pipfd[1] = -1;
    }
}

static void _ev_async_exit(ev_async_t* handle, ev_async_cb close_cb, int is_force)
{
    assert(!ev__handle_is_closing(&handle->base));

    handle->close_cb = close_cb;
    _async_close_pipe(handle);

    ev__handle_exit(&handle->base, is_force);
}

void ev__async_exit_force(ev_async_t* handle)
{
    _ev_async_exit(handle, NULL, 1);
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    int errcode;

    handle->active_cb = cb;
    handle->close_cb = NULL;
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC, _ev_async_on_close);

#if defined(__linux__)
    if ((handle->backend.pipfd[0] = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) < 0)
    {
        errcode = errno;
        errcode = ev__translate_sys_error(errcode);
        goto err_close_handle;
    }

    handle->backend.pipfd[1] = dup(handle->backend.pipfd[0]);
    if (handle->backend.pipfd[1] == -1)
    {
        errcode = errno;
        errcode = ev__translate_sys_error(errcode);
        close(handle->backend.pipfd[0]);
        goto err_close_handle;
    }
#else
    errcode = ev_pipe_make(handle->backend.pipfd, EV_PIPE_NONBLOCK, EV_PIPE_NONBLOCK);
    if (errcode != EV_SUCCESS)
    {
        goto err_close_handle;
    }
#endif

    ev__nonblock_io_init(&handle->backend.io, handle->backend.pipfd[0],
        _async_on_wakeup_unix, NULL);
    ev__nonblock_io_add(loop, &handle->backend.io, EV_IO_IN);
    ev__handle_active(&handle->base);

    return EV_SUCCESS;

err_close_handle:
    _async_close_pipe(handle);
    ev__handle_exit(&handle->base, 1);
    return errcode;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    _ev_async_exit(handle, close_cb, 0);
}

void ev_async_wakeup(ev_async_t* handle)
{
    uint64_t val = 1;

    ssize_t write_size;
    int errcode;

    do
    {
        write_size = write(handle->backend.pipfd[1], &val, sizeof(val));
    }while(write_size == -1 && (errcode = errno) == EINTR);

    if (write_size < 0)
    {
        abort();
    }
}
