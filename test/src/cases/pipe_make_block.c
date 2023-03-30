#include "test.h"
#include "utils/random.h"
#include <string.h>

#if !defined(_WIN32)
#   include <unistd.h>
#endif

typedef struct test_pipe_make
{
    ev_os_pipe_t            fds[2];
    ev_loop_t               loop;

    ev_work_t               write_token;
    ev_work_t               read_token;

    /* 8MB should exceed most system default underlying cache */
    char                    rbuf[8 * 1024 * 1024];
    char                    wbuf[8 * 1024 * 1024];

    size_t                  wsize;
    size_t                  rsize;

    int                     rlasterror; /**< Last error code for read */
    int                     wlasterror; /**< Last error code for write */
}test_pipe_make_t;

test_pipe_make_t*           g_test_pipe_make = NULL;

TEST_FIXTURE_SETUP(pipe)
{
    g_test_pipe_make = mmc_calloc(1, sizeof(*g_test_pipe_make));

    size_t i;
    for (i = 0; i < ARRAY_SIZE(g_test_pipe_make->fds); i++)
    {
        g_test_pipe_make->fds[i] = EV_OS_PIPE_INVALID;
    }

    ASSERT_EQ_INT(ev_loop_init(&g_test_pipe_make->loop), 0);

    test_random(g_test_pipe_make->wbuf, sizeof(g_test_pipe_make->wbuf));
}

