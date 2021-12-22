#include "test.h"

struct test_3b3b
{
    ev_os_mutex_t   mutex_r;
};

struct test_3b3b g_test_3b3b;

TEST_FIXTURE_SETUP(mutex)
{
    ASSERT_EQ_D32(ev_mutex_init(&g_test_3b3b.mutex_r, 1), 0);
}

TEST_FIXTURE_TEAREDOWN(mutex)
{
    ev_mutex_exit(&g_test_3b3b.mutex_r);
}

TEST_F(mutex, recursive)
{
    ev_mutex_enter(&g_test_3b3b.mutex_r);
    ev_mutex_enter(&g_test_3b3b.mutex_r);
    ASSERT_EQ_D32(ev_mutex_try_enter(&g_test_3b3b.mutex_r), EV_SUCCESS);

    ev_mutex_leave(&g_test_3b3b.mutex_r);
    ev_mutex_leave(&g_test_3b3b.mutex_r);
    ev_mutex_leave(&g_test_3b3b.mutex_r);
}
