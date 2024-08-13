#include "ev.h"
#include "test.h"
#include <string.h>

struct test_async
{
    ev_loop_t       s_loop;
    ev_async_t      s_async;
    int             f_called;
};

struct test_async*   g_test_sync = NULL;

static void _test_on_async(ev_async_t* handle)
{
    ASSERT_EQ_PTR(handle, &g_test_sync->s_async);
    g_test_sync->f_called = 1;
    ev_async_exit(handle, NULL);
}

TEST_FIXTURE_SETUP(async)
{
    g_test_sync = ev_calloc(1, sizeof(*g_test_sync));

    ASSERT_EQ_INT(ev_loop_init(&g_test_sync->s_loop), 0);
    ASSERT_EQ_INT(ev_async_init(&g_test_sync->s_loop, &g_test_sync->s_async, _test_on_async), 0);
}

TEST_FIXTURE_TEARDOWN(async)
{
    ASSERT_EQ_INT(ev_loop_run(&g_test_sync->s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_EVLOOP(&g_test_sync->s_loop, &empty_loop);
    ev_loop_exit(&g_test_sync->s_loop);

    ev_free(g_test_sync);
    g_test_sync = NULL;
}

TEST_F(async, async)
{
    ev_async_wakeup(&g_test_sync->s_async);
    ev_async_wakeup(&g_test_sync->s_async);
    ev_async_wakeup(&g_test_sync->s_async);
    ASSERT_EQ_INT(ev_loop_run(&g_test_sync->s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_INT(g_test_sync->f_called, 1);
}

TEST(async, static_initializer)
{
    static ev_async_t tmp = EV_ASYNC_INVALID;
    (void)tmp;
}
