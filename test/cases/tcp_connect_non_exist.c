#include "test.h"

struct test_7145
{
    ev_tcp_t     s_sock;
    ev_tcp_t     c_sock;
    ev_loop_t    s_loop;
};

struct test_7145 g_test_7145;

TEST_FIXTURE_SETUP(tcp)
{
    ASSERT_EQ_INT(ev_loop_init(&g_test_7145.s_loop), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_7145.s_loop, &g_test_7145.c_sock), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_7145.s_loop, &g_test_7145.s_sock), 0);
}

TEST_FIXTURE_TEARDOWN(tcp)
{
    ev_tcp_exit(&g_test_7145.s_sock, NULL);
    ev_tcp_exit(&g_test_7145.c_sock, NULL);

    ASSERT_EQ_INT(ev_loop_run(&g_test_7145.s_loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_EVLOOP(&g_test_7145.s_loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_7145.s_loop), 0);
}

static void _connect_non_exist_on_ret(ev_tcp_t* sock, int stat)
{
    (void)sock;
    ASSERT_EQ_INT(stat, EV_ECONNREFUSED);
}

TEST_T(tcp, connect_non_exist, 1000)
{
    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("255.255.255.255", 0, &addr), 0);

    ASSERT_LE_INT(ev_tcp_connect(&g_test_7145.c_sock, (struct sockaddr*)&addr, sizeof(addr), _connect_non_exist_on_ret), 0);
    ASSERT_EQ_INT(ev_loop_run(&g_test_7145.s_loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
}
