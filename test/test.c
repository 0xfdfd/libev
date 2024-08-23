#define _GNU_SOURCE
#include "test.h"
#include "tools/__init__.h"
#include "utils/config.h"
#include "type/__init__.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if !defined(_WIN32)
#   include <unistd.h>
#endif

typedef struct test_ctx
{
    mmc_snapshot_t* mm_snapshot[2];
}test_ctx_t;

static test_ctx_t g_test_ctx = {
    { NULL, NULL },
};

static void _check_mmc_leak(void)
{
    mmc_info_t info;
    mmc_dump(&info);

    if (info.blocks == 0)
    {
        return;
    }

    fflush(NULL);
    fprintf(stderr, "[  ERROR   ] memory leak detected: %zu block%s not free.\n",
            info.blocks, info.blocks == 1 ? "" : "s");

    exit(EXIT_FAILURE);
}

static void _before_all_test(int argc, char* argv[])
{
    test_config_setup(argc, argv);

    mmc_init(!test_config.config.pure_malloc);

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
    _check_mmc_leak();

    ev_library_shutdown();

    /* ensure all output was done. */
    fflush(NULL);

    mmc_exit();
}

static void _on_mem_leak(memblock_t* block, void* arg)
{
    (void)arg;
    size_t* p_leak_size = arg;
    *p_leak_size += block->size;
}

static void _before_fixture_setup(const char* fixture_name)
{
    (void)fixture_name;
    fflush(NULL);
    mmc_snapshot_take(&g_test_ctx.mm_snapshot[0]);
}

static void _after_fixture_teardown(const char* fixture_name, int ret)
{
    (void)fixture_name; (void)ret;
    fflush(NULL);

    mmc_snapshot_take(&g_test_ctx.mm_snapshot[1]);

    size_t leak_size = 0;
    mmc_snapshot_compare(g_test_ctx.mm_snapshot[0], g_test_ctx.mm_snapshot[1],
        _on_mem_leak, &leak_size);

    mmc_snapshot_free(&g_test_ctx.mm_snapshot[0]);
    mmc_snapshot_free(&g_test_ctx.mm_snapshot[1]);

    if (leak_size != 0)
    {
        test_abort("[  ERROR   ] memory leak detected: %zu byte%s\n",
            leak_size, leak_size > 1 ? "s" : "");
    }
}

cutest_hook_t test_hook = {
    _before_all_test,           /* .before_all_test */
    _after_all_test,            /* .after_all_test */
    _before_fixture_setup,      /* .before_setup */
    NULL,                       /* .after_setup */
    NULL,                       /* .before_teardown */
    _after_fixture_teardown,    /* .after_teardown */
    NULL,                       /* .before_test */
    NULL,                       /* .after_test */
};

ev_loop_t empty_loop = EV_LOOP_INVALID;

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

void test_abort(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    cutest_porting_abort(fmt, ap);
    va_end(ap);
}
