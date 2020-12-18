#include "test.h"
#include "ev.h"

static ev_loop_t	s_loop;
static ev_async_t	s_async;
static int			f_called = 0;

static void _test_on_async(ev_async_t* handle)
{
	f_called = 1;
	ev_async_exit(handle, NULL);
}

TEST(async)
{
	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_async_init(&s_loop, &s_async, _test_on_async), 0);

	ev_async_weakup(&s_async);
	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ev_loop_exit(&s_loop);
}
