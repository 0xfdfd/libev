#include "test.h"
#include "tools/memcheck.h"
#include "utils/config.h"
#include <stdlib.h>
#include <string.h>

static void _start_as_stdio_echo_server(void)
{
    int c;
    while ((c = fgetc(stdin)) != EOF)
    {
        fputc(c, stdout);
    }
    exit(EXIT_SUCCESS);
}

static void _before_all_test(int argc, char* argv[])
{
    test_config_setup(argc, argv);

    if (test_config.flag_as_stdio_echo_server)
    {
        _start_as_stdio_echo_server();
    }

    if (!test_config.flag_no_memcheck)
    {
        setup_memcheck();
    }
}

static void _after_all_test(void)
{
    fflush(NULL);

    if (!test_config.flag_no_memcheck)
    {
        dump_memcheck();
    }
}

static const char* _cutest_get_log_level_str(cutest_log_level_t level)
{
    switch(level)
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

static cutest_hook_t test_hook = {
    _before_all_test,   /* .before_all_test */
    _after_all_test,    /* .after_all_test */
    NULL,               /* .before_fixture_setup */
    NULL,               /* .after_fixture_setup */
    NULL,               /* .before_fixture_teardown */
    NULL,               /* .after_fixture_teardown */
    NULL,               /* .before_fixture_test */
    NULL,               /* .after_fixture_test */
    NULL,               /* .before_parameterized_test */
    NULL,               /* .after_parameterized_test */
    NULL,               /* .before_simple_test */
    NULL,               /* .after_simple_test */
    _on_log,            /* .on_log_print */
};

int main(int argc, char* argv[])
{
    return cutest_run_tests(argc, argv, &test_hook);
}
