
struct ev_sem_s
{
    sem_t r;
};

void ev_sem_init(ev_sem_t **sem, unsigned value)
{
    ev_sem_t *new_sem = ev_malloc(sizeof(ev_sem_t));
    if (new_sem == NULL)
    {
        EV_ABORT();
    }

    if (sem_init(&new_sem->r, 0, value))
    {
        EV_ABORT();
    }

    *sem = new_sem;
}

void ev_sem_exit(ev_sem_t *sem)
{
    if (sem_destroy(&sem->r))
    {
        EV_ABORT();
    }

    ev_free(sem);
}

void ev_sem_post(ev_sem_t *sem)
{
    if (sem_post(&sem->r))
    {
        EV_ABORT();
    }
}

void ev_sem_wait(ev_sem_t *sem)
{
    int r;
    do
    {
        r = sem_wait(&sem->r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        EV_ABORT();
    }
}

int ev_sem_try_wait(ev_sem_t *sem)
{
    int r;

    do
    {
        r = sem_trywait(&sem->r);
    } while (r == -1 && errno == EINTR);

    if (r)
    {
        if (errno == EAGAIN)
        {
            return EV_EAGAIN;
        }
        EV_ABORT();
    }

    return 0;
}
