#include "ev-common.h"
#include "loop.h"
#include <process.h>

typedef struct ev_thread_helper_win
{
    ev_thread_cb    cb;         /**< User thread body */
    void*           arg;        /**< User thread argument */
    HANDLE          start_sem;  /**< Start semaphore */
    HANDLE          thread_id;  /**< Thread handle */
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

    ev_tls_set(&g_ev_loop_win_ctx.thread.thread_key, (void*)p_helper->thread_id);
    if (!ReleaseSemaphore(p_helper->start_sem, 1, NULL))
    {
        abort();
    }

    helper.cb(helper.arg);
    return 0;
}

void ev__thread_init_win(void)
{
    int ret = ev_tls_init(&g_ev_loop_win_ctx.thread.thread_key);
    if (ret != EV_SUCCESS)
    {
        abort();
    }
}

int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg)
{
    int err = EV_SUCCESS;
    ev__init_once_win();

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
        _ev_thread_proxy_proc_win, &helper, CREATE_SUSPENDED, NULL);
    if (helper.thread_id == NULL)
    {
        err = GetLastError();
        goto err_create_thread;
    }

    if (ResumeThread(helper.thread_id) == -1)
    {
        err = GetLastError();
        abort();
    }

    int ret = WaitForSingleObject(helper.start_sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        err = ret != WAIT_FAILED ? EV_EINVAL : GetLastError();
        goto err_create_thread;
    }

    *thr = helper.thread_id;

err_create_thread:
    CloseHandle(helper.start_sem);
err_fin:
    return ev__translate_sys_error(err);
}

int ev_thread_exit(ev_os_thread_t* thr, unsigned long timeout)
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
    *thr = NULL;

    return EV_SUCCESS;
}

ev_os_thread_t ev_thread_self(void)
{
    ev__init_once_win();
    return ev_tls_get(&g_ev_loop_win_ctx.thread.thread_key);
}

ev_os_tid_t ev_thread_id(void)
{
    return GetCurrentThreadId();
}

int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2)
{
    return *t1 == *t2;
}

void ev_thread_sleep(unsigned long timeout)
{
    Sleep(timeout);
}

int ev_tls_init(ev_tls_t* tls)
{
    int err;
    if ((tls->tls = TlsAlloc()) == TLS_OUT_OF_INDEXES)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

void ev_tls_exit(ev_tls_t* tls)
{
    if (TlsFree(tls->tls) == FALSE)
    {
        abort();
    }
    tls->tls = TLS_OUT_OF_INDEXES;
}

void ev_tls_set(ev_tls_t* tls, void* val)
{
    if (TlsSetValue(tls->tls, val) == FALSE)
    {
        abort();
    }
}

void* ev_tls_get(ev_tls_t* tls)
{
    void* val = TlsGetValue(tls->tls);
    if (val == NULL)
    {
        int err = GetLastError();
        if (err != ERROR_SUCCESS)
        {
            abort();
        }
    }
    return val;
}
