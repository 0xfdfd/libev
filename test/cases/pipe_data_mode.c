#include "ev.h"
#include "test.h"
#include "utils/random.h"
#include <string.h>

#define TEST_PACK_NUM_221D 4
#define TEST_BUFFER_SIZE_221D (8 * 1024 * 1024)

typedef struct test_pipe_data_mode
{
    ev_loop_t *loop;

    ev_pipe_t *pipe_w; /**< Write handle */
    ev_pipe_t *pipe_r; /**< Receive handle */

    struct
    {
        ev_pipe_read_req_t read_req;
        ev_buf_t           buf;
        uint8_t buffer[TEST_BUFFER_SIZE_221D * (TEST_PACK_NUM_221D + 1)];
    } r_pack;

    struct
    {
        ev_buf_t buf;
        uint8_t  buffer[TEST_BUFFER_SIZE_221D];
    } w_pack[TEST_PACK_NUM_221D];
} test_pipe_data_mode_t;

test_pipe_data_mode_t *g_test_pipe_data = NULL;

static void _on_write_callback_221d(ev_pipe_t *pipe, ssize_t size, void *arg)
{
    (void)pipe;
    ASSERT_EQ_SSIZE(size, TEST_BUFFER_SIZE_221D);

    if (arg == (void *)&g_test_pipe_data->w_pack[TEST_PACK_NUM_221D - 1])
    {
        ev_pipe_exit(g_test_pipe_data->pipe_w, NULL, NULL);
    }
}

static void _on_read_callback_221d(ev_pipe_read_req_t *req, ssize_t size)
{
    ASSERT_EQ_PTR(req, &g_test_pipe_data->r_pack.read_req);

    if (size == EV_EOF)
    {
        return;
    }
    ASSERT_GE_SSIZE(size, 0);

    g_test_pipe_data->r_pack.buf =
        ev_buf_make((char *)g_test_pipe_data->r_pack.buf.data + size,
                    g_test_pipe_data->r_pack.buf.size - size);
    ASSERT_EQ_INT(ev_pipe_read(g_test_pipe_data->pipe_r,
                               &g_test_pipe_data->r_pack.read_req,
                               &g_test_pipe_data->r_pack.buf, 1,
                               _on_read_callback_221d),
                  0);
}

TEST_FIXTURE_SETUP(pipe)
{
    size_t i;
    g_test_pipe_data = ev_calloc(1, sizeof(*g_test_pipe_data));

    for (i = 0; i < TEST_PACK_NUM_221D; i++)
    {
        test_random(g_test_pipe_data->w_pack[i].buffer,
                    sizeof(g_test_pipe_data->w_pack[i].buffer));
    }

    ASSERT_EQ_INT(ev_loop_init(&g_test_pipe_data->loop), 0);
    ASSERT_EQ_INT(
        ev_pipe_init(g_test_pipe_data->loop, &g_test_pipe_data->pipe_r, 0), 0);
    ASSERT_EQ_INT(
        ev_pipe_init(g_test_pipe_data->loop, &g_test_pipe_data->pipe_w, 0), 0);

    int rwflags = EV_PIPE_READABLE | EV_PIPE_WRITABLE | EV_PIPE_NONBLOCK;
    ev_os_pipe_t fds[2];
    ASSERT_EQ_INT(ev_pipe_make(fds, rwflags, rwflags), 0);
    ASSERT_EQ_INT(ev_pipe_open(g_test_pipe_data->pipe_r, fds[0]), 0);
    ASSERT_EQ_INT(ev_pipe_open(g_test_pipe_data->pipe_w, fds[1]), 0);
}

TEST_FIXTURE_TEARDOWN(pipe)
{
    ev_pipe_exit(g_test_pipe_data->pipe_r, NULL, NULL);
    ASSERT_EQ_INT(ev_loop_run(g_test_pipe_data->loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);

    ev_loop_exit(g_test_pipe_data->loop);

    ev_free(g_test_pipe_data);
    g_test_pipe_data = NULL;
}

TEST_F(pipe, data_mode)
{
    size_t i;
    for (i = 0; i < TEST_PACK_NUM_221D; i++)
    {
        g_test_pipe_data->w_pack[i].buf =
            ev_buf_make(g_test_pipe_data->w_pack[i].buffer,
                        sizeof(g_test_pipe_data->w_pack[i].buffer));

        ASSERT_EQ_INT(ev_pipe_write(g_test_pipe_data->pipe_w,
                                    &g_test_pipe_data->w_pack[i].buf, 1,
                                    _on_write_callback_221d,
                                    &g_test_pipe_data->w_pack[i]),
                      0);
    }

    g_test_pipe_data->r_pack.buf =
        ev_buf_make(g_test_pipe_data->r_pack.buffer,
                    sizeof(g_test_pipe_data->r_pack.buffer));
    ASSERT_EQ_INT(ev_pipe_read(g_test_pipe_data->pipe_r,
                               &g_test_pipe_data->r_pack.read_req,
                               &g_test_pipe_data->r_pack.buf, 1,
                               _on_read_callback_221d),
                  0);

    ASSERT_EQ_INT(ev_loop_run(g_test_pipe_data->loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    for (i = 0; i < TEST_PACK_NUM_221D; i++)
    {
        void *buf1 = g_test_pipe_data->w_pack[i].buffer;
        void *buf2 =
            (uint8_t *)g_test_pipe_data->r_pack.buffer + TEST_BUFFER_SIZE_221D;
        int ret =
            memcmp(buf1, buf2, sizeof(g_test_pipe_data->w_pack[i].buffer));
        ASSERT_EQ_INT(ret, 0);
    }
}
