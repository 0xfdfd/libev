#define _GNU_SOURCE
#include "test.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static void _test_proxy(void* arg)
{
    fn_execute callback = (fn_execute)arg;
    callback();
}

int test_thread_execute(test_execute_token_t* token, fn_execute callback)
{
    return ev_thread_init(&token->thr, NULL, _test_proxy, (void*)callback);
}

int test_thread_wait(test_execute_token_t* token)
{
    return ev_thread_exit(&token->thr, 0);
}

const char* test_strerror(int errcode)
{
#if defined(_WIN32)
    static char buffer[1024];
    strerror_s(buffer, sizeof(buffer), errcode);
    return buffer;
#else
    return strerror(errcode);
#endif
}
