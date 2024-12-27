
struct ev_mutex
{
    CRITICAL_SECTION r; /**< Real mutex */
};

void ev_mutex_init(ev_mutex_t **handle, int recursive)
{
    (void)recursive;
    ev_mutex_t *new_mutex = ev_malloc(sizeof(ev_mutex_t));
    if (new_mutex == NULL)
    {
        abort();
    }

    InitializeCriticalSection(&new_mutex->r);

    *handle = new_mutex;
}

void ev_mutex_exit(ev_mutex_t *handle)
{
    DeleteCriticalSection(&handle->r);
    ev_free(handle);
}

void ev_mutex_enter(ev_mutex_t *handle)
{
    EnterCriticalSection(&handle->r);
}

void ev_mutex_leave(ev_mutex_t *handle)
{
    LeaveCriticalSection(&handle->r);
}

int ev_mutex_try_enter(ev_mutex_t *handle)
{
    if (TryEnterCriticalSection(&handle->r))
    {
        return 0;
    }

    return EV_EBUSY;
}
