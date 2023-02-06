#include "ev.h"
#include "test.h"
#include <string.h>

struct test_0b72
{
    ev_loop_t       s_loop;
    ev_timer_t      s_timer;
};

struct test_0b72    g_test_0b72;

static void _on_timer(ev_timer_t* timer)
{
    ASSERT_EQ_PTR(timer, &g_test_0b72.s_timer);
    ev_loop_stop(&g_test_0b72.s_loop);
}

TEST_FIXTURE_SETUP(timer)
{
    memset(&g_test_0b72, 0, sizeof(g_test_0b72));
    ASSERT_EQ_INT(ev_loop_init(&g_test_0b72.s_loop), 0);
    ASSERT_EQ_INT(ev_timer_init(&g_test_0b72.s_loop, &g_test_0b72.s_timer), 0);
}

TEST_FIXTURE_TEAREDOWN(timer)
{
    ASSERT_EQ_EVLOOP(&g_test_0b72.s_loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_0b72.s_loop), 0);
}

TEST_F(timer, stop_loop)
{
    ASSERT_EQ_INT(ev_timer_start(&g_test_0b72.s_timer, _on_timer, 1, 1), 0);
    ASSERT_NE_INT(ev_loop_run(&g_test_0b72.s_loop, EV_LOOP_MODE_DEFAULT), 0);

    ev_timer_exit(&g_test_0b72.s_timer, NULL);
    ASSERT_EQ_INT(ev_loop_run(&g_test_0b72.s_loop, EV_LOOP_MODE_DEFAULT), 0);
}
