#include "loop.h"

int ev_sem_init(ev_sem_t* sem, unsigned value)
{
    if (sem_init(&sem->u.r, 0, value))
    {
        return ev__translate_sys_error(errno);
    }
    return 0;
}

void ev_sem_exit(ev_sem_t* sem)
{
    if (sem_destroy(&sem->u.r))
    {
        abort();
    }
}

void ev_sem_post(ev_sem_t* sem)
{
    if (sem_post(&sem->u.r))
    {
        abort();
    }
}

void ev_sem_wait(ev_sem_t* sem)
{
    int r;
    do
    {
        r = sem_wait(&sem->u.r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        abort();
    }
}

int ev_sem_try_wait(ev_sem_t* sem)
{
    int r;

    do
    {
        r = sem_trywait(&sem->u.r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        if (errno == EAGAIN)
        {
            return EV_EAGAIN;
        }
        abort();
    }

    return 0;
}
