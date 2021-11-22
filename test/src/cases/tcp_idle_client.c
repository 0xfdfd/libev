#include "ev.h"
#include "test.h"

static ev_tcp_t     l_sock; /**< Listen socket */
static ev_tcp_t     s_sock; /**< Section socket */
static ev_tcp_t     c_sock; /**< Client socket */
static ev_loop_t    s_loop;

TEST_FIXTURE_SETUP(tcp)
{
    ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
    ASSERT_EQ_D32(ev_tcp_init(&s_loop, &l_sock), 0);
    ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_sock), 0);
    ASSERT_EQ_D32(ev_tcp_init(&s_loop, &c_sock), 0);
}

TEST_FIXTURE_TEAREDOWN(tcp)
{
    ev_tcp_exit(&l_sock, NULL);
    ev_tcp_exit(&s_sock, NULL);
    ev_tcp_exit(&c_sock, NULL);
    ASSERT_EQ_D32(ev_loop_run(&s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ev_loop_exit(&s_loop);
}

static void _idle_client_on_accept(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    ASSERT_EQ_PTR(lisn, &l_sock);
    ASSERT_EQ_PTR(conn, &s_sock);
    ASSERT_EQ_D32(stat, EV_SUCCESS);
}

static int _idle_client_setup_once_server(void)
{
    struct sockaddr_in addr;
    ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);
    ASSERT_EQ_D32(ev_tcp_bind(&l_sock, (struct sockaddr*)&addr, sizeof(addr)), 0);

    size_t len = sizeof(addr);
    ASSERT_EQ_D32(ev_tcp_getsockname(&l_sock, (struct sockaddr*)&addr, &len), 0);

    int port = 0;
    ASSERT_EQ_D32(ev_ipv4_name(&addr, &port, NULL, 0), 0);
    ASSERT_GT_D32(port, 0);

    ASSERT_EQ_D32(ev_tcp_listen(&l_sock, 1024), 0);
    ASSERT_EQ_D32(ev_tcp_accept(&l_sock, &s_sock, _idle_client_on_accept), 0);

    return port;
}

static void _idle_client_on_connect(ev_tcp_t* sock, int stat)
{
    ASSERT_EQ_PTR(sock, &c_sock);
    ASSERT_EQ_D32(stat, EV_SUCCESS);
}

static void _idle_client_connect(int port)
{
    struct sockaddr_in addr;
    ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", port, &addr), 0);

    ASSERT_EQ_D32(ev_tcp_connect(&c_sock, (struct sockaddr*)&addr, sizeof(addr), _idle_client_on_connect), 0);
}

TEST_T(tcp, idle_client, 1000)
{
    int port = _idle_client_setup_once_server();
    _idle_client_connect(port);

    ASSERT_EQ_D32(ev_loop_run(&s_loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_D32(ev_loop_run(&s_loop, EV_LOOP_MODE_DEFAULT), 0);
}
