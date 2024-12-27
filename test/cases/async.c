#include "ev.h"
#include "test.h"

#define MAGIC_PTR ((void *)(uintptr_t)(0xfdfd))

struct test_async
{
    ev_loop_t  *s_loop;
    ev_async_t *s_async;
    int         f_called;
};

struct test_async *g_test_sync = NULL;

static void test_on_async(ev_async_t *handle, void *arg)
{
    ASSERT_EQ_PTR(handle, g_test_sync->s_async);
    ASSERT_EQ_PTR(arg, MAGIC_PTR);
    g_test_sync->f_called = 1;
}

TEST_FIXTURE_SETUP(async)
{
    g_test_sync = ev_calloc(1, sizeof(*g_test_sync));

    ASSERT_EQ_INT(ev_loop_init(&g_test_sync->s_loop), 0);
    ASSERT_EQ_INT(ev_async_init(g_test_sync->s_loop, &g_test_sync->s_async,
                                test_on_async, MAGIC_PTR),
                  0);
}

TEST_FIXTURE_TEARDOWN(async)
{
    ev_async_exit(g_test_sync->s_async, NULL, NULL);
    ASSERT_EQ_INT(ev_loop_run(g_test_sync->s_loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    ASSERT_EQ_INT(ev_loop_exit(g_test_sync->s_loop), 0);

    ev_free(g_test_sync);
    g_test_sync = NULL;
}

TEST_F(async, async)
{
    ev_async_wakeup(g_test_sync->s_async);
    ev_async_wakeup(g_test_sync->s_async);
    ev_async_wakeup(g_test_sync->s_async);
    ev_loop_run(g_test_sync->s_loop, EV_LOOP_MODE_DEFAULT, 0);
    ASSERT_EQ_INT(g_test_sync->f_called, 1);
}
