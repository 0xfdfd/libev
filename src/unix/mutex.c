#include "ev-common.h"

static int _ev_mutex_init_unix(ev_os_mutex_t* handle)
{
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
    int err = pthread_mutex_init(handle, NULL);
    return ev__translate_sys_error(err);
#else
    pthread_mutexattr_t attr;
    int err;

    if (pthread_mutexattr_init(&attr))
    {
        abort();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        abort();
    }

    err = pthread_mutex_init(handle, &attr);

    if (pthread_mutexattr_destroy(&attr))
    {
        abort();
    }

    return ev__translate_sys_error(err);
#endif
}

static int _ev_mutex_init_recursive_unix(ev_os_mutex_t* handle)
{
    pthread_mutexattr_t attr;
    int err;

    if (pthread_mutexattr_init(&attr))
    {
        abort();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    {
        abort();
    }

    err = pthread_mutex_init(handle, &attr);
    if (pthread_mutexattr_destroy(&attr))
    {
        abort();
    }

    return ev__translate_sys_error(err);
}

int ev_mutex_init(ev_os_mutex_t* handle, int recursive)
{
    return recursive ?
        _ev_mutex_init_recursive_unix(handle) : _ev_mutex_init_unix(handle);
}

void ev_mutex_exit(ev_os_mutex_t* handle)
{
    if (pthread_mutex_destroy(handle))
    {
        abort();
    }
}

void ev_mutex_enter(ev_os_mutex_t* handle)
{
    if (pthread_mutex_lock(handle))
    {
        abort();
    }
}

void ev_mutex_leave(ev_os_mutex_t* handle)
{
    if (pthread_mutex_unlock(handle))
    {
        abort();
    }
}

int ev_mutex_try_enter(ev_os_mutex_t* handle)
{
    int err = pthread_mutex_trylock(handle);
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
