#include "test.h"
#include "ev.h"

static ev_loop_t	s_loop;
static ev_timer_t	s_timer;
static int flag_timer_exit = 0;

static void _on_timer_exit(ev_timer_t* timer)
{
	flag_timer_exit = 1;
}

static void _on_timer(ev_timer_t* timer)
{
	ev_timer_exit(timer, _on_timer_exit);
}

TEST(timer_stop_loop)
{
	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_timer_init(&s_loop, &s_timer), 0);
	ASSERT_EQ_D32(ev_timer_start(&s_timer, _on_timer, 1, 1), 0);

	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(flag_timer_exit, 1);
	ev_loop_exit(&s_loop);
}
