#include "ev-common.h"

int ev_mutex_init(ev_os_mutex_t* handle, int recursive)
{
    (void)recursive;
    InitializeCriticalSection(handle);
    return 0;
}

void ev_mutex_exit(ev_os_mutex_t* handle)
{
    DeleteCriticalSection(handle);
}

void ev_mutex_enter(ev_os_mutex_t* handle)
{
    EnterCriticalSection(handle);
}

void ev_mutex_leave(ev_os_mutex_t* handle)
{
    LeaveCriticalSection(handle);
}

int ev_mutex_try_enter(ev_os_mutex_t* handle)
{
    if (TryEnterCriticalSection(handle))
    {
        return EV_SUCCESS;
    }

    return EV_EBUSY;
}
