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

typedef struct test_write_pack
{
	ev_write_t		w_req;
	ev_buf_t		buf;
	char			send_buf[4 * 1024 * 1024];
}test_write_pack_t;

typedef struct test_read_pack
{
	ev_read_t		r_req;
	ev_buf_t		buf;
	char			recv_buf[6 * 1024 * 1024];
}test_read_pack_t;

static ev_loop_t	s_loop;
static ev_tcp_t		s_server;
static ev_tcp_t		s_conn;
static ev_tcp_t		s_client;
static int			s_listen_port = -1;
static int			s_cnt_server_close = 0;
static int			s_cnt_conn_close = 0;
static int			s_cnt_client_close = 0;
static test_write_pack_t	s_write_pack;
static test_read_pack_t		s_read_pack;

static void _on_close_server_socket(ev_tcp_t* sock)
{
	ASSERT_EQ_PTR(sock, &s_server);
	s_cnt_server_close++;
}

static void _on_close_conn_socket(ev_tcp_t* sock)
{
	ASSERT_EQ_PTR(sock, &s_conn);
	s_cnt_conn_close++;
}

static void _on_close_client_socket(ev_tcp_t* sock)
{
	ASSERT_EQ_PTR(sock, &s_client);
	s_cnt_client_close++;
}

static void _on_send_finish(ev_write_t* req, size_t size, int stat)
{
	ASSERT_EQ_D32(stat, EV_SUCCESS);

	/* Close connection */
	ev_tcp_exit(&s_conn, _on_close_conn_socket);
}

static void _on_accept(ev_tcp_t* from, ev_tcp_t* to, int stat)
{
	ASSERT_EQ_PTR(&s_server, from);
	ASSERT_EQ_PTR(&s_conn, to);
	ASSERT_EQ_D32(stat, EV_SUCCESS);

	ev_tcp_exit(&s_server, _on_close_server_socket);
	ASSERT_EQ_D32(ev_tcp_write(to, &s_write_pack.w_req, &s_write_pack.buf, 1, _on_send_finish), 0);
}

static void _on_read(ev_read_t* req, size_t size, int stat)
{
	if (stat != EV_SUCCESS && stat != EV_EOF)
	{
		ABORT();
	}

	if (stat == EV_EOF)
	{
		int ret = memcmp(s_write_pack.send_buf, s_read_pack.recv_buf, sizeof(s_read_pack.recv_buf));
		ASSERT_EQ_D32(ret, 0);
		return;
	}

	ASSERT_EQ_D32(stat, EV_SUCCESS);

	s_read_pack.buf.data = (char*)s_read_pack.buf.data + size;
	s_read_pack.buf.size -= (unsigned)size;
	ASSERT_EQ_D32(ev_tcp_read(&s_client, &s_read_pack.r_req, &s_read_pack.buf, 1, _on_read), 0);
}

static void _on_connect(ev_tcp_t* sock, int stat)
{
	ASSERT_EQ_PTR(sock, &s_client);
	ASSERT_EQ_D32(stat, EV_SUCCESS);

	ASSERT_EQ_D32(ev_tcp_read(&s_client, &s_read_pack.r_req, &s_read_pack.buf, 1, _on_read), 0);
}

TEST(tcp_accept)
{
	memset(&s_read_pack, 0, sizeof(s_read_pack));
	s_read_pack.buf.data = s_read_pack.recv_buf;
	s_read_pack.buf.size = sizeof(s_read_pack.recv_buf);

	test_random(s_write_pack.send_buf, sizeof(s_write_pack.send_buf));
	s_write_pack.buf.data = s_write_pack.send_buf;
	s_write_pack.buf.size = sizeof(s_write_pack.send_buf);

	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_server), 0);
	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_conn), 0);
	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_client), 0);

	/* Listen */
	struct sockaddr_in addr;
	ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", 0, &addr), 0);
	ASSERT_EQ_D32(ev_tcp_bind(&s_server, (struct sockaddr*)&addr, sizeof(addr)), 0);
	ASSERT_EQ_D32(ev_tcp_listen(&s_server, 1), 0);
	ASSERT_EQ_D32(ev_tcp_accept(&s_server, &s_conn, _on_accept), 0);

	/* Get listen port */
	size_t len = sizeof(addr);
	ASSERT_EQ_D32(ev_tcp_getsockname(&s_server, (struct sockaddr*)&addr, &len), 0);
	ASSERT_EQ_D32(ev_ipv4_name(&addr, &s_listen_port, NULL, 0), 0);

	/* Connect to listen socket */
	ASSERT_EQ_D32(ev_ipv4_addr("127.0.0.1", s_listen_port, &addr), 0);
	ASSERT_EQ_D32(ev_tcp_connect(&s_client, (struct sockaddr*)&addr, sizeof(addr), _on_connect), 0,
		"%s", ev_strerror(_l));

	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	/* Close all socket */
	ev_tcp_exit(&s_client, _on_close_client_socket);
	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(s_cnt_server_close, 1);
	ev_loop_exit(&s_loop);
}
