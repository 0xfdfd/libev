#include "ev.h"
#include "test.h"

struct test_43e2
{
    ev_tcp_t     l_sock; /**< Listen socket */
    ev_tcp_t     s_sock; /**< Section socket */
    ev_tcp_t     c_sock; /**< Client socket */
    ev_loop_t    s_loop;
};

struct test_43e2 g_test_43e2;

TEST_FIXTURE_SETUP(tcp)
{
    ASSERT_EQ_INT(ev_loop_init(&g_test_43e2.s_loop), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_43e2.s_loop, &g_test_43e2.l_sock), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_43e2.s_loop, &g_test_43e2.s_sock), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_43e2.s_loop, &g_test_43e2.c_sock), 0);
}

TEST_FIXTURE_TEARDOWN(tcp)
{
    ev_tcp_exit(&g_test_43e2.l_sock, NULL);
    ev_tcp_exit(&g_test_43e2.s_sock, NULL);
    ev_tcp_exit(&g_test_43e2.c_sock, NULL);

    ASSERT_EQ_INT(ev_loop_run(&g_test_43e2.s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_EVLOOP(&g_test_43e2.s_loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_43e2.s_loop), 0);
}

static void _idle_client_on_accept(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    ASSERT_EQ_PTR(lisn, &g_test_43e2.l_sock);
    ASSERT_EQ_PTR(conn, &g_test_43e2.s_sock);
    ASSERT_EQ_INT(stat, 0);
}

static int _idle_client_setup_once_server(void)
{
    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);
    ASSERT_EQ_INT(ev_tcp_bind(&g_test_43e2.l_sock, (struct sockaddr*)&addr, sizeof(addr)), 0);

    size_t len = sizeof(addr);
    ASSERT_EQ_INT(ev_tcp_getsockname(&g_test_43e2.l_sock, (struct sockaddr*)&addr, &len), 0);

    int port = 0;
    ASSERT_EQ_INT(ev_ipv4_name(&addr, &port, NULL, 0), 0);
    ASSERT_GT_INT(port, 0);

    ASSERT_EQ_INT(ev_tcp_listen(&g_test_43e2.l_sock, 1024), 0);
    ASSERT_EQ_INT(ev_tcp_accept(&g_test_43e2.l_sock, &g_test_43e2.s_sock, _idle_client_on_accept), 0);

    return port;
}

static void _idle_client_on_connect(ev_tcp_t* sock, int stat)
{
    ASSERT_EQ_PTR(sock, &g_test_43e2.c_sock);
    ASSERT_EQ_INT(stat, 0);
}

static void _idle_client_connect(int port)
{
    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", port, &addr), 0);
    ASSERT_EQ_INT(ev_tcp_connect(&g_test_43e2.c_sock, (struct sockaddr*)&addr, sizeof(addr), _idle_client_on_connect), 0);
}

TEST_T(tcp, idle_client, 1000)
{
    int port = _idle_client_setup_once_server();
    _idle_client_connect(port);

    ASSERT_EQ_INT(ev_loop_run(&g_test_43e2.s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_INT(ev_loop_run(&g_test_43e2.s_loop, EV_LOOP_MODE_DEFAULT), 0);
}
