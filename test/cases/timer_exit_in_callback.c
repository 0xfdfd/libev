#include "ev.h"
#include "test.h"
#include <string.h>

struct test_6f93
{
    ev_loop_t   s_loop;
    ev_timer_t  s_timer;
    int         flag_timer_exit;
};

struct test_6f93 g_test_6f93;

static void _on_timer_exit(ev_timer_t* timer)
{
    ASSERT_EQ_PTR(timer, &g_test_6f93.s_timer);
    g_test_6f93.flag_timer_exit = 1;
}

static void _on_timer(ev_timer_t* timer)
{
    ASSERT_EQ_PTR(timer, &g_test_6f93.s_timer);
    ev_timer_exit(timer, _on_timer_exit);
}

TEST_FIXTURE_SETUP(timer)
{
    memset(&g_test_6f93, 0, sizeof(g_test_6f93));
    ASSERT_EQ_INT(ev_loop_init(&g_test_6f93.s_loop), 0);
    ASSERT_EQ_INT(ev_timer_init(&g_test_6f93.s_loop, &g_test_6f93.s_timer), 0);
}

TEST_FIXTURE_TEARDOWN(timer)
{
    ASSERT_EQ_EVLOOP(&g_test_6f93.s_loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_6f93.s_loop), 0);
}

TEST_F(timer, exit_in_callback)
{
    ASSERT_EQ_INT(ev_timer_start(&g_test_6f93.s_timer, _on_timer, 1, 1), 0);

    ASSERT_EQ_INT(ev_loop_run(&g_test_6f93.s_loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_INT(g_test_6f93.flag_timer_exit, 1);
}
