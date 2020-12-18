#include "test.h"
#include "ev.h"

static ev_once_t s_once_token = EV_ONCE_INIT;
static int s_count = 0;

static void _once_callback(void)
{
	s_count++;
}

TEST(once)
{
	ASSERT_EQ_D32(s_count, 0);
	ev_once_execute(&s_once_token, _once_callback);
	ASSERT_EQ_D32(s_count, 1);
	ev_once_execute(&s_once_token, _once_callback);
	ASSERT_EQ_D32(s_count, 1);
}
