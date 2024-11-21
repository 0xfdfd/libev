#include "test.h"
#include <string.h>

struct test_7ab9
{
    ev_loop_t       loop;           /**< Event loop */
    ev_udp_t        client;         /**< Client */
};

struct test_7ab9    g_test_7ab9;

TEST_FIXTURE_SETUP(udp)
{
    memset(&g_test_7ab9, 0, sizeof(g_test_7ab9));
    ASSERT_EQ_INT(ev_loop_init(&g_test_7ab9.loop), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_7ab9.loop, &g_test_7ab9.client, AF_UNSPEC), 0);

    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("0.0.0.0", 0, &addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_7ab9.client, (struct sockaddr*)&addr, 0), 0);
}

TEST_FIXTURE_TEARDOWN(udp)
{
    ev_udp_exit(&g_test_7ab9.client, NULL);
    ASSERT_EQ_INT(ev_loop_run(&g_test_7ab9.loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_EVLOOP(&g_test_7ab9.loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_7ab9.loop), 0);
}

TEST_F(udp, ttl_invalid)
{
    ASSERT_NE_INT(ev_udp_set_ttl(&g_test_7ab9.client, 0), 0);
    ASSERT_NE_INT(ev_udp_set_ttl(&g_test_7ab9.client, 256), 0);
}

TEST_F(udp, ttl)
{
    ASSERT_EQ_INT(ev_udp_set_ttl(&g_test_7ab9.client, 32), 0);
}
