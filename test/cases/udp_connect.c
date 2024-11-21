#include "test.h"
#include "utils/random.h"
#include <string.h>

struct test_5295
{
    ev_loop_t       loop;           /**< Event loop */
    ev_udp_t        client;         /**< Client UDP socket */
    ev_udp_t        server;         /**< Server UDP socket */

    ev_udp_write_t  w_req;
    ev_udp_read_t   r_req;

    uint8_t         w_buf[1024];
    uint8_t         r_buf[2048];
};

struct test_5295*   g_test_5295 = NULL;

TEST_FIXTURE_SETUP(udp)
{
    g_test_5295 = ev_calloc(1, sizeof(*g_test_5295));

    test_random(&g_test_5295->w_buf, sizeof(g_test_5295->w_buf));

    ASSERT_EQ_INT(ev_loop_init(&g_test_5295->loop), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_5295->loop, &g_test_5295->client, AF_INET), 0);
    ASSERT_EQ_INT(ev_udp_init(&g_test_5295->loop, &g_test_5295->server, AF_INET), 0);
}

TEST_FIXTURE_TEARDOWN(udp)
{
    ev_udp_exit(&g_test_5295->client, NULL);
    ev_udp_exit(&g_test_5295->server, NULL);

    ASSERT_EQ_INT(ev_loop_run(&g_test_5295->loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
    ASSERT_EQ_EVLOOP(&g_test_5295->loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_5295->loop), 0);

    ev_free(g_test_5295);
    g_test_5295 = NULL;
}

static void _on_test_write_done_5295(ev_udp_write_t* req, ssize_t size)
{
    ASSERT_EQ_PTR(&g_test_5295->w_req, req);
    ASSERT_EQ_SSIZE(size, sizeof(g_test_5295->w_buf));
}

static void _on_test_read_done_5295(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    (void)addr;
    ASSERT_EQ_PTR(req, &g_test_5295->r_req);
    ASSERT_EQ_SSIZE(size, sizeof(g_test_5295->w_buf));

    ASSERT_EQ_INT(memcmp(g_test_5295->w_buf, g_test_5295->r_buf, size), 0);
}

TEST_F(udp, connect)
{
    /* Bind to an available address */
    struct sockaddr_in bind_addr;
    ASSERT_EQ_INT(ev_ipv4_addr("0.0.0.0", 0, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_bind(&g_test_5295->server, (struct sockaddr*)&bind_addr, 0), 0);

    /* Get bind address */
    size_t namelen = sizeof(bind_addr);
    ASSERT_EQ_INT(ev_udp_getsockname(&g_test_5295->server, (struct sockaddr*)&bind_addr, &namelen), 0);
    int port = 0;
    ASSERT_EQ_INT(ev_ipv4_name(&bind_addr, &port, NULL, 0), 0);

    /* Connect to address */
    ASSERT_EQ_INT(ev_ipv4_addr("127.0.0.1", port, &bind_addr), 0);
    ASSERT_EQ_INT(ev_udp_connect(&g_test_5295->client, (struct sockaddr*)&bind_addr), 0);

    /* Send data */
    {
        ev_buf_t buf = ev_buf_make(g_test_5295->w_buf, sizeof(g_test_5295->w_buf));
        ASSERT_EQ_INT(ev_udp_send(&g_test_5295->client, &g_test_5295->w_req, &buf, 1,
            NULL, _on_test_write_done_5295), 0);
    }

    /* Recv data */
    {
        ev_buf_t buf = ev_buf_make(&g_test_5295->r_buf, sizeof(g_test_5295->r_buf));
        ASSERT_EQ_INT(ev_udp_recv(&g_test_5295->server, &g_test_5295->r_req, &buf, 1,
            _on_test_read_done_5295), 0);
    }

    ASSERT_EQ_INT(ev_loop_run(&g_test_5295->loop, EV_LOOP_MODE_DEFAULT, EV_INFINITE_TIMEOUT), 0);
}
