#include "loop.h"
#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

static void _ev_async_on_close_unix(ev_handle_t* handle)
{
    ev_async_t* async = container_of(handle, ev_async_t, base);

    if (async->close_cb != NULL)
    {
        async->close_cb(async);
    }
}

static void _ev_aync_clear_eventfd(ev_loop_t* loop)
{
    uint64_t cnt = 0;

    for (;;)
    {
        ssize_t read_size = read(loop->backend.async.fd, &cnt, sizeof(cnt));
        if (read_size >= 0)
        {
            continue;
        }

        int err = errno;
        if (err == EINTR)
        {
            continue;
        }

        break;
    }
}

static void _ev_loop_on_async_unix(ev_nonblock_io_t* io, unsigned evts)
{
    (void)evts;
    ev_loop_t* loop = container_of(io, ev_loop_t, backend.async.io);
    _ev_aync_clear_eventfd(loop);

    ev_list_t queue = loop->backend.async.queue;
    ev_list_init(&loop->backend.async.queue);

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&queue)) != NULL)
    {
        ev_async_t* handle = container_of(it, ev_async_t, backend.node);
        ev_list_push_back(&loop->backend.async.queue, it);

        int pending;
        ev_mutex_enter(&handle->backend.mutex);
        {
            pending = handle->backend.pending;
            handle->backend.pending = 0;
        }
        ev_mutex_leave(&handle->backend.mutex);

        if (pending == 0)
        {
            continue;
        }

        if (handle->active_cb != NULL)
        {
            handle->active_cb(handle);
        }
    }
}

static void _ev_async_wakeup_loop(ev_loop_t* loop)
{
    uint64_t val = 1;
    ssize_t write_size;
    do
    {
        write_size = write(loop->backend.async.fd, &val, sizeof(val));
    } while (write_size < 0 && errno == EINTR);
}

int ev__async_init_loop(ev_loop_t* loop)
{
    loop->backend.async.fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);
    if (loop->backend.async.fd == -1)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }
    ev__nonblock_io_init(&loop->backend.async.io, loop->backend.async.fd, _ev_loop_on_async_unix);
    ev__nonblock_io_add(loop, &loop->backend.async.io, EPOLLIN);
    ev_list_init(&loop->backend.async.queue);

    return EV_SUCCESS;
}

void ev__async_exit_loop(ev_loop_t* loop)
{
    if (loop->backend.async.fd != -1)
    {
        close(loop->backend.async.fd);
        loop->backend.async.fd = -1;
    }
}

int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb)
{
    int ret;
    ev__handle_init(loop, &handle->base, EV_ROLE_EV_ASYNC, _ev_async_on_close_unix);
    handle->active_cb = cb;
    handle->backend.pending = 0;

    if ((ret = ev_mutex_init(&handle->backend.mutex, 0)) != EV_SUCCESS)
    {
        return ret;
    }

    ev_list_push_back(&loop->backend.async.queue, &handle->backend.node);
    ev__handle_active(&handle->base);

    return EV_SUCCESS;
}

void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb)
{
    ev_loop_t* loop = handle->base.data.loop;
    assert(!ev__handle_is_closing(&handle->base));

    handle->close_cb = close_cb;
    ev_list_erase(&loop->backend.async.queue, &handle->backend.node);
    ev_mutex_exit(&handle->backend.mutex);
    ev__handle_exit(&handle->base);
}

void ev_async_wakeup(ev_async_t* handle)
{
    /* Do a cheap read first. */
    int pending;
    ev_mutex_enter(&handle->backend.mutex);
    {
        pending = handle->backend.pending;
        handle->backend.pending = 1;
    }
    ev_mutex_leave(&handle->backend.mutex);

    if (pending != 0)
    {
        return;
    }

    ev_loop_t* loop = handle->base.data.loop;
    _ev_async_wakeup_loop(loop);
}
