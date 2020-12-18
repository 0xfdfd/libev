#include "test.h"
#include "ev.h"

static ev_loop_t s_loop;
static ev_timer_t s_timer;

static int f_on_timer = 0;
static int f_on_timer_close = 0;

static void _on_timer(ev_timer_t* timer)
{
	f_on_timer = 1;
	ev_timer_stop(timer);
}

static void _on_timer_close(ev_timer_t* timer)
{
	f_on_timer_close = 1;
}

TEST(timer)
{
	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_timer_init(&s_loop, &s_timer), 0);
	ASSERT_EQ_D32(ev_timer_start(&s_timer, _on_timer, 1000, 1000), 0);

	ASSERT_EQ_D32(f_on_timer, 0);
	ASSERT_EQ_D32(f_on_timer_close, 0);
	
	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(f_on_timer, 1);
	ASSERT_EQ_D32(f_on_timer_close, 0);

	ev_timer_exit(&s_timer, _on_timer_close);

	ASSERT_EQ_D32(f_on_timer, 1);
	ASSERT_EQ_D32(f_on_timer_close, 0);

	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(f_on_timer, 1);
	ASSERT_EQ_D32(f_on_timer_close, 1);

	ev_loop_exit(&s_loop);
}
