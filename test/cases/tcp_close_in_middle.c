#include "test.h"
#include "utils/random.h"

typedef struct test_send_data_pack
{
    char data[128 * 1024 * 1024];
} test_send_data_pack_t;

typedef struct test_recv_data_pack
{
    char buff[128 * 1024];
} test_recv_data_pack_t;

typedef struct test_tcp_close_in_middle
{
    ev_loop_t *loop; /**< Event loop. */

    int       l_sock_open;
    ev_tcp_t *l_sock; /**< Listen socket. */
    int       l_port; /**< Listen port. */

    int       s_sock_open;
    ev_tcp_t *s_sock; /**< Accepted socket. */

    int       c_sock_open;
    ev_tcp_t *c_sock; /**< Client socket. */
} test_tcp_close_in_middle_t;

test_tcp_close_in_middle_t g_test_tcp_close_in_middle;

TEST_FIXTURE_SETUP(tcp)
{
    memset(&g_test_tcp_close_in_middle, 0, sizeof(g_test_tcp_close_in_middle));

    /* Initialize */
    ASSERT_EQ_INT(ev_loop_init(&g_test_tcp_close_in_middle.loop), 0);
    ASSERT_EQ_INT(ev_tcp_init(g_test_tcp_close_in_middle.loop,
                              &g_test_tcp_close_in_middle.l_sock),
                  0);
    g_test_tcp_close_in_middle.l_sock_open = 1;
    ASSERT_EQ_INT(ev_tcp_init(g_test_tcp_close_in_middle.loop,
                              &g_test_tcp_close_in_middle.s_sock),
                  0);
    g_test_tcp_close_in_middle.s_sock_open = 1;
    ASSERT_EQ_INT(ev_tcp_init(g_test_tcp_close_in_middle.loop,
                              &g_test_tcp_close_in_middle.c_sock),
                  0);
    g_test_tcp_close_in_middle.c_sock_open = 1;

    /* Bind to local address */
    struct sockaddr_storage local_addr;
    ASSERT_EQ_INT(ev_ip_addr("127.0.0.1", 0, (struct sockaddr *)&local_addr,
                             sizeof(local_addr)),
                  0);
    ASSERT_EQ_INT(ev_tcp_bind(g_test_tcp_close_in_middle.l_sock,
                              (struct sockaddr *)&local_addr,
                              sizeof(local_addr)),
                  0);
    ASSERT_EQ_INT(ev_tcp_listen(g_test_tcp_close_in_middle.l_sock, 1024), 0);

    /* Get local bind port */
    size_t local_addr_sz = sizeof(local_addr);
    ASSERT_EQ_INT(ev_tcp_getsockname(g_test_tcp_close_in_middle.l_sock,
                                     (struct sockaddr *)&local_addr,
                                     &local_addr_sz),
                  0);
    ASSERT_EQ_INT(ev_ip_name((struct sockaddr *)&local_addr,
                             &g_test_tcp_close_in_middle.l_port, NULL, 0),
                  0);
}

TEST_FIXTURE_TEARDOWN(tcp)
{
    if (g_test_tcp_close_in_middle.l_sock_open)
    {
        g_test_tcp_close_in_middle.l_sock_open = 0;
        ev_tcp_exit(g_test_tcp_close_in_middle.l_sock, NULL, NULL);
    }

    if (g_test_tcp_close_in_middle.c_sock_open)
    {
        g_test_tcp_close_in_middle.c_sock_open = 0;
        ev_tcp_exit(g_test_tcp_close_in_middle.c_sock, NULL, NULL);
    }

    if (g_test_tcp_close_in_middle.s_sock_open)
    {
        g_test_tcp_close_in_middle.s_sock_open = 0;
        ev_tcp_exit(g_test_tcp_close_in_middle.s_sock, NULL, NULL);
    }

    ASSERT_EQ_INT(ev_loop_run(g_test_tcp_close_in_middle.loop,
                              EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT),
                  0);
    ASSERT_EQ_INT(ev_loop_exit(g_test_tcp_close_in_middle.loop), 0);
}

