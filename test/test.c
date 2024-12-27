#define _GNU_SOURCE
#include "test.h"
#include "tools/__init__.h"
#include "utils/config.h"
#include "type/__init__.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static void _check_mmc_leak(void)
{
    mmc_info_t info;
    mmc_dump(&info);

    if (info.blocks == 0)
    {
        return;
    }

    fflush(NULL);
    fprintf(stderr,
            "[  ERROR   ] memory leak detected: %zu block%s not free.\n",
            info.blocks, info.blocks == 1 ? "" : "s");

    exit(EXIT_FAILURE);
}

static void _before_all_test(int argc, char *argv[])
{
    test_config_setup(argc, argv);

#if defined(TEST_NO_MMC)
    mmc_init(0);
#else
    mmc_init(!test_config.config.pure_malloc);
#endif

    register_types();

    if (test_config.tool.argvt != NULL)
    {
        int ret = tool_exec(test_config.tool.argct, test_config.tool.argvt);
        _check_mmc_leak();
        exit(ret);
    }
}

static void _after_all_test(void)
{
    test_config_cleanup();

    ev_library_shutdown();
    _check_mmc_leak();

    /* ensure all output was done. */
    fflush(NULL);

    mmc_exit();
}

cutest_hook_t test_hook = {
    _before_all_test, /* .before_all_test */
    _after_all_test,  /* .after_all_test */
    NULL,             /* .before_setup */
    NULL,             /* .after_setup */
    NULL,             /* .before_teardown */
    NULL,             /* .after_teardown */
    NULL,             /* .before_test */
    NULL,             /* .after_test */
};

static void _test_proxy(void *arg)
{
    fn_execute callback = (fn_execute)arg;
    callback();
}

int test_thread_execute(test_execute_token_t *token, fn_execute callback)
{
    return ev_thread_init(&token->thr, NULL, _test_proxy, (void *)callback);
}

int test_thread_wait(test_execute_token_t *token)
{
    return ev_thread_exit(token->thr, 0);
}

const char *test_strerror(int errcode)
{
#if defined(_WIN32)
    static char buffer[1024];
    strerror_s(buffer, sizeof(buffer), errcode);
    return buffer;
#else
    return strerror(errcode);
#endif
}

void test_abort(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    cutest_porting_abort(fmt, ap);
    va_end(ap);
}
