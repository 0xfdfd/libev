#define _GNU_SOURCE
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

struct ev_thread
{
    pthread_t thr;
};

struct ev_thread_key
{
    pthread_key_t tls; /**< Thread local storage */
};

typedef struct ev_thread_helper_unix
{
    ev_thread_cb cb;
    void        *arg;
    sem_t        sem;
} ev_thread_helper_unix_t;

static void *_ev_thread_proxy_unix(void *arg)
{
    ev_thread_helper_unix_t *p_helper = arg;
    ev_thread_helper_unix_t  helper = *p_helper;

    if (sem_post(&p_helper->sem) != 0)
    {
        EV_ABORT();
    }

    helper.cb(helper.arg);
    return NULL;
}

static int s_thread_init(ev_thread_t *thr, const ev_thread_opt_t *opt,
                         ev_thread_cb cb, void *arg)
{
    int             err;
    pthread_attr_t *attr = NULL;
    pthread_attr_t  attr_storage;

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

    if ((err = pthread_create(&thr->thr, attr, _ev_thread_proxy_unix,
                              &helper)) != 0)
    {
        goto release_sem;
    }

    do
    {
        err = sem_wait(&helper.sem);
    } while (err == -1 && errno == EINTR);

    err = err != 0 ? errno : 0;

release_sem:
    sem_destroy(&helper.sem);
err_fin:
    if (attr != NULL)
    {
        pthread_attr_destroy(attr);
    }
    return ev__translate_sys_error(err);
}

int ev_thread_init(ev_thread_t **thr, const ev_thread_opt_t *opt,
                   ev_thread_cb cb, void *arg)
{
    ev_thread_t *new_thread = ev_malloc(sizeof(ev_thread_t));
    if (new_thread == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = s_thread_init(new_thread, opt, cb, arg);
    if (ret != 0)
    {
        ev_free(new_thread);
        return ret;
    }

    *thr = new_thread;
    return 0;
}

int ev_thread_exit(ev_thread_t *thr, unsigned long timeout)
{
    int ret = EBUSY;
    if (timeout == EV_INFINITE_TIMEOUT)
    {
        int err = pthread_join(thr->thr, NULL);
        if (err == 0)
        {
            ev_free(thr);
            return 0;
        }
        return ev__translate_sys_error(err);
    }

    const uint64_t t_start = ev_hrtime() / 1000000;
    const uint64_t t_end = t_start + timeout;

    uint64_t t_now;
    while ((t_now = ev_hrtime() / 1000000) < t_end)
    {
        if ((ret = pthread_tryjoin_np(thr->thr, NULL)) == 0)
        {
            break;
        }

        uint64_t t_diff = t_end - t_now;
        unsigned sleep_time = t_diff < 10 ? t_diff : 10;
        ev_thread_sleep(sleep_time);
    }

    /* try last time */
    if (ret == EBUSY)
    {
        ret = pthread_tryjoin_np(thr->thr, NULL);
    }
    if (ret == 0)
    {
        ev_free(thr);
        return 0;
    }

    return ret == EBUSY ? EV_ETIMEDOUT : ev__translate_sys_error(ret);
}

ev_os_tid_t ev_thread_id(void)
{
    return syscall(__NR_gettid);
}

void ev_thread_sleep(uint32_t timeout)
{
    struct timespec t_req, t_rem;
    t_req.tv_sec = timeout / 1000;
    t_req.tv_nsec = (timeout - t_req.tv_sec * 1000) * 1000 * 1000;

    int ret;
    while ((ret = nanosleep(&t_req, &t_rem)) != 0)
    {
        ret = errno;
        if (ret != EINTR)
        {
            EV_ABORT();
        }
        t_req = t_rem;
    }
}

int ev_thread_key_init(ev_thread_key_t **key)
{
    ev_thread_key_t *new_key = ev_malloc(sizeof(ev_thread_key_t));
    if (new_key == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = pthread_key_create(&new_key->tls, NULL);
    if (ret != 0)
    {
        ev_free(new_key);
        return ev__translate_sys_error(ret);
    }

    *key = new_key;
    return 0;
}

void ev_thread_key_exit(ev_thread_key_t *key)
{
    int ret = pthread_key_delete(key->tls);
    if (ret != 0)
    {
        EV_ABORT();
    }
    ev_free(key);
}

void ev_thread_key_set(ev_thread_key_t *key, void *val)
{
    int ret = pthread_setspecific(key->tls, val);
    if (ret != 0)
    {
        EV_ABORT();
    }
}

void *ev_thread_key_get(ev_thread_key_t *key)
{
    return pthread_getspecific(key->tls);
}
