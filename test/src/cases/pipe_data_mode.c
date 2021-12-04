#include "ev.h"
#include "test.h"
#include "utils/random.h"
#include <string.h>

typedef struct r_pack
{
    ev_read_t   read_req;
    ev_buf_t    buf;
    uint8_t     buffer[33 * 1024 * 1024];
}r_pack_t;

typedef struct w_pack
{
    ev_write_t  write_req;
    ev_buf_t    buf;
    uint8_t     buffer[32 * 1024 * 1024];
}w_pack_t;

struct test_221d
{
    ev_loop_t   loop;

    ev_pipe_t   pipe_w;  /**< Write handle */
    ev_pipe_t   pipe_r;  /**< Receive handle */

    r_pack_t    r_pack;
    w_pack_t    w_pack;
};

static struct test_221d g_test_221d;

static void _on_write_callback(ev_write_t* req, size_t size, int stat)
{
    ASSERT_EQ_PTR(req, &g_test_221d.w_pack.write_req);
    ASSERT_EQ_U64(size, sizeof(g_test_221d.w_pack.buffer));
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    ev_pipe_exit(&g_test_221d.pipe_w, NULL);
}

static void _on_read_callback(ev_read_t* req, size_t size, int stat)
{
    ASSERT_EQ_PTR(req, &g_test_221d.r_pack.read_req);

    if (stat == EV_EOF)
    {
        return;
    }
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    g_test_221d.r_pack.buf = ev_buf_make((char*)g_test_221d.r_pack.buf.data + size, g_test_221d.r_pack.buf.size - size);
    ASSERT_EQ_D32(ev_read_init(&g_test_221d.r_pack.read_req, &g_test_221d.r_pack.buf, 1, _on_read_callback), 0);
    ASSERT_EQ_D32(ev_pipe_read(&g_test_221d.pipe_r, &g_test_221d.r_pack.read_req), 0);
}

TEST_FIXTURE_SETUP(pipe)
{
    test_random(g_test_221d.w_pack.buffer, sizeof(g_test_221d.w_pack.buffer));

    ASSERT_EQ_D32(ev_loop_init(&g_test_221d.loop), 0);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_221d.loop, &g_test_221d.pipe_r, 0), 0);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_221d.loop, &g_test_221d.pipe_w, 0), 0);

    ev_os_pipe_t fds[2];
    ASSERT_EQ_D32(ev_pipe_make(fds), 0);
    ASSERT_EQ_D32(ev_pipe_open(&g_test_221d.pipe_r, fds[0]), 0);
    ASSERT_EQ_D32(ev_pipe_open(&g_test_221d.pipe_w, fds[1]), 0);
}

TEST_FIXTURE_TEAREDOWN(pipe)
{
    ev_pipe_exit(&g_test_221d.pipe_r, NULL);
    ASSERT_EQ_D32(ev_loop_run(&g_test_221d.loop, EV_LOOP_MODE_DEFAULT), 0);

    ev_loop_exit(&g_test_221d.loop);
}

TEST_F(pipe, data_mode)
{
    g_test_221d.w_pack.buf = ev_buf_make(g_test_221d.w_pack.buffer, sizeof(g_test_221d.w_pack.buffer));
    ASSERT_EQ_D32(ev_write_init(&g_test_221d.w_pack.write_req, &g_test_221d.w_pack.buf, 1, _on_write_callback), 0);
    ASSERT_EQ_D32(ev_pipe_write(&g_test_221d.pipe_w, &g_test_221d.w_pack.write_req), 0);

    g_test_221d.r_pack.buf = ev_buf_make(g_test_221d.r_pack.buffer, sizeof(g_test_221d.r_pack.buffer));
    ASSERT_EQ_D32(ev_read_init(&g_test_221d.r_pack.read_req, &g_test_221d.r_pack.buf, 1, _on_read_callback), 0);
    ASSERT_EQ_D32(ev_pipe_read(&g_test_221d.pipe_r, &g_test_221d.r_pack.read_req), 0);

    ASSERT_EQ_D32(ev_loop_run(&g_test_221d.loop, EV_LOOP_MODE_DEFAULT), 0);
    {
        int ret = memcmp(g_test_221d.w_pack.buffer, g_test_221d.r_pack.buffer, sizeof(g_test_221d.w_pack.buffer));
        ASSERT_EQ_D32(ret, 0);
    }
}
