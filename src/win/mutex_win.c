#include "ev/errno.h"
#include "loop.h"

void ev_mutex_init(ev_mutex_t* handle, int recursive)
{
    (void)recursive;
    InitializeCriticalSection(&handle->u.r);
}

void ev_mutex_exit(ev_mutex_t* handle)
{
    DeleteCriticalSection(&handle->u.r);
}

void ev_mutex_enter(ev_mutex_t* handle)
{
    EnterCriticalSection(&handle->u.r);
}

void ev_mutex_leave(ev_mutex_t* handle)
{
    LeaveCriticalSection(&handle->u.r);
}

int ev_mutex_try_enter(ev_mutex_t* handle)
{
    if (TryEnterCriticalSection(&handle->u.r))
    {
        return EV_SUCCESS;
    }

    return EV_EBUSY;
}
