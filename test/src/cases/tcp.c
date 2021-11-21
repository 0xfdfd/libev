#include "ev.h"
#include "test.h"

static ev_tcp_t     s_sock;
static ev_tcp_t     c_sock;
static ev_loop_t    s_loop;

TEST_FIXTURE_SETUP(tcp)
{
    ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
    ASSERT_EQ_D32(ev_tcp_init(&s_loop, &c_sock), 0);
    ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_sock), 0);
}

TEST_FIXTURE_TEAREDOWN(tcp)
{
    ev_loop_exit(&s_loop);
}

/**
 * @defgroup empty_loop
 * @{
 */

TEST_T(tcp, empty_loop, 1000)
{
    ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);
}

/**
 * @} empty_loop
 */

/**
 * @defgroup connect_non_exist
 * @{
 */

static void _connect_non_exist_on_ret(ev_tcp_t* sock, int stat)
{
    ASSERT_EQ_D32(stat, EV_ECONNREFUSED);
}

TEST_T(tcp, connect_non_exist, 1000)
{
    struct sockaddr_in addr;
    ASSERT_EQ_D32(ev_ipv4_addr("255.255.255.255", 0, &addr), 0);

    ASSERT_LE_D32(ev_tcp_connect(&c_sock, (struct sockaddr*)&addr, sizeof(addr), _connect_non_exist_on_ret), 0);
    ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

    ev_tcp_exit(&s_sock, NULL);
    ev_tcp_exit(&c_sock, NULL);
    ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);
}

/**
 * @} connect_non_exist
 */
