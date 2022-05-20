#include "ev/errno.h"
#include "ev/sem.h"
#include "loop.h"

void ev_sem_init(ev_sem_t* sem, unsigned value)
{
    sem->u.r = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (sem->u.r == NULL)
    {
        EV_ABORT();
    }
}

void ev_sem_exit(ev_sem_t* sem)
{
    if (!CloseHandle(sem->u.r))
    {
        EV_ABORT();
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    if (!ReleaseSemaphore(sem->u.r, 1, NULL))
    {
        EV_ABORT();
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    if (WaitForSingleObject(sem->u.r, INFINITE) != WAIT_OBJECT_0)
    {
        EV_ABORT();
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    DWORD r = WaitForSingleObject(sem->u.r, 0);

    if (r == WAIT_OBJECT_0)
        return EV_SUCCESS;

    if (r == WAIT_TIMEOUT)
        return EV_EAGAIN;

    EV_ABORT();
}
