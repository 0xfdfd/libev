#include "ev/errno.h"
#include "ev/sem.h"
#include "loop.h"

void ev_sem_init(ev_sem_t* sem, unsigned value)
{
    DWORD errcode;
    sem->u.r = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (sem->u.r == NULL)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_exit(ev_sem_t* sem)
{
    DWORD errcode;
    if (!CloseHandle(sem->u.r))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    DWORD errcode;
    if (!ReleaseSemaphore(sem->u.r, 1, NULL))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    DWORD errcode;
    if (WaitForSingleObject(sem->u.r, INFINITE) != WAIT_OBJECT_0)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    DWORD ret = WaitForSingleObject(sem->u.r, 0);

    if (ret == WAIT_OBJECT_0)
    {
        return EV_SUCCESS;
    }

    if (ret == WAIT_TIMEOUT)
    {
        return EV_EAGAIN;
    }

    DWORD errcode = GetLastError();
    EV_ABORT("ret:%lu, GetLastError:%lu", ret, errcode);
}