TEST_FIXTURE_TEARDOWN(pipe)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(g_test_pipe_make->fds); i++)
    {
        if (g_test_pipe_make->fds[i] != EV_OS_PIPE_INVALID)
        {
            ev_pipe_close(g_test_pipe_make->fds[i]);
            g_test_pipe_make->fds[i] = EV_OS_PIPE_INVALID;
        }
    }

    ASSERT_EQ_INT(ev_loop_run(&g_test_pipe_make->loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_EVLOOP(&g_test_pipe_make->loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_pipe_make->loop), 0);

    mmc_free(g_test_pipe_make);
    g_test_pipe_make = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// pipe.make_block
///////////////////////////////////////////////////////////////////////////////

static void _test_pipe_make_block_on_write(ev_work_t* work)
{
    (void)work;

#if defined(_WIN32)
    DWORD write_size;
    BOOL ret = WriteFile(g_test_pipe_make->fds[1], g_test_pipe_make->wbuf,
        sizeof(g_test_pipe_make->wbuf), &write_size, NULL);

    ASSERT_EQ_INT(ret, TRUE, "GetLastError:%d", (int)GetLastError());
    ASSERT_EQ_UINT64(write_size, sizeof(g_test_pipe_make->wbuf));
#else
    ssize_t write_size = write(g_test_pipe_make->fds[1], g_test_pipe_make->wbuf,
        sizeof(g_test_pipe_make->wbuf));
    ASSERT_EQ_INT64(write_size, sizeof(g_test_pipe_make->wbuf));
#endif

    g_test_pipe_make->wsize = (size_t)write_size;
}

static void _test_pipe_make_block_on_write_done(ev_work_t* work, int status)
{
    (void)work; (void)status;
    ASSERT_EQ_SIZE(g_test_pipe_make->wsize, sizeof(g_test_pipe_make->wbuf));
}

static void _test_pipe_make_block_on_read(ev_work_t* work)
{
    (void)work;

    size_t readen_size = 0;
    while (readen_size < sizeof(g_test_pipe_make->rbuf))
    {
#if defined(_WIN32)
        DWORD read_size;
        void* read_pos = g_test_pipe_make->rbuf + readen_size;
        DWORD max_read_size = (DWORD)(sizeof(g_test_pipe_make->rbuf) - readen_size);
        BOOL ret = ReadFile(g_test_pipe_make->fds[0], read_pos, max_read_size,
            &read_size, NULL);
        ASSERT_EQ_INT(ret, TRUE, "GetLastError:%d", (int)GetLastError());
        ASSERT_GT_UINT64(read_size, 0);
#else
        void* read_pos = g_test_pipe_make->rbuf + readen_size;
        size_t max_read_size = sizeof(g_test_pipe_make->rbuf) - readen_size;
        ssize_t read_size = read(g_test_pipe_make->fds[0], read_pos, max_read_size);
        ASSERT_GT_INT64(read_size, 0);
#endif
        readen_size += (size_t)read_size;
    }

    g_test_pipe_make->rsize = (size_t)readen_size;
}

static void _test_pipe_make_block_on_read_done(ev_work_t* work, int status)
{
    (void)work; (void)status;
    ASSERT_EQ_SIZE(g_test_pipe_make->rsize, sizeof(g_test_pipe_make->rbuf));
}

TEST_F(pipe, make_block)
{
    int ret;

    ASSERT_EQ_INT(ev_pipe_make(g_test_pipe_make->fds, 0, 0), 0);
    ASSERT_NE_UINTPTR((uintptr_t)g_test_pipe_make->fds[0], (uintptr_t)EV_OS_PIPE_INVALID);
    ASSERT_NE_UINTPTR((uintptr_t)g_test_pipe_make->fds[1], (uintptr_t)EV_OS_PIPE_INVALID);

    ret = ev_loop_queue_work(&g_test_pipe_make->loop, &g_test_pipe_make->write_token,
        _test_pipe_make_block_on_write, _test_pipe_make_block_on_write_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_queue_work(&g_test_pipe_make->loop, &g_test_pipe_make->read_token,
        _test_pipe_make_block_on_read, _test_pipe_make_block_on_read_done);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(&g_test_pipe_make->loop, EV_LOOP_MODE_DEFAULT), 0);
}

///////////////////////////////////////////////////////////////////////////////
// pipe.make_nonblock_unix
///////////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32)

static void _test_pipe_make_nonblock_on_read(ev_work_t* work)
{
    (void)work;

    ssize_t read_size;
    while (g_test_pipe_make->rsize < sizeof(g_test_pipe_make->rbuf))
    {
        void* buf_pos = g_test_pipe_make->rbuf + g_test_pipe_make->rsize;
        size_t buf_size = sizeof(g_test_pipe_make->rbuf) - g_test_pipe_make->rsize;
        read_size = read(g_test_pipe_make->fds[0], buf_pos, buf_size);
        if (read_size < 0)
        {
            g_test_pipe_make->rlasterror = errno;
            break;
        }

        g_test_pipe_make->rsize += read_size;
    }
}

static void _test_pipe_make_nonblock_on_read_done(ev_work_t* work, int status)
{
    (void)work; (void)status;
}

static void _test_pipe_make_nonblock_on_write(ev_work_t* work)
{
    (void)work;
    ssize_t write_size = write(g_test_pipe_make->fds[1], g_test_pipe_make->wbuf,
            sizeof(g_test_pipe_make->wbuf));
    ASSERT_GT_INT64(write_size, 0);
    g_test_pipe_make->wsize = write_size;
}

static void _test_pipe_make_nonblock_on_write_done(ev_work_t* work, int status)
{
    (void)work; (void)status;
}

TEST_F(pipe, make_nonblock_unix)
{
    int ret;

    ret = ev_pipe_make(g_test_pipe_make->fds, EV_PIPE_NONBLOCK, EV_PIPE_NONBLOCK);
    ASSERT_EQ_INT(ret, 0);

    /* Read on nonblock pipe should get nothing. */
    {
        ret = ev_loop_queue_work(&g_test_pipe_make->loop, &g_test_pipe_make->read_token,
                _test_pipe_make_nonblock_on_read, _test_pipe_make_nonblock_on_read_done);
        ASSERT_EQ_INT(ret, 0);
        ASSERT_EQ_INT(ev_loop_run(&g_test_pipe_make->loop, EV_LOOP_MODE_DEFAULT), 0);
        ASSERT_EQ_SIZE(g_test_pipe_make->rsize, 0);
#if EAGAIN == EWOULDBLOCK
        ASSERT_EQ_INT(g_test_pipe_make->rlasterror, EAGAIN);
#else
        ASSERT(g_test_pipe_make->rlasterror == EAGAIN || g_test_pipe_make->rlasterror == EWOULDBLOCK);
#endif
    }

    /* Write on nonblock pipe should have fewer bytes than send data */
    {
        ret = ev_loop_queue_work(&g_test_pipe_make->loop, &g_test_pipe_make->write_token,
                _test_pipe_make_nonblock_on_write, _test_pipe_make_nonblock_on_write_done);
        ASSERT_EQ_INT(ret, 0);
        ASSERT_EQ_INT(ev_loop_run(&g_test_pipe_make->loop, EV_LOOP_MODE_DEFAULT), 0);
        ASSERT_GT_SIZE(g_test_pipe_make->wsize, 0);
        ASSERT_LT_SIZE(g_test_pipe_make->wsize, sizeof(g_test_pipe_make->wbuf));
    }
}

#endif
