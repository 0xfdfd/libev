#include "sockpair.h"
#include "cutest.h"

typedef struct test_sockpair_ctx
{
    ev_tcp_t a_sock;
}test_sockpair_ctx_t;

static void _test_sockpair_on_accept(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    (void)conn;
    ASSERT_EQ_INT(stat, 0);
    ev_tcp_exit(lisn, NULL);
}

static void _test_sockpair_on_connect(ev_tcp_t* sock, int stat)
{
    (void)sock;
    ASSERT_EQ_INT(stat, 0);
}

void test_sockpair(ev_loop_t* loop, ev_tcp_t* s_sock, ev_tcp_t* c_sock)
{
    test_sockpair_ctx_t ctx;
    ASSERT_EQ_INT(ev_tcp_init(loop, &ctx.a_sock), 0);

    struct sockaddr_in addr;
    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);
    ASSERT_EQ_INT(ev_tcp_bind(&ctx.a_sock, (struct sockaddr*)&addr, sizeof(addr)), 0);

    size_t len = sizeof(addr);
    ASSERT_EQ_INT(ev_tcp_getsockname(&ctx.a_sock, (struct sockaddr*)&addr, &len), 0);

    int port = 0;
    ASSERT_EQ_INT(ev_ipv4_name(&addr, &port, NULL, 0), 0);
    ASSERT_GT_INT(port, 0);

    ASSERT_EQ_INT(ev_tcp_listen(&ctx.a_sock, 1), 0);
    ASSERT_EQ_INT(ev_tcp_accept(&ctx.a_sock, s_sock, _test_sockpair_on_accept), 0);

    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", port, &addr), 0);
    ASSERT_EQ_INT(ev_tcp_connect(c_sock, (struct sockaddr*)&addr, sizeof(addr), _test_sockpair_on_connect), 0);

    ASSERT_EQ_INT(ev_loop_run(loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
}
