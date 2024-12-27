#include "test.h"

struct test_3b3b
{
    ev_mutex_t *mutex_r;
};

struct test_3b3b g_test_3b3b;

TEST_FIXTURE_SETUP(mutex)
{
    ev_mutex_init(&g_test_3b3b.mutex_r, 1);
}

TEST_FIXTURE_TEARDOWN(mutex)
{
    ev_mutex_exit(g_test_3b3b.mutex_r);
}

TEST_F(mutex, recursive)
{
    ev_mutex_enter(g_test_3b3b.mutex_r);
    ev_mutex_enter(g_test_3b3b.mutex_r);
    ASSERT_EQ_INT(ev_mutex_try_enter(g_test_3b3b.mutex_r), 0);

    ev_mutex_leave(g_test_3b3b.mutex_r);
    ev_mutex_leave(g_test_3b3b.mutex_r);
    ev_mutex_leave(g_test_3b3b.mutex_r);
}
