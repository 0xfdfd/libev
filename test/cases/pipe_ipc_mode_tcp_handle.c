#include "ev.h"
#include "test.h"
#include "utils/sockpair.h"
#include "utils/random.h"
#include <string.h>

struct test_19f1
{
    ev_loop_t *loop; /**< Runtime loop */

    ev_pipe_t *s_pipe; /**< Server pipe */
    ev_pipe_t *c_pipe; /**< Client pipe */

    ev_tcp_t *s_tcp; /**< Session, will be transfer to peer */
    ev_tcp_t *c_tcp; /**< Client */
    ev_tcp_t *d_tcp; /**< Peer socket to receive handle */

    struct
    {
        ev_tcp_read_req_t  r_req;
        ev_tcp_write_req_t w_req;
    } tcp;

    struct
    {
        ev_pipe_read_req_t r_req; /**< Read request */
    } pipe;

    unsigned cnt_wcb;
    unsigned cnt_rcb;

    char data1[1024];
    char data2[1024];
};

struct test_19f1 *g_test_19f1 = NULL;

TEST_FIXTURE_SETUP(pipe)
{
    g_test_19f1 = ev_calloc(1, sizeof(*g_test_19f1));

    test_random(g_test_19f1->data1, sizeof(g_test_19f1->data1));
    ASSERT_EQ_INT(ev_loop_init(&g_test_19f1->loop), 0);

    ASSERT_EQ_INT(ev_pipe_init(g_test_19f1->loop, &g_test_19f1->s_pipe, 1), 0);
    ASSERT_EQ_INT(ev_pipe_init(g_test_19f1->loop, &g_test_19f1->c_pipe, 1), 0);

    int          rwflags = EV_PIPE_NONBLOCK | EV_PIPE_IPC;
    ev_os_pipe_t fds[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };
    ASSERT_EQ_INT(ev_pipe_make(fds, rwflags, rwflags), 0);

    ASSERT_EQ_INT(ev_pipe_open(g_test_19f1->s_pipe, fds[0]), 0);
    ASSERT_EQ_INT(ev_pipe_open(g_test_19f1->c_pipe, fds[1]), 0);

    ASSERT_EQ_INT(ev_tcp_init(g_test_19f1->loop, &g_test_19f1->s_tcp), 0);
    ASSERT_EQ_INT(ev_tcp_init(g_test_19f1->loop, &g_test_19f1->c_tcp), 0);
    test_sockpair(g_test_19f1->loop, g_test_19f1->s_tcp, g_test_19f1->c_tcp);
    ASSERT_EQ_INT(ev_tcp_init(g_test_19f1->loop, &g_test_19f1->d_tcp), 0);
}

TEST_FIXTURE_TEARDOWN(pipe)
{
    ev_pipe_exit(g_test_19f1->s_pipe, NULL, NULL);
    ev_pipe_exit(g_test_19f1->c_pipe, NULL, NULL);
    ev_tcp_exit(g_test_19f1->s_tcp, NULL, NULL);
    ev_tcp_exit(g_test_19f1->c_tcp, NULL, NULL);
    ev_tcp_exit(g_test_19f1->d_tcp, NULL, NULL);
    ASSERT_EQ_INT(ev_loop_run(g_test_19f1->loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    ASSERT_EQ_INT(ev_loop_exit(g_test_19f1->loop), 0);

    ev_free(g_test_19f1);
    g_test_19f1 = NULL;
}

static void _on_pipe_write_done_19f1(ev_pipe_t *pipe, ssize_t size, void *arg)
{
    (void)pipe;
    (void)arg;
    ASSERT_EQ_SSIZE(size, sizeof(g_test_19f1->data1));

    g_test_19f1->cnt_wcb++;
}

static void _on_tcp_write_done_19f1(ev_tcp_write_req_t *req, ssize_t size)
{
    ASSERT_EQ_PTR(req, &g_test_19f1->tcp.w_req);
    ASSERT_EQ_SSIZE(size, sizeof(g_test_19f1->data1));

    g_test_19f1->cnt_wcb++;
}

static void _on_pipe_read_done_19f1(ev_pipe_read_req_t *req, ssize_t size)
{
    (void)size;
    ASSERT_EQ_PTR(req, &g_test_19f1->pipe.r_req);

    g_test_19f1->cnt_rcb++;
}

static void _on_tcp_read_done_19f1(ev_tcp_read_req_t *req, ssize_t size)
{
    ASSERT_EQ_PTR(req, &g_test_19f1->tcp.r_req);
    ASSERT_GE_SSIZE(size, 0);

    g_test_19f1->cnt_rcb++;
}

TEST_F(pipe, ipc_mode_tcp_handle)
{
    ev_buf_t buf;

    /* send data and handle */
    buf = ev_buf_make(g_test_19f1->data1, sizeof(g_test_19f1->data1));
    ASSERT_EQ_INT(ev_pipe_write_ex(g_test_19f1->s_pipe, &buf, 1, EV_ROLE_EV_TCP,
                                   g_test_19f1->s_tcp, _on_pipe_write_done_19f1,
                                   NULL),
                  0);

    /* recv data and handle */
    buf = ev_buf_make(g_test_19f1->data2, sizeof(g_test_19f1->data2));
    ASSERT_EQ_INT(ev_pipe_read(g_test_19f1->c_pipe, &g_test_19f1->pipe.r_req,
                               &buf, 1, _on_pipe_read_done_19f1),
                  0);

    /* communicate */
    ASSERT_EQ_INT(ev_loop_run(g_test_19f1->loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    ASSERT_EQ_UINT(g_test_19f1->cnt_wcb, 1);
    ASSERT_EQ_UINT(g_test_19f1->cnt_rcb, 1);

    /* accept handle */
    ASSERT_EQ_INT(ev_pipe_accept(g_test_19f1->c_pipe, &g_test_19f1->pipe.r_req,
                                 EV_ROLE_EV_TCP, g_test_19f1->d_tcp),
                  0);

    /* now we are able to send data via d_tcp */
    buf = ev_buf_make(g_test_19f1->data1, sizeof(g_test_19f1->data1));
    ASSERT_EQ_INT(ev_tcp_write(g_test_19f1->d_tcp, &g_test_19f1->tcp.w_req,
                               &buf, 1, _on_tcp_write_done_19f1),
                  0);

    buf = ev_buf_make(g_test_19f1->data2, sizeof(g_test_19f1->data2));
    ASSERT_EQ_INT(ev_tcp_read(g_test_19f1->c_tcp, &g_test_19f1->tcp.r_req, &buf,
                              1, _on_tcp_read_done_19f1),
                  0);

    /* communicate */
    ASSERT_EQ_INT(ev_loop_run(g_test_19f1->loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
    ASSERT_EQ_UINT(g_test_19f1->cnt_wcb, 2);
    ASSERT_EQ_UINT(g_test_19f1->cnt_rcb, 2);
}
