#include "ev.h"
#include "test.h"
#include "utils/random.h"
#include <string.h>

typedef struct r_pack
{
    ev_read_t   read_req;
    ev_buf_t    buf;
    char        buffer[4096];
}r_pack_t;

typedef struct w_pack
{
    ev_write_t  write_req;
    ev_buf_t    buf;
    char        buffer[1024];
}w_pack_t;

static ev_loop_t    s_loop;
static ev_pipe_t    s_pipe_w;  /**< Write handle */
static ev_pipe_t    s_pipe_r;  /**< Receive handle */
static r_pack_t     s_r_pack;
static w_pack_t     s_w_pack;

static void _on_write_callback(ev_write_t* req, size_t size, int stat)
{
    ASSERT_EQ_PTR(req, &s_w_pack.write_req);
    ASSERT_EQ_U64(size, sizeof(s_w_pack.buffer));
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    ev_pipe_exit(&s_pipe_w, NULL);
}

static void _on_read_callback(ev_read_t* req, size_t size, int stat)
{
    ASSERT_EQ_PTR(req, &s_r_pack.read_req);

    if (stat == EV_EOF)
    {
        return;
    }

    s_r_pack.buf = ev_buf_make((char*)s_r_pack.buf.data + size, s_r_pack.buf.size - size);
    ASSERT_EQ_D32(ev_pipe_read(&s_pipe_r, &s_r_pack.read_req, &s_r_pack.buf, 1, _on_read_callback), 0);
}

TEST(pipe, pipe)
{
    test_random(s_w_pack.buffer, sizeof(s_w_pack.buffer));

    ASSERT_EQ_D32(ev_loop_init(&s_loop), 0);
    ASSERT_EQ_D32(ev_pipe_init(&s_loop, &s_pipe_r), 0);
    ASSERT_EQ_D32(ev_pipe_init(&s_loop, &s_pipe_w), 0);

    ev_os_handle_t fds[2];
    ASSERT_EQ_D32(ev_pipe_make(fds), 0);
    ASSERT_EQ_D32(ev_pipe_open(&s_pipe_r, fds[0]), 0);
    ASSERT_EQ_D32(ev_pipe_open(&s_pipe_w, fds[1]), 0);

    s_w_pack.buf = ev_buf_make(s_w_pack.buffer, sizeof(s_w_pack.buffer));
    ASSERT_EQ_D32(ev_pipe_write(&s_pipe_w, &s_w_pack.write_req, &s_w_pack.buf, 1, _on_write_callback), 0);

    s_r_pack.buf = ev_buf_make(s_r_pack.buffer, sizeof(s_r_pack.buffer));
    ASSERT_EQ_D32(ev_pipe_read(&s_pipe_r, &s_r_pack.read_req, &s_r_pack.buf, 1, _on_read_callback), 0);

    ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);
    {
        int ret = memcmp(s_w_pack.buffer, s_r_pack.buffer, sizeof(s_w_pack.buffer));
        ASSERT_EQ_D32(ret, 0);
    }

    ev_pipe_exit(&s_pipe_r, NULL);
    ASSERT_EQ_D32(ev_loop_run(&s_loop, ev_loop_mode_default), 0);

    ev_loop_exit(&s_loop);
}
