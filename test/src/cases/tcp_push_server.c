#if defined(_WIN32)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

#include "ev.h"
#include "test.h"
#include "utils/random.h"

struct test_6d69
{
    ev_loop_t               s_loop;
    ev_tcp_t                s_server;
    ev_tcp_t                s_conn;
    ev_tcp_t                s_client;
    int                     s_listen_port;
    int                     s_cnt_server_close;
    int                     s_cnt_conn_close;
    int                     s_cnt_client_close;

    struct
    {
        ev_tcp_write_req_t  w_req;
        ev_buf_t            buf;
        char                send_buf[4 * 1024 * 1024];
    }s_write_pack;

    struct
    {
        ev_tcp_read_req_t   r_req;
        ev_buf_t            buf;
        size_t              pos;
        char                recv_buf[6 * 1024 * 1024];
    }s_read_pack;
};

struct test_6d69*           g_test_6d69 = NULL;

static void _on_close_server_socket(ev_tcp_t* sock)
{
    ASSERT_EQ_PTR(sock, &g_test_6d69->s_server);
    g_test_6d69->s_cnt_server_close++;
}

static void _on_close_conn_socket(ev_tcp_t* sock)
{
    ASSERT_EQ_PTR(sock, &g_test_6d69->s_conn);
    g_test_6d69->s_cnt_conn_close++;
}

static void _on_close_client_socket_6d69(ev_tcp_t* sock)
{
    ASSERT_EQ_PTR(sock, &g_test_6d69->s_client);
    g_test_6d69->s_cnt_client_close++;
}

static void _on_send_finish_6d69(ev_tcp_write_req_t* req, size_t size, int stat)
{
    (void)req;
    ASSERT_EQ_D32(size, sizeof(g_test_6d69->s_write_pack.send_buf));
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    /* Close connection */
    ev_tcp_exit(&g_test_6d69->s_conn, _on_close_conn_socket);
}

static void _on_accept_6d69(ev_tcp_t* from, ev_tcp_t* to, int stat)
{
    ASSERT_EQ_PTR(&g_test_6d69->s_server, from);
    ASSERT_EQ_PTR(&g_test_6d69->s_conn, to);
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    ev_tcp_exit(&g_test_6d69->s_server, _on_close_server_socket);
    ASSERT_EQ_D32(ev_tcp_write(to, &g_test_6d69->s_write_pack.w_req,
        &g_test_6d69->s_write_pack.buf, 1, _on_send_finish_6d69), 0);
}

static void _on_read_6d69(ev_tcp_read_req_t* req, size_t size, int stat)
{
    (void)req;

    ASSERT_LE_D32(size, sizeof(g_test_6d69->s_write_pack.send_buf));
    g_test_6d69->s_read_pack.pos += size;

    if (stat == EV_EOF)
    {
        ASSERT_EQ_D32(g_test_6d69->s_read_pack.pos, sizeof(g_test_6d69->s_write_pack.send_buf));
        int ret = memcmp(g_test_6d69->s_write_pack.send_buf,
            g_test_6d69->s_read_pack.recv_buf, sizeof(g_test_6d69->s_write_pack.send_buf));
        ASSERT_EQ_D32(ret, 0);
        return;
    }

    ASSERT_EQ_D32(stat, EV_SUCCESS);

    g_test_6d69->s_read_pack.buf =
        ev_buf_make(g_test_6d69->s_read_pack.recv_buf + g_test_6d69->s_read_pack.pos,
            sizeof(g_test_6d69->s_read_pack.recv_buf) - g_test_6d69->s_read_pack.pos);
    ASSERT_EQ_D32(ev_tcp_read(&g_test_6d69->s_client, &g_test_6d69->s_read_pack.r_req,
        &g_test_6d69->s_read_pack.buf, 1, _on_read_6d69), 0);
}

static void _on_connect_6d69(ev_tcp_t* sock, int stat)
{
    ASSERT_EQ_PTR(sock, &g_test_6d69->s_client);
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    ASSERT_EQ_D32(ev_tcp_read(&g_test_6d69->s_client, &g_test_6d69->s_read_pack.r_req,
        &g_test_6d69->s_read_pack.buf, 1, _on_read_6d69), 0);
}

TEST_FIXTURE_SETUP(tcp)
{
    g_test_6d69 = memcheck_calloc(1, sizeof(*g_test_6d69));
    g_test_6d69->s_listen_port = -1;

    ASSERT_EQ_D32(ev_loop_init(&g_test_6d69->s_loop), 0);
    ASSERT_EQ_D32(ev_tcp_init(&g_test_6d69->s_loop, &g_test_6d69->s_server), 0);
    ASSERT_EQ_D32(ev_tcp_init(&g_test_6d69->s_loop, &g_test_6d69->s_conn), 0);
    ASSERT_EQ_D32(ev_tcp_init(&g_test_6d69->s_loop, &g_test_6d69->s_client), 0);

    test_random(g_test_6d69->s_write_pack.send_buf, sizeof(g_test_6d69->s_write_pack.send_buf));
}

TEST_FIXTURE_TEAREDOWN(tcp)
{
    ev_tcp_exit(&g_test_6d69->s_client, _on_close_client_socket_6d69);
    ASSERT_EQ_D32(ev_loop_run(&g_test_6d69->s_loop, EV_LOOP_MODE_DEFAULT), 0);

    ASSERT_EQ_D32(g_test_6d69->s_cnt_server_close, 1);
    ev_loop_exit(&g_test_6d69->s_loop);

    memcheck_free(g_test_6d69);
    g_test_6d69 = NULL;
}

TEST_F(tcp, push_server)
{
    g_test_6d69->s_read_pack.buf =
        ev_buf_make(g_test_6d69->s_read_pack.recv_buf, sizeof(g_test_6d69->s_read_pack.recv_buf));

    g_test_6d69->s_write_pack.buf =
        ev_buf_make(g_test_6d69->s_write_pack.send_buf, sizeof(g_test_6d69->s_write_pack.send_buf));

    /* Listen */
    struct sockaddr_in addr;
    ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);
    ASSERT_EQ_D32(ev_tcp_bind(&g_test_6d69->s_server, (struct sockaddr*)&addr, sizeof(addr)), 0);
    ASSERT_EQ_D32(ev_tcp_listen(&g_test_6d69->s_server, 1), 0);
    ASSERT_EQ_D32(ev_tcp_accept(&g_test_6d69->s_server, &g_test_6d69->s_conn, _on_accept_6d69), 0);

    /* Get listen port */
    size_t len = sizeof(addr);
    ASSERT_EQ_D32(ev_tcp_getsockname(&g_test_6d69->s_server, (struct sockaddr*)&addr, &len), 0);
    ASSERT_EQ_D32(ev_ipv4_name(&addr, &g_test_6d69->s_listen_port, NULL, 0), 0);

    /* Connect to listen socket */
    ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", g_test_6d69->s_listen_port, &addr), 0);
    ASSERT_EQ_D32(ev_tcp_connect(&g_test_6d69->s_client, (struct sockaddr*)&addr,
        sizeof(addr), _on_connect_6d69), 0,
        "%s", ev_strerror(_1));

    ASSERT_EQ_D32(ev_loop_run(&g_test_6d69->s_loop, EV_LOOP_MODE_DEFAULT), 0);
}
