#include "test.h"
#include <string.h>

struct test_6e87
{
    ev_loop_t       loop;           /**< Event loop */
    ev_udp_t        client;         /**< Multicast client */
    ev_udp_write_t  token;          /**< Send token */
    int             cnt_send;       /**< Send flag */
};

struct test_6e87    g_test_6e87;

TEST_FIXTURE_SETUP(udp)
{
    memset(&g_test_6e87, 0, sizeof(g_test_6e87));
    ASSERT_EQ_INT(ev_loop_init(&g_test_6e87.loop), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_6e87.loop, &g_test_6e87.client, AF_UNSPEC), 0);
}

TEST_FIXTURE_TEARDOWN(udp)
{
    ev_udp_exit(&g_test_6e87.client, NULL);
    ASSERT_EQ_INT(ev_loop_run(&g_test_6e87.loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_EVLOOP(&g_test_6e87.loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_6e87.loop), 0);
}

static void _on_send_finish_6e87(ev_udp_write_t* req, ssize_t size)
{
    g_test_6e87.cnt_send++;
    ASSERT_EQ_PTR(req, &g_test_6e87.token);
    ASSERT_EQ_SSIZE(size, 4);
}

TEST_F(udp, multicast_interface)
{
    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("0.0.0.0", 0, &addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_6e87.client, (struct sockaddr*)&addr, 0), 0);
    ASSERT_EQ_INT(ev_udp_set_multicast_interface(&g_test_6e87.client, "0.0.0.0"), 0);

    ev_buf_t buf = ev_buf_make("PING", 4);
    ASSERT_EQ_INT(ev_ipv4_addr("239.240.241.242", 10000, &addr), 0);
    ASSERT_EQ_INT(ev_udp_send(&g_test_6e87.client, &g_test_6e87.token, &buf, 1,
        (struct sockaddr*)&addr, _on_send_finish_6e87), 0);

    ASSERT_EQ_INT(g_test_6e87.cnt_send, 0);
    ASSERT_EQ_INT(ev_loop_run(&g_test_6e87.loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_INT(g_test_6e87.cnt_send, 1);
}
