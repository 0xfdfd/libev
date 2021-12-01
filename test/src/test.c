#define _GNU_SOURCE
#include "test.h"
#include <errno.h>
#include <stdio.h>

#if defined(_MSC_VER)

static DWORD WINAPI _thread_proc(LPVOID lpParameter)
{
    fn_execute callback = (fn_execute)lpParameter;
    callback();
    return 0;
}

int test_thread_execute(test_execute_token_t* token, fn_execute callback)
{
    token->thr_handle = CreateThread(NULL, 0, _thread_proc, (LPVOID)callback, 0, NULL);
    return token->thr_handle != INVALID_HANDLE_VALUE ? 0 : -1;
}

int test_thread_wait(test_execute_token_t* token)
{
    DWORD ret = WaitForSingleObject(token->thr_handle, 0);
    switch (ret)
    {
    case WAIT_ABANDONED:
        abort();
        break;

        /* thread finish */
    case WAIT_OBJECT_0:
        return 0;

        /* not finish */
    case WAIT_TIMEOUT:
        return 1;

        /* error */
    case WAIT_FAILED:
    default:
        break;
    }
    return -1;
}

void test_thread_sleep(unsigned ms)
{
    Sleep(ms);
}

#else
#   include <unistd.h>

static void* _task_body(void* arg)
{
    fn_execute callback = arg;
    callback();
    return NULL;
}

int test_thread_execute(test_execute_token_t* token, fn_execute callback)
{
    return pthread_create(&token->thr_handle, NULL, _task_body, callback);
}

int test_thread_wait(test_execute_token_t* token)
{
    int ret = pthread_tryjoin_np(token->thr_handle, NULL);

    //printf("ret:%d\n", ret);
    switch (ret)
    {
        /* terminated */
    case 0:
        return 0;
        /* not finish */
    case EBUSY:
        return 1;
    default:
        return -1;
    }
}

void test_thread_sleep(unsigned ms)
{
    usleep(ms * 1000);
}

#endif

