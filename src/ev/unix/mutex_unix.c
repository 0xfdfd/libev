
struct ev_mutex
{
    pthread_mutex_t r; /**< Real mutex */
};

static void _ev_mutex_init_unix(pthread_mutex_t *handle)
{
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
    if (pthread_mutex_init(handle, NULL) != 0)
    {
        EV_ABORT();
    }
#else
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        EV_ABORT();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        EV_ABORT();
    }
#endif
}

static void _ev_mutex_init_recursive_unix(pthread_mutex_t *handle)
{
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    {
        EV_ABORT();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        EV_ABORT();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        EV_ABORT();
    }
}

void ev_mutex_init(ev_mutex_t **handle, int recursive)
{
    ev_mutex_t *new_mutex = ev_malloc(sizeof(ev_mutex_t));
    if (new_mutex == NULL)
    {
        abort();
    }

    if (recursive)
    {
        _ev_mutex_init_recursive_unix(&new_mutex->r);
    }
    else
    {
        _ev_mutex_init_unix(&new_mutex->r);
    }

    *handle = new_mutex;
}

void ev_mutex_exit(ev_mutex_t *handle)
{
    if (pthread_mutex_destroy(&handle->r))
    {
        EV_ABORT();
    }

    ev_free(handle);
}

void ev_mutex_enter(ev_mutex_t *handle)
{
    if (pthread_mutex_lock(&handle->r))
    {
        EV_ABORT();
    }
}

void ev_mutex_leave(ev_mutex_t *handle)
{
    if (pthread_mutex_unlock(&handle->r))
    {
        EV_ABORT();
    }
}

int ev_mutex_try_enter(ev_mutex_t *handle)
{
    int err = pthread_mutex_trylock(&handle->r);
    if (!err)
    {
        return 0;
    }

    if (err != EBUSY && err != EAGAIN)
    {
        EV_ABORT();
    }

    return EV_EBUSY;
}
