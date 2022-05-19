#include "ev-common.h"

static void _ev_mutex_init_unix(ev_os_mutex_t* handle)
{
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
    if (pthread_mutex_init(handle, NULL) != 0)
    {
        abort();
    }
#else
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        abort();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        abort();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        abort();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        abort();
    }
#endif
}

static void _ev_mutex_init_recursive_unix(ev_os_mutex_t* handle)
{
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
    {
        abort();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    {
        abort();
    }

    if (pthread_mutex_init(handle, &attr) != 0)
    {
        abort();
    }

    if (pthread_mutexattr_destroy(&attr))
    {
        abort();
    }
}

void ev_mutex_init(ev_mutex_t* handle, int recursive)
{
    if (recursive)
    {
        _ev_mutex_init_recursive_unix(&handle->u.r);
    }
    else
    {
        _ev_mutex_init_unix(&handle->u.r);
    }
}

void ev_mutex_exit(ev_mutex_t* handle)
{
    if (pthread_mutex_destroy(&handle->u.r))
    {
        abort();
    }
}

void ev_mutex_enter(ev_mutex_t* handle)
{
    if (pthread_mutex_lock(&handle->u.r))
    {
        abort();
    }
}

void ev_mutex_leave(ev_mutex_t* handle)
{
    if (pthread_mutex_unlock(&handle->u.r))
    {
        abort();
    }
}

int ev_mutex_try_enter(ev_mutex_t* handle)
{
    int err = pthread_mutex_trylock(&handle->u.r);
    if (!err)
    {
        return EV_SUCCESS;
    }

    if (err != EBUSY && err != EAGAIN)
    {
        abort();
    }

    return EV_EBUSY;
}
