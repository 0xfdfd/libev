#include <string.h>
#include "ev.h"
#include "test.h"

static ev_loop_t	s_loop;
static ev_tcp_t		s_socket;
static char			s_buffer[64];

TEST(ipv4_addr)
{
	const char* ip = "127.0.0.1";
	memset(s_buffer, 0, sizeof(s_buffer));

	ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
	ASSERT_EQ_D32(ev_tcp_init(&s_loop, &s_socket), 0);

	struct sockaddr_in addr;
	size_t len = sizeof(addr);

	ASSERT_EQ_D32(ev_ipv4_addr(ip, 0, &addr), 0);
	ASSERT_EQ_D32(ev_tcp_bind(&s_socket, (struct sockaddr*)&addr, sizeof(addr)), 0);

	memset(&addr, 0, sizeof(addr));
	ASSERT_EQ_D32(ev_tcp_getsockname(&s_socket, (struct sockaddr*)&addr, &len), 0);

	ASSERT_EQ_D32(ev_ipv4_name(&addr, NULL, s_buffer, sizeof(s_buffer)), 0);
	ASSERT_EQ_STR(ip, s_buffer);
}
