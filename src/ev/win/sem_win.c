
struct ev_sem_s
{
    HANDLE r;
};

void ev_sem_init(ev_sem_t **sem, unsigned value)
{
    DWORD     errcode;
    ev_sem_t *new_sem = ev_malloc(sizeof(ev_sem_t));
    if (new_sem == NULL)
    {
        EV_ABORT("out of memory.");
    }

    new_sem->r = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (new_sem->r == NULL)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }

    *sem = new_sem;
}

void ev_sem_exit(ev_sem_t *sem)
{
    DWORD errcode;
    if (!CloseHandle(sem->r))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
    ev_free(sem);
}

void ev_sem_post(ev_sem_t *sem)
{
    DWORD errcode;
    if (!ReleaseSemaphore(sem->r, 1, NULL))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void ev_sem_wait(ev_sem_t *sem)
{
    DWORD errcode;
    if (WaitForSingleObject(sem->r, INFINITE) != WAIT_OBJECT_0)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

int ev_sem_try_wait(ev_sem_t *sem)
{
    DWORD ret = WaitForSingleObject(sem->r, 0);

    if (ret == WAIT_OBJECT_0)
    {
        return 0;
    }

    if (ret == WAIT_TIMEOUT)
    {
        return EV_EAGAIN;
    }

    DWORD errcode = GetLastError();
    EV_ABORT("ret:%lu, GetLastError:%lu", ret, errcode);
}
