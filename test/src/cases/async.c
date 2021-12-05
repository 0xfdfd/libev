#include "ev.h"
#include "test.h"
#include <string.h>

struct test_5b9c
{
    ev_loop_t    s_loop;
    ev_async_t   s_async;
    int          f_called;
};

struct test_5b9c g_test_5b9c;

static void _test_on_async(ev_async_t* handle)
{
    ASSERT_EQ_PTR(handle, &g_test_5b9c.s_async);
    g_test_5b9c.f_called = 1;
    ev_async_exit(handle, NULL);
}

TEST_FIXTURE_SETUP(async)
{
    memset(&g_test_5b9c, 0, sizeof(g_test_5b9c));
    ASSERT_EQ_D32(ev_loop_init(&g_test_5b9c.s_loop), 0);
    ASSERT_EQ_D32(ev_async_init(&g_test_5b9c.s_loop, &g_test_5b9c.s_async, _test_on_async), 0);
}

TEST_FIXTURE_TEAREDOWN(async)
{
    ASSERT_EQ_D32(ev_loop_run(&g_test_5b9c.s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ev_loop_exit(&g_test_5b9c.s_loop);
}

TEST_F(async, async)
{
    ev_async_weakup(&g_test_5b9c.s_async);
    ev_async_weakup(&g_test_5b9c.s_async);
    ev_async_weakup(&g_test_5b9c.s_async);
    ASSERT_EQ_D32(ev_loop_run(&g_test_5b9c.s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_D32(g_test_5b9c.f_called, 1);
}
