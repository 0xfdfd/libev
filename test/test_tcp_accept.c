#if defined(_WIN32)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

#include "ev.h"
#include "test.h"

static ev_loop_t	s_loop;
static ev_tcp_t		s_listen;
static ev_tcp_t		s_conn;
static int			s_flag_socket_close = 0;

static void _on_close_socket(ev_tcp_t* sock)
{
	(void)sock;
	s_flag_socket_close = 1;
}

static void _on_accept(ev_tcp_t* from, ev_tcp_t* to, int stat)
{

}

static void _connect_to_server(void)
{

}

TEST(tcp_accept)
{
	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);

	{
		static struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(0);
		ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_listen), 0);
		ASSERT_EQ_D32(ev_tcp_bind(&s_listen, (struct sockaddr*)&addr, sizeof(addr)), 0);
		ASSERT_EQ_D32(ev_tcp_listen(&s_listen, 1), 0);
	}

	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_conn), 0);
	ASSERT_EQ_D32(ev_tcp_accept(&s_listen, &s_conn, _on_accept), 0);

	_connect_to_server();

	ev_tcp_exit(&s_listen, _on_close_socket);
	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(s_flag_socket_close, 1);
	ev_loop_exit(&s_loop);
}
