#include <process.h>

struct ev_thread
{
    HANDLE thread;
};

struct ev_thread_key
{
    DWORD tls; /**< Thread local storage */
};

typedef struct ev_thread_helper_win
{
    ev_thread_cb cb;        /**< User thread body */
    void        *arg;       /**< User thread argument */
    HANDLE       start_sem; /**< Start semaphore */
    HANDLE       thread_id; /**< Thread handle */
} ev_thread_helper_win_t;

static size_t _ev_thread_calculate_stack_size_win(const ev_thread_opt_t *opt)
{
    if (opt == NULL || !opt->flags.have_stack_size)
    {
        return 0;
    }

    return opt->stack_size;
}

static unsigned CALLBACK _ev_thread_proxy_proc_win(void *lpThreadParameter)
{
    DWORD                   errcode;
    ev_thread_helper_win_t *p_helper = lpThreadParameter;
    ev_thread_helper_win_t  helper = *p_helper;

    if (!ReleaseSemaphore(p_helper->start_sem, 1, NULL))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
    }

    helper.cb(helper.arg);
    return 0;
}

static int s_ev_thread_init(ev_thread_t *thr, const ev_thread_opt_t *opt,
                            ev_thread_cb cb, void *arg)
{
    DWORD err = 0;

    ev_thread_helper_win_t helper;
    helper.cb = cb;
    helper.arg = arg;
    if ((helper.start_sem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL)
    {
        err = GetLastError();
        goto err_fin;
    }

    size_t stack_size = _ev_thread_calculate_stack_size_win(opt);
    helper.thread_id = (HANDLE)_beginthreadex(NULL, (unsigned)stack_size,
                                              _ev_thread_proxy_proc_win,
                                              &helper, CREATE_SUSPENDED, NULL);
    if (helper.thread_id == NULL)
    {
        err = GetLastError();
        goto err_create_thread;
    }

    if (ResumeThread(helper.thread_id) == -1)
    {
        err = GetLastError();
        EV_ABORT("GetLastError:%lu", err);
    }

    int ret = WaitForSingleObject(helper.start_sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        err = (ret != WAIT_FAILED) ? ERROR_INVALID_PARAMETER : GetLastError();
        goto err_create_thread;
    }

    thr->thread = helper.thread_id;

err_create_thread:
    CloseHandle(helper.start_sem);
err_fin:
    return ev__translate_sys_error(err);
}

int ev_thread_init(ev_thread_t **thr, const ev_thread_opt_t *opt,
                   ev_thread_cb cb, void *arg)
{
    ev__init_once_win();

    ev_thread_t *new_thread = ev_malloc(sizeof(ev_thread_t));
    if (new_thread == NULL)
    {
        return EV_ENOMEM;
    }

    const int ret = s_ev_thread_init(new_thread, opt, cb, arg);
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
    int ret = WaitForSingleObject(thr->thread, timeout);
    switch (ret)
    {
    case WAIT_TIMEOUT:
        return EV_ETIMEDOUT;
    case WAIT_ABANDONED:
        EV_ABORT("WAIT_ABANDONED"); // should not happen
        break;
    case WAIT_FAILED:
        ret = GetLastError();
        return ret == WAIT_TIMEOUT ? EV_ETIMEDOUT
                                   : ev__translate_sys_error(ret);
    default:
        break;
    }

    CloseHandle(thr->thread);
    thr->thread = NULL;

    ev_free(thr);

    return 0;
}

ev_os_tid_t ev_thread_id(void)
{
    return GetCurrentThreadId();
}

void ev_thread_sleep(uint32_t timeout)
{
    Sleep(timeout);
}

int ev_thread_key_init(ev_thread_key_t **key)
{
    ev_thread_key_t *new_key = ev_malloc(sizeof(ev_thread_key_t));
    if (new_key == NULL)
    {
        return EV_ENOMEM;
    }

    if ((new_key->tls = TlsAlloc()) == TLS_OUT_OF_INDEXES)
    {
        DWORD err = GetLastError();
        ev_free(new_key);
        return ev__translate_sys_error(err);
    }

    *key = new_key;
    return 0;
}

void ev_thread_key_exit(ev_thread_key_t *key)
{
    if (TlsFree(key->tls) == FALSE)
    {
        DWORD errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
    ev_free(key);
}

void ev_thread_key_set(ev_thread_key_t *key, void *val)
{
    if (TlsSetValue(key->tls, val) == FALSE)
    {
        DWORD errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

void *ev_thread_key_get(ev_thread_key_t *key)
{
    void *val = TlsGetValue(key->tls);
    if (val == NULL)
    {
        DWORD errcode = GetLastError();
        if (errcode != ERROR_SUCCESS)
        {
            EV_ABORT("GetLastError:%lu", errcode);
        }
    }
    return val;
}
