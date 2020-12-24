#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "test.h"
#include "ev.h"

static ev_loop_t	s_loop;
static ev_tcp_t		s_sock;
static int			s_flag_socket_close = 0;

static void _on_close_socket(ev_tcp_t* sock)
{
	(void)sock;
	s_flag_socket_close = 1;
}

TEST(tcp_bind)
{
	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_sock, 0), 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = 0;
	size_t addrlen = sizeof(addr);
	ASSERT_EQ_D32(ev_tcp_bind(&s_sock, (struct sockaddr*)&addr, addrlen), 0);

	ev_tcp_exit(&s_sock, _on_close_socket);
	ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

	ASSERT_EQ_D32(s_flag_socket_close, 1);
	ev_loop_exit(&s_loop);
}
