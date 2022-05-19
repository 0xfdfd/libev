#include "ev/sem.h"
#include "loop.h"

void ev_sem_init(ev_sem_t* sem, unsigned value)
{
    sem->u.r = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (sem->u.r == NULL)
    {
        abort();
    }
}

void ev_sem_exit(ev_sem_t* sem)
{
    if (!CloseHandle(sem->u.r))
    {
        abort();
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    if (!ReleaseSemaphore(sem->u.r, 1, NULL))
    {
        abort();
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    if (WaitForSingleObject(sem->u.r, INFINITE) != WAIT_OBJECT_0)
    {
        abort();
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    DWORD r = WaitForSingleObject(sem->u.r, 0);

    if (r == WAIT_OBJECT_0)
        return EV_SUCCESS;

    if (r == WAIT_TIMEOUT)
        return EV_EAGAIN;

    abort();
}