static void _test_tcp_close_in_middle_on_connect(ev_tcp_t *sock, int stat,
                                                 void *arg)
{
    (void)arg;
    ASSERT_EQ_INT(stat, 0);
    ASSERT_EQ_PTR(sock, g_test_tcp_close_in_middle.c_sock);
}

static void _test_tcp_close_in_middle_on_accept(ev_tcp_t *lisn, ev_tcp_t *conn,
                                                int stat, void *arg)
{
    (void)arg;
    ASSERT_EQ_INT(stat, 0);
    ASSERT_EQ_PTR(lisn, g_test_tcp_close_in_middle.l_sock);
    ASSERT_EQ_PTR(conn, g_test_tcp_close_in_middle.s_sock);
}

static void _test_tcp_close_in_middle_on_send(ev_tcp_t *sock, ssize_t size,
                                              void *arg)
{
    (void)sock;
    test_send_data_pack_t *send_data = (test_send_data_pack_t *)arg;
    ev_free(send_data);

    ASSERT_LT_SSIZE(size, 0);
}

static void _test_tcp_close_in_middle_on_recv(ev_tcp_t *sock, ssize_t size,
                                              void *arg)
{
    (void)sock;
    test_recv_data_pack_t *recv_data = (test_recv_data_pack_t *)arg;
    ev_free(recv_data);

    ASSERT_LT_SSIZE(size, 0);
}

TEST_F(tcp, close_in_middle)
{
    /* Try to connect to server. */
    struct sockaddr_storage peer_addr;
    ASSERT_EQ_INT(ev_ip_addr("127.0.0.1", g_test_tcp_close_in_middle.l_port,
                             (struct sockaddr *)&peer_addr, sizeof(peer_addr)),
                  0);
    ASSERT_EQ_INT(ev_tcp_connect(g_test_tcp_close_in_middle.c_sock,
                                 (struct sockaddr *)&peer_addr,
                                 sizeof(peer_addr),
                                 _test_tcp_close_in_middle_on_connect, NULL),
                  0);

    /* Try to accept connection. */
    ASSERT_EQ_INT(ev_tcp_accept(g_test_tcp_close_in_middle.l_sock,
                                g_test_tcp_close_in_middle.s_sock,
                                _test_tcp_close_in_middle_on_accept, NULL),
                  0);

    /* Establish connection. */
    ASSERT_EQ_INT(ev_loop_run(g_test_tcp_close_in_middle.loop,
                              EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT),
                  0);

    /* Send data. */
    test_send_data_pack_t *send_data = ev_malloc(sizeof(test_send_data_pack_t));
    ev_buf_t send_buf = ev_buf_make(send_data->data, sizeof(send_data->data));
    test_random(send_data->data, sizeof(send_data->data));
    ASSERT_EQ_INT(ev_tcp_write(g_test_tcp_close_in_middle.s_sock, &send_buf, 1,
                               _test_tcp_close_in_middle_on_send, send_data),
                  0);

    /* Recv data. */
    test_recv_data_pack_t *recv_data = ev_malloc(sizeof(test_recv_data_pack_t));
    ev_buf_t recv_buf = ev_buf_make(recv_data->buff, sizeof(recv_data->buff));
    ASSERT_EQ_INT(ev_tcp_read(g_test_tcp_close_in_middle.c_sock, &recv_buf, 1,
                              _test_tcp_close_in_middle_on_recv, recv_data),
                  0);

    g_test_tcp_close_in_middle.s_sock_open = 0;
    ev_tcp_exit(g_test_tcp_close_in_middle.s_sock, NULL, NULL);

    g_test_tcp_close_in_middle.c_sock_open = 0;
    ev_tcp_exit(g_test_tcp_close_in_middle.c_sock, NULL, NULL);

    ASSERT_EQ_INT(ev_loop_run(g_test_tcp_close_in_middle.loop,
                              EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT),
                  0);
}
