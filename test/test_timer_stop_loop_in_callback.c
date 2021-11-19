#include "ev.h"
#include "test.h"

static ev_loop_t    s_loop;
static ev_timer_t   s_timer;

static void _on_timer(ev_timer_t* timer)
{
    ev_loop_stop(&s_loop);
}

TEST(timer, stop_loop)
{
    ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
    ASSERT_EQ_D32(ev_timer_init(&s_loop, &s_timer), 0);
    ASSERT_EQ_D32(ev_timer_start(&s_timer, _on_timer, 1, 1), 0);

    ASSERT_NE_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

    ev_timer_exit(&s_timer, NULL);
    ev_loop_exit(&s_loop);
}
