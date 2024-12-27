#include "ev.h"
#include "test.h"

TEST(tcp, static_initializer)
{
    { ev_write_t ret = EV_WRITE_INVALID;   (void)ret; }
    { ev_read_t ret = EV_READ_INVALID;     (void)ret; }
}
