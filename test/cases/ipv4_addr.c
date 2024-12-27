#include "ev.h"
#include "test.h"
#include <string.h>

struct test_09d8
{
    ev_loop_t *s_loop;
    ev_tcp_t  *s_socket;
    char       s_buffer[64];
};

struct test_09d8 g_test_09d8;

TEST_FIXTURE_SETUP(misc)
{
    memset(&g_test_09d8, 0, sizeof(g_test_09d8));
    ASSERT_EQ_INT(ev_loop_init(&g_test_09d8.s_loop), 0);
    ASSERT_EQ_INT(ev_tcp_init(g_test_09d8.s_loop, &g_test_09d8.s_socket), 0);
}

TEST_FIXTURE_TEARDOWN(misc)
{
    ev_tcp_exit(g_test_09d8.s_socket, NULL, NULL);
    ASSERT_EQ_INT(ev_loop_run(g_test_09d8.s_loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    ev_loop_exit(g_test_09d8.s_loop);
}

TEST_F(misc, ipv4_addr)
{
    const char *ip = "127.0.0.1";

    struct sockaddr_in addr;
    size_t             len = sizeof(addr);

    ASSERT_EQ_INT(ev_ipv4_addr(ip, 0, &addr), 0);
    ASSERT_EQ_INT(ev_tcp_bind(g_test_09d8.s_socket, (struct sockaddr *)&addr,
                              sizeof(addr)),
                  0);

    memset(&addr, 0, sizeof(addr));
    ASSERT_EQ_INT(ev_tcp_getsockname(g_test_09d8.s_socket,
                                     (struct sockaddr *)&addr, &len),
                  0);

    ASSERT_EQ_INT(ev_ipv4_name(&addr, NULL, g_test_09d8.s_buffer,
                               sizeof(g_test_09d8.s_buffer)),
                  0);
    ASSERT_EQ_STR(ip, g_test_09d8.s_buffer);
}
