#define _GNU_SOURCE
#include "test.h"
#include "tools/memcheck.h"
#include "utils/config.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if !defined(_WIN32)
#   include <unistd.h>
#endif

static void _start_as_stdio_echo_server(void)
{
    char buffer[1024];

#if defined(_WIN32)
    BOOL bSuccess;
    DWORD errcode;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE || hStdin == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "invalid stdio handle.\n");
        exit(EXIT_SUCCESS);
    }

    for (;;)
    {
        DWORD dwread;
        bSuccess = ReadFile(hStdin, buffer, sizeof(buffer), &dwread, NULL);
        if (!bSuccess)
        {
            errcode = GetLastError();
            fprintf(stderr, "ReadFile failed: %d.\n", (long)errcode);
            exit(EXIT_SUCCESS);
        }

        DWORD dwwrite;
        bSuccess = WriteFile(hStdout, buffer, dwread, &dwwrite, NULL);
        if (!bSuccess)
        {
            errcode = GetLastError();
            fprintf(stderr, "WriteFile failed: %d.\n", (long)errcode);
            exit(EXIT_SUCCESS);
        }
    }
#else
    for (;;)
    {
        ssize_t read_size = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (read_size == 0)
        {
            fprintf(stderr, "stdin EOF.\n");
            exit(EXIT_SUCCESS);
        }
        if (read_size < 0)
        {
            fprintf(stderr, "stdin error: %s(%d).\n", strerror(errno), errno);
            exit(EXIT_SUCCESS);
        }

        ssize_t write_size = write(STDOUT_FILENO, buffer, read_size);
        if (write_size < 0)
        {
            fprintf(stderr, "stdout error: %s(%d).\n", strerror(errno), errno);
            exit(EXIT_SUCCESS);
        }
    }

#endif
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

cutest_hook_t test_hook = {
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

char* test_strdup(const char* str)
{
#if defined(_WIN32)
    return _strdup(str);
#else
    return strdup(str);
#endif
}
