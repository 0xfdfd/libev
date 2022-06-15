#include "ev.h"
#include "test.h"
#include <string.h>

struct test_3615
{
    ev_loop_t       s_loop;
    ev_timer_t      s_timer;

    int             f_on_timer;
    int             f_on_timer_close;
};

struct test_3615    g_test_3615;

static void _on_timer_close(ev_timer_t* timer)
{
    (void)timer;
    g_test_3615.f_on_timer_close = 1;
}

TEST_FIXTURE_SETUP(timer)
{
    memset(&g_test_3615, 0, sizeof(g_test_3615));
    ASSERT_EQ_D32(ev_loop_init(&g_test_3615.s_loop), 0);
    ASSERT_EQ_D32(ev_timer_init(&g_test_3615.s_loop, &g_test_3615.s_timer), 0);
}

TEST_FIXTURE_TEAREDOWN(timer)
{
    ev_timer_exit(&g_test_3615.s_timer, _on_timer_close);
    ASSERT_EQ_D32(g_test_3615.f_on_timer_close, 0);
    ASSERT_EQ_D32(ev_loop_run(&g_test_3615.s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_D32(g_test_3615.f_on_timer_close, 1);

    ASSERT_LOOP_EMPTY(&g_test_3615.s_loop);
    ASSERT_EQ_D32(ev_loop_exit(&g_test_3615.s_loop), 0);
}

static void _on_timer(ev_timer_t* timer)
{
    g_test_3615.f_on_timer = 1;
    ev_timer_stop(timer);
}

TEST_F(timer, normal)
{
    ASSERT_EQ_D32(ev_timer_start(&g_test_3615.s_timer, _on_timer, 1000, 1000), 0);

    ASSERT_EQ_D32(g_test_3615.f_on_timer, 0);
    
    ASSERT_EQ_D32(ev_loop_run(&g_test_3615.s_loop, EV_LOOP_MODE_DEFAULT), 0);

    ASSERT_EQ_D32(g_test_3615.f_on_timer, 1);
}

TEST_F(timer, static_initializer)
{
    ev_timer_t tmp = EV_TIMER_INVALID;
    (void)tmp;
}
