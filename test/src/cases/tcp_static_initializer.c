#include "ev.h"
#include "test.h"

TEST(tcp, static_initializer)
{
    { ev_write_t ret = EV_WRITE_INIT;   (void)ret; }
    { ev_read_t ret = EV_READ_INIT;     (void)ret; }
    { ev_tcp_t ret = EV_TCP_INIT;       (void)ret; }
    { ev_pipe_t ret = EV_PIPE_INIT;     (void)ret; }
}
