#include "ev.h"
#include "test.h"

struct test_b380
{
    ev_once_t   s_once_token;
    int         s_count;
};

struct test_b380 g_test_b380;

static void _once_callback(void)
{
    g_test_b380.s_count++;
}

TEST_FIXTURE_SETUP(misc)
{
    g_test_b380.s_once_token = (ev_once_t)EV_ONCE_INIT;
    g_test_b380.s_count = 0;
}

TEST_FIXTURE_TEAREDOWN(misc)
{

}

TEST_F(misc, once)
{
    ASSERT_EQ_D32(g_test_b380.s_count, 0);
    ev_once_execute(&g_test_b380.s_once_token, _once_callback);
    ASSERT_EQ_D32(g_test_b380.s_count, 1);
    ev_once_execute(&g_test_b380.s_once_token, _once_callback);
    ASSERT_EQ_D32(g_test_b380.s_count, 1);
}
