#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

static void _async_on_wakeup_unix(ev_nonblock_io_t *io, unsigned evts,
                                  void *arg)
{
    (void)evts;
    (void)arg;
    ev_async_t *handle = EV_CONTAINER_OF(io, ev_async_t, backend.io);

    ev__async_pend(handle->backend.pipfd[0]);
    handle->activate_cb(handle, handle->activate_arg);
}

static void _ev_async_on_close(ev_handle_t *handle)
{
    ev_async_t *async = EV_CONTAINER_OF(handle, ev_async_t, base);

    ev_async_cb close_cb = async->close_cb;
    void       *close_arg = async->close_arg;
    ev_free(async);

    if (close_cb != NULL)
    {
        close_cb(async, close_arg);
    }
}

static void _async_close_pipe(ev_async_t *handle)
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

static void _ev_async_exit(ev_async_t *handle, ev_async_cb close_cb,
                           void *close_arg)
{
    assert(!ev__handle_is_closing(&handle->base));

    handle->close_cb = close_cb;
    handle->close_arg = close_arg;
    _async_close_pipe(handle);

    ev__handle_deactive(&handle->base);

    if (close_cb != NULL)
    {
        ev__handle_exit(&handle->base, _ev_async_on_close);
    }
    else
    {
        ev__handle_exit(&handle->base, NULL);
        ev_free(handle);
    }
}

EV_LOCAL void ev__async_exit_force(ev_async_t *handle)
{
    _ev_async_exit(handle, NULL, NULL);
}

EV_LOCAL int ev__asyc_eventfd(int evtfd[2])
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

EV_LOCAL void ev__async_eventfd_close(int fd)
{
    close(fd);
}

EV_LOCAL void ev__async_post(int wfd)
{
    uint64_t val = 1;

    ssize_t write_size;
    int     errcode;

    do
    {
        write_size = write(wfd, &val, sizeof(val));
    } while (write_size == -1 && (errcode = errno) == EINTR);

    if (write_size < 0)
    {
        EV_ABORT();
    }
}

EV_LOCAL void ev__async_pend(int rfd)
{
    uint64_t val;
    int      errcode;
    ssize_t  read_size;

    do
    {
        read_size = read(rfd, &val, sizeof(val));
    } while (read_size == -1 && (errcode = errno) == EINTR);

    if (read_size < 0)
    {
        EV_ABORT();
    }
}

int ev_async_init(ev_loop_t *loop, ev_async_t **handle, ev_async_cb activate_cb,
                  void *activate_arg)
{
    int         errcode;
    ev_async_t *new_handle = ev_malloc(sizeof(ev_async_t));
    if (new_handle == NULL)
    {
        return EV_ENOMEM;
    }

    new_handle->activate_cb = activate_cb;
    new_handle->activate_arg = activate_arg;
    new_handle->close_cb = NULL;
    ev__handle_init(loop, &new_handle->base, EV_ROLE_EV_ASYNC);

    errcode = ev__asyc_eventfd(new_handle->backend.pipfd);
    if (errcode != 0)
    {
        goto err_close_handle;
    }

    ev__nonblock_io_init(&new_handle->backend.io, new_handle->backend.pipfd[0],
                         _async_on_wakeup_unix, NULL);
    ev__nonblock_io_add(loop, &new_handle->backend.io, EV_IO_IN);
    ev__handle_active(&new_handle->base);

    *handle = new_handle;
    return 0;

err_close_handle:
    _async_close_pipe(new_handle);
    ev__handle_exit(&new_handle->base, NULL);
    ev_free(new_handle);
    return errcode;
}

void ev_async_exit(ev_async_t *handle, ev_async_cb close_cb, void *close_arg)
{
    _ev_async_exit(handle, close_cb, close_arg);
}

void ev_async_wakeup(ev_async_t *handle)
{
    ev__async_post(handle->backend.pipfd[1]);
}
