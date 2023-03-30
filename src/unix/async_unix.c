#include "ev.h"
#include "io_unix.h"
#include "loop.h"
#include "handle.h"
#include "misc_unix.h"
#include "async_unix.h"
#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

static void _async_on_wakeup_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;
    ev_async_t* handle = EV_CONTAINER_OF(io, ev_async_t, backend.io);

    ev__async_pend(handle->backend.pipfd[0]);
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

static void _ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    assert(!ev__handle_is_closing(&handle->base));

    handle->close_cb = close_cb;
    _async_close_pipe(handle);

    ev__handle_event_dec(&handle->base);
    ev__handle_exit(&handle->base, close_cb != NULL ? _ev_async_on_close : NULL);
}

API_LOCAL void ev__async_exit_force(ev_async_t* handle)
{
    _ev_async_exit(handle, NULL);
}

API_LOCAL int ev__asyc_eventfd(int evtfd[2])
{
    int errcode;

#if defined(__linux__)
    if ((evtfd[0] = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) < 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    if ((evtfd[1] = dup(evtfd[0])) < 0)
    {
        errcode = errno;
        close(evtfd[0]);
        return ev__translate_sys_error(errcode);
    }
#else
    errcode = ev_pipe_make(evtfd, EV_PIPE_NONBLOCK, EV_PIPE_NONBLOCK);
    if (errcode != 0)
    {
        return errcode;
    }
#endif

    return 0;
}

API_LOCAL void ev__async_eventfd_close(int fd)
{
    close(fd);
}

API_LOCAL void ev__async_post(int wfd)
{
    uint64_t val = 1;

    ssize_t write_size;
    int errcode;

    do
    {
        write_size = write(wfd, &val, sizeof(val));
    }while(write_size == -1 && (errcode = errno) == EINTR);

    if (write_size < 0)
    {
        EV_ABORT();
    }
}

API_LOCAL void ev__async_pend(int rfd)
{
    uint64_t val;
    int errcode;
    ssize_t read_size;

    do
    {
        read_size = read(rfd, &val, sizeof(val));
    }while(read_size == -1 && (errcode = errno) == EINTR);

    if (read_size < 0)
    {
        EV_ABORT();
    }
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    int errcode;

    handle->active_cb = cb;
    handle->close_cb = NULL;
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC);

    errcode = ev__asyc_eventfd(handle->backend.pipfd);
    if (errcode != 0)
    {
        goto err_close_handle;
    }

    ev__nonblock_io_init(&handle->backend.io, handle->backend.pipfd[0],
        _async_on_wakeup_unix, NULL);
    ev__nonblock_io_add(loop, &handle->backend.io, EV_IO_IN);
    ev__handle_event_add(&handle->base);

    return 0;

err_close_handle:
    _async_close_pipe(handle);
    ev__handle_exit(&handle->base, NULL);
    return errcode;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    _ev_async_exit(handle, close_cb);
}

void ev_async_wakeup(ev_async_t* handle)
{
    ev__async_post(handle->backend.pipfd[1]);
}
