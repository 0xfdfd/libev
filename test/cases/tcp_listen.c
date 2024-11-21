#if defined(_WIN32)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

#include "ev.h"
#include "test.h"

struct test_ec8c
{
    ev_loop_t    s_loop;
    ev_tcp_t     s_sock;
    int          s_flag_socket_close;
};

struct test_ec8c g_test_ec8c;

static void _on_close_socket(ev_tcp_t* sock)
{
    (void)sock;
    g_test_ec8c.s_flag_socket_close = 1;
}

TEST_FIXTURE_SETUP(tcp)
{
    g_test_ec8c.s_flag_socket_close = 0;
    ASSERT_EQ_INT(ev_loop_init(&g_test_ec8c.s_loop), 0);
    ASSERT_EQ_INT(ev_tcp_init(&g_test_ec8c.s_loop, &g_test_ec8c.s_sock), 0);
}

TEST_FIXTURE_TEARDOWN(tcp)
{
    ASSERT_EQ_EVLOOP(&g_test_ec8c.s_loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_ec8c.s_loop), 0);
}

TEST_F(tcp, bind)
{
    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);

    /* 1st bind should success */
    ASSERT_EQ_INT(ev_tcp_bind(&g_test_ec8c.s_sock, (struct sockaddr*)&addr, sizeof(addr)), 0);
    /* 1st bind should failure */
    ASSERT_NE_INT(ev_tcp_bind(&g_test_ec8c.s_sock, (struct sockaddr*)&addr, sizeof(addr)), 0);

    /* 1st listen should success */
    ASSERT_EQ_INT(ev_tcp_listen(&g_test_ec8c.s_sock, 1), 0);
    /* 2st listen should failure */
    ASSERT_NE_INT(ev_tcp_listen(&g_test_ec8c.s_sock, 1), 0);

    ev_tcp_exit(&g_test_ec8c.s_sock, _on_close_socket);
    ASSERT_EQ_INT(ev_loop_run(&g_test_ec8c.s_loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);

    ASSERT_EQ_INT(g_test_ec8c.s_flag_socket_close, 1);
}
