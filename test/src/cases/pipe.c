#include "test.h"

TEST(pipe, make_close)
{
    ev_os_pipe_t pipfd[2];
    ASSERT_EQ_D32(ev_pipe_make(pipfd), EV_SUCCESS);
    ev_pipe_close(pipfd[0]);
    ev_pipe_close(pipfd[1]);
}
