#include "test.h"
#include <string.h>

struct test_ffe1
{
    ev_loop_t       loop;   /**< Event loop */
    ev_udp_t        udp1;   /**< UDP socket 1 */
    ev_udp_t        udp2;   /**< UDP socket 1 */
};

struct test_ffe1    g_test_ffe1;

TEST_FIXTURE_SETUP(udp)
{
    memset(&g_test_ffe1, 0, sizeof(g_test_ffe1));

    ASSERT_EQ_INT(ev_loop_init(&g_test_ffe1.loop), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_ffe1.loop, &g_test_ffe1.udp1, AF_UNSPEC), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_ffe1.loop, &g_test_ffe1.udp2, AF_UNSPEC), 0);
}

TEST_FIXTURE_TEARDOWN(udp)
{
    ev_udp_exit(&g_test_ffe1.udp1, NULL);
    ev_udp_exit(&g_test_ffe1.udp2, NULL);

    ASSERT_EQ_INT(ev_loop_run(&g_test_ffe1.loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_EVLOOP(&g_test_ffe1.loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_ffe1.loop), 0);
}

TEST_F(udp, bind)
{
    const char* bind_ip = "127.0.0.1";
    struct sockaddr_in bind_addr;
    ASSERT_EQ_INT(ev_ipv4_addr(bind_ip, 0, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_ffe1.udp1, (struct sockaddr*)&bind_addr, 0), 0);

    size_t namelen = sizeof(bind_addr);
    ASSERT_EQ_INT(ev_udp_getsockname(&g_test_ffe1.udp1, (struct sockaddr*)&bind_addr, &namelen), 0);

    int bind_port = 0;
    ASSERT_EQ_INT(ev_ipv4_name(&bind_addr, &bind_port, NULL, 0), 0);
    ASSERT_NE_INT(bind_port, 0);

    ASSERT_EQ_INT(ev_ipv4_addr(bind_ip, bind_port, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_ffe1.udp2, (struct sockaddr*)&bind_addr, 0), EV_EADDRINUSE);
}

TEST_F(udp, bind_reuseaddr)
{
    const char* bind_ip = "127.0.0.1";
    struct sockaddr_in bind_addr;
    ASSERT_EQ_INT(ev_ipv4_addr(bind_ip, 0, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_ffe1.udp1, (struct sockaddr*)&bind_addr, EV_UDP_REUSEADDR), 0);

    size_t namelen = sizeof(bind_addr);
    ASSERT_EQ_INT(ev_udp_getsockname(&g_test_ffe1.udp1, (struct sockaddr*)&bind_addr, &namelen), 0);

    int bind_port = 0;
    ASSERT_EQ_INT(ev_ipv4_name(&bind_addr, &bind_port, NULL, 0), 0);
    ASSERT_NE_INT(bind_port, 0);

    ASSERT_EQ_INT(ev_ipv4_addr(bind_ip, bind_port, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_ffe1.udp2, (struct sockaddr*)&bind_addr, EV_UDP_REUSEADDR), 0);
}
