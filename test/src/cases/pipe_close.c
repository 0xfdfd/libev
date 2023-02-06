#include "test.h"
#include <string.h>

#if defined(_WIN32)
#else
#   include <unistd.h>
#endif

typedef struct test_pipe_close
{
    ev_os_pipe_t    pipfd[2];
    char            buffer[16];
} test_pipe_close_t;

test_pipe_close_t   g_test_pipe_close;

TEST_FIXTURE_SETUP(pipe)
{
    memset(&g_test_pipe_close, 0, sizeof(g_test_pipe_close));
    g_test_pipe_close.pipfd[0] = EV_OS_PIPE_INVALID;
    g_test_pipe_close.pipfd[1] = EV_OS_PIPE_INVALID;
}

TEST_FIXTURE_TEAREDOWN(pipe)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(g_test_pipe_close.pipfd); i++)
    {
        if (g_test_pipe_close.pipfd[i] != EV_OS_PIPE_INVALID)
        {
            ev_pipe_close(g_test_pipe_close.pipfd[i]);
            g_test_pipe_close.pipfd[i] = EV_OS_PIPE_INVALID;
        }
    }
}

TEST_F(pipe, 0_0_close_read)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, 0, 0), EV_SUCCESS);

    /* Close read side */
    ev_pipe_close(g_test_pipe_close.pipfd[0]);
    g_test_pipe_close.pipfd[0] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD written_size;
    int ret = WriteFile(g_test_pipe_close.pipfd[1], "w", 1, &written_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_NO_DATA);   /* The pipe is being closed. */
#else
    ssize_t write_size = write(g_test_pipe_close.pipfd[1], "w", 1);
    ASSERT_EQ_INT64(write_size, -1);
    int errcode = errno;
    ASSERT_EQ_INT(errcode, EPIPE);
#endif
}

TEST_F(pipe, 0_0_close_write)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, 0, 0), EV_SUCCESS);

    /* Close write side */
    ev_pipe_close(g_test_pipe_close.pipfd[1]);
    g_test_pipe_close.pipfd[1] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD read_size;
    int ret = ReadFile(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer), &read_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_BROKEN_PIPE);   /* The pipe has been ended. */
#else
    ssize_t read_size = read(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer));
    ASSERT_EQ_INT64(read_size, 0);
#endif
}

TEST_F(pipe, 0_NONBLOCK_close_read)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, 0, EV_PIPE_NONBLOCK), EV_SUCCESS);

    /* Close read side */
    ev_pipe_close(g_test_pipe_close.pipfd[0]);
    g_test_pipe_close.pipfd[0] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD written_size;
    int ret = WriteFile(g_test_pipe_close.pipfd[1], "w", 1, &written_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_NO_DATA);   /* The pipe is being closed. */
#else
    ssize_t write_size = write(g_test_pipe_close.pipfd[1], "w", 1);
    ASSERT_EQ_INT64(write_size, -1);
    int errcode = errno;
    ASSERT_EQ_INT(errcode, EPIPE);
#endif
}

TEST_F(pipe, 0_NONBLOCK_close_write)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, 0, EV_PIPE_NONBLOCK), EV_SUCCESS);

    /* Close write side */
    ev_pipe_close(g_test_pipe_close.pipfd[1]);
    g_test_pipe_close.pipfd[1] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD read_size;
    int ret = ReadFile(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer), &read_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_BROKEN_PIPE);   /* The pipe has been ended. */
#else
    ssize_t read_size = read(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer));
    ASSERT_EQ_INT64(read_size, 0);
#endif
}

TEST_F(pipe, NONBLOCK_0_close_read)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, EV_PIPE_NONBLOCK, 0), EV_SUCCESS);

    /* Close read side */
    ev_pipe_close(g_test_pipe_close.pipfd[0]);
    g_test_pipe_close.pipfd[0] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD written_size;
    int ret = WriteFile(g_test_pipe_close.pipfd[1], "w", 1, &written_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_NO_DATA);   /* The pipe is being closed. */
#else
    ssize_t write_size = write(g_test_pipe_close.pipfd[1], "w", 1);
    ASSERT_EQ_INT64(write_size, -1);
    int errcode = errno;
    ASSERT_EQ_INT(errcode, EPIPE);
#endif
}

TEST_F(pipe, NONBLOCK_0_close_write)
{
    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_close.pipfd, EV_PIPE_NONBLOCK, 0), EV_SUCCESS);

    /* Close write side */
    ev_pipe_close(g_test_pipe_close.pipfd[1]);
    g_test_pipe_close.pipfd[1] = EV_OS_PIPE_INVALID;

#if defined(_WIN32)
    DWORD read_size;
    int ret = ReadFile(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer), &read_size, NULL);
    ASSERT_EQ_INT(ret, FALSE);
    ASSERT_EQ_INT(GetLastError(), ERROR_BROKEN_PIPE);   /* The pipe has been ended. */
#else
    ssize_t read_size = read(g_test_pipe_close.pipfd[0], g_test_pipe_close.buffer,
        sizeof(g_test_pipe_close.buffer));
    ASSERT_EQ_INT64(read_size, 0);
#endif
}
