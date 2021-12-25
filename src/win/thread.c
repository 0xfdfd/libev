#include "ev-common.h"
#include <process.h>

typedef struct ev_thread_helper_win
{
    ev_thread_cb    cb;
    void*           arg;
    HANDLE          start_sem;
}ev_thread_helper_win_t;

static size_t _ev_thread_calculate_stack_size_win(const ev_thread_opt_t* opt)
{
    if (opt == NULL || !opt->flags.have_stack_size)
    {
        return 0;
    }

    return opt->stack_size;
}

static unsigned __stdcall _ev_thread_proxy_proc_win(void* lpThreadParameter)
{
    ev_thread_helper_win_t* p_helper = lpThreadParameter;
    ev_thread_helper_win_t helper = *p_helper;

    if (!ReleaseSemaphore(p_helper->start_sem, 1, NULL))
    {
        abort();
    }

    helper.cb(helper.arg);
    return 0;
}

int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg)
{
    int err = EV_SUCCESS;

    ev_thread_helper_win_t helper;
    helper.cb = cb;
    helper.arg = arg;
    if ((helper.start_sem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL)
    {
        err = GetLastError();
        goto err_fin;
    }

    size_t stack_size = _ev_thread_calculate_stack_size_win(opt);
    HANDLE thr_ret = (HANDLE)_beginthreadex(NULL, (unsigned)stack_size,
        _ev_thread_proxy_proc_win, &helper, 0, NULL);
    if (thr_ret == NULL)
    {
        err = GetLastError();
        goto err_create_thread;
    }

    int ret = WaitForSingleObject(helper.start_sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        err = ret != WAIT_FAILED ? EV_EINVAL : GetLastError();
        goto err_create_thread;
    }

    *thr = thr_ret;

err_create_thread:
    CloseHandle(helper.start_sem);
err_fin:
    return ev__translate_sys_error(err);
}

int ev_thread_exit(ev_os_thread_t* thr, unsigned timeout)
{
    int ret = WaitForSingleObject(*thr, timeout);
    switch (ret)
    {
    case WAIT_TIMEOUT:
        return EV_ETIMEDOUT;
    case WAIT_ABANDONED:
        abort();    // should not happen
    case WAIT_FAILED:
        ret = GetLastError();
        return ret == WAIT_TIMEOUT ? EV_ETIMEDOUT : ev__translate_sys_error(ret);
    default:
        break;
    }

    CloseHandle(*thr);
    *thr = EV_OS_THREAD_INVALID;

    return EV_SUCCESS;
}

ev_os_thread_t ev_thread_self(void)
{
    return GetCurrentThread();
}

int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2)
{
    return *t1 == *t2;
}

int ev_thread_sleep(unsigned req, unsigned* rem)
{
    Sleep(req);
    if (rem != NULL)
    {
        *rem = 0;
    }
    return EV_SUCCESS;
}
