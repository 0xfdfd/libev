#define _GNU_SOURCE
#include "ev-common.h"
#include <semaphore.h>
#include <pthread.h>

typedef struct ev_thread_helper_unix
{
    ev_thread_cb    cb;
    void*           arg;
    sem_t           sem;
}ev_thread_helper_unix_t;

static void* _ev_thread_proxy_unix(void* arg)
{
    ev_thread_helper_unix_t* p_helper = arg;
    ev_thread_helper_unix_t helper = *p_helper;

    if (sem_post(&p_helper->sem) != 0)
    {
        abort();
    }

    helper.cb(helper.arg);
    return NULL;
}

int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg)
{
    int err = EV_SUCCESS;

    pthread_attr_t* attr = NULL;
    pthread_attr_t attr_storage;

    if (opt != NULL && opt->flags.have_stack_size)
    {
        if (pthread_attr_init(&attr_storage) != 0)
        {
            err = errno;
            return ev__translate_sys_error(err);
        }

        if (pthread_attr_setstacksize(&attr_storage, opt->stack_size) != 0)
        {
            err = errno;
            goto err_fin;
        }

        attr = &attr_storage;
    }

    ev_thread_helper_unix_t helper;
    helper.cb = cb;
    helper.arg = arg;
    if (sem_init(&helper.sem, 0, 0) != 0)
    {
        err = errno;
        goto err_fin;
    }

    if ((err = pthread_create(thr, attr, _ev_thread_proxy_unix, &helper)) != 0)
    {
        goto release_sem;
    }

    do 
    {
        err = sem_wait(&helper.sem);
    } while (err == -1 && errno == EINTR);

    err = err != 0 ? errno : EV_SUCCESS;

release_sem:
    sem_destroy(&helper.sem);
err_fin:
    if (attr != NULL)
    {
        pthread_attr_destroy(attr);
    }
    return ev__translate_sys_error(err);
}

int ev_thread_exit(ev_os_thread_t* thr, unsigned timeout)
{
    int ret = EBUSY;
    if (timeout == EV_THREAD_WAIT_INFINITE)
    {
        int err = pthread_join(*thr, NULL);
        return ev__translate_sys_error(err);
    }

    const uint64_t t_start = ev__clocktime();
    const uint64_t t_end = t_start + timeout;

    uint64_t t_now;
    while ((t_now = ev__clocktime()) < t_end)
    {
        if ((ret = pthread_tryjoin_np(*thr, NULL)) == 0)
        {
            break;
        }

        uint64_t t_diff = t_end - t_now;
        unsigned sleep_time = t_diff < 10 ? t_diff : 10;
        ev_thread_sleep(sleep_time, NULL);
    }

    /* try last time */
    if (ret == EBUSY)
    {
        ret = pthread_tryjoin_np(*thr, NULL);
    }

    return ret == EBUSY ? EV_ETIMEDOUT : ev__translate_sys_error(ret);
}

ev_os_thread_t ev_thread_self(void)
{
    return pthread_self();
}

int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2)
{
    return pthread_equal(*t1, *t2);
}

int ev_thread_sleep(unsigned req, unsigned* rem)
{
    struct timespec t_req, t_rem;
    t_req.tv_sec = req / 1000;
    t_req.tv_nsec = (req - t_req.tv_sec * 1000) * 1000 * 1000;

    int ret = nanosleep(&t_req, &t_rem);
    if (rem != NULL)
    {
        *rem = t_rem.tv_sec * 1000 + t_rem.tv_nsec / 1000 / 1000;
    }

    ret = ret != 0 ? errno : EV_SUCCESS;
    return ev__translate_sys_error(ret);
}
