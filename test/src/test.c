#define _GNU_SOURCE
#include "test.h"
#include "tools/init.h"
#include "utils/config.h"
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
    mmc_init();
    test_config_setup(argc, argv);

    if (test_config.argvt != NULL)
    {
        int ret = tool_exec(test_config.argct, test_config.argvt);
        _check_mmc_leak();
        exit(ret);
    }
}

static void _after_all_test(void)
{
    test_config_cleanup();
    _check_mmc_leak();
}

static const char* _cutest_get_log_level_str(cutest_log_level_t level)
{
    switch (level)
    {
    case CUTEST_LOG_DEBUG:
        return "D";
    case CUTEST_LOG_INFO:
        return "I";
    case CUTEST_LOG_WARN:
        return "W";
    case CUTEST_LOG_ERROR:
        return "E";
    case CUTEST_LOG_FATAL:
        return "F";
    default:
        break;
    }
    return "U";
}

static void _on_log(cutest_log_meta_t* info, const char* fmt, va_list ap, FILE* out)
{
    fprintf(out, "[%s %u %s:%d] ", _cutest_get_log_level_str(info->leve),
        (unsigned)ev_thread_id(), info->file, info->line);
    vfprintf(out, fmt, ap);
    fprintf(out, "\n");
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
        TEST_LOG_F("memory leak: %zu byte%s", leak_size, leak_size > 1 ? "s" : "");
    }
}

cutest_hook_t test_hook = {
    _before_all_test,           /* .before_all_test */
    _after_all_test,            /* .after_all_test */
    _before_fixture_setup,      /* .before_fixture_setup */
    NULL,                       /* .after_fixture_setup */
    NULL,                       /* .before_fixture_teardown */
    _after_fixture_teardown,    /* .after_fixture_teardown */
    NULL,                       /* .before_fixture_test */
    NULL,                       /* .after_fixture_test */
    NULL,                       /* .before_parameterized_test */
    NULL,                       /* .after_parameterized_test */
    NULL,                       /* .before_simple_test */
    NULL,                       /* .after_simple_test */
    _on_log,                    /* .on_log_print */
};

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
