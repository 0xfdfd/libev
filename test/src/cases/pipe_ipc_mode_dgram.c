#include "ev.h"
#include "test.h"
#include "utils/hash.h"
#include "utils/random.h"
#include <string.h>
#include <stdlib.h>

#define TEST_PACK_NUM_A548  8

struct wdata_pack_a548
{
    ev_write_t              req;
    struct
    {
        size_t              size;
        uint64_t            hash;
    }data1;
    void*                   data2;
    size_t                  data2_size; /**< 32MB */
};

struct rdata_pack_a548
{
    ev_read_t               req;
    struct
    {
        size_t              size;
        uint64_t            hash;
    }data1;
    void*                   data2;
    size_t                  data2_size; /**< 64MB */
};

struct test_a548
{
    ev_loop_t               loop;
    ev_pipe_t               s_pipe;
    ev_pipe_t               c_pipe;

    struct wdata_pack_a548  w_req[TEST_PACK_NUM_A548];
    struct rdata_pack_a548  r_req[TEST_PACK_NUM_A548];

    size_t                  w_req_cnt;
    size_t                  r_req_cnt;
};

static struct test_a548 g_test_a548;

TEST_FIXTURE_SETUP(pipe)
{
    memset(&g_test_a548, 0, sizeof(g_test_a548));

    ASSERT_EQ_D32(ev_loop_init(&g_test_a548.loop), 0);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_a548.loop, &g_test_a548.s_pipe, 1), 0);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_a548.loop, &g_test_a548.c_pipe, 1), 0);

    ev_os_pipe_t fds[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };
    ASSERT_EQ_D32(ev_pipe_make(fds), 0);

    ASSERT_EQ_D32(ev_pipe_open(&g_test_a548.s_pipe, fds[0]), 0);
    ASSERT_EQ_D32(ev_pipe_open(&g_test_a548.c_pipe, fds[1]), 0);

    size_t i;
    for (i = 0; i < TEST_PACK_NUM_A548; i++)
    {
        g_test_a548.w_req[i].data2_size = 32 * 1024 * 1024;
        g_test_a548.w_req[i].data2 = malloc(g_test_a548.w_req[i].data2_size);
        ASSERT_NE_PTR(g_test_a548.w_req[i].data2, NULL);
        test_random(g_test_a548.w_req[i].data2, g_test_a548.w_req[i].data2_size);
        g_test_a548.w_req[i].data1.size = g_test_a548.w_req[i].data2_size;
        g_test_a548.w_req[i].data1.hash = test_hash64(g_test_a548.w_req[i].data2, g_test_a548.w_req[i].data2_size, 0);

        g_test_a548.r_req[i].data2_size = 64 * 1024 * 1024;
        g_test_a548.r_req[i].data2 = malloc(g_test_a548.r_req[i].data2_size);
        ASSERT_NE_PTR(g_test_a548.r_req[i].data2, NULL);
    }
}

TEST_FIXTURE_TEAREDOWN(pipe)
{
    ev_pipe_exit(&g_test_a548.s_pipe, NULL);
    ev_pipe_exit(&g_test_a548.c_pipe, NULL);

    ASSERT_EQ_D32(ev_loop_run(&g_test_a548.loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_D32(ev_loop_exit(&g_test_a548.loop), 0);

    size_t i;
    for (i = 0; i < TEST_PACK_NUM_A548; i++)
    {
        free(g_test_a548.r_req[i].data2);
        free(g_test_a548.w_req[i].data2);
    }
}

static void _on_test_a548_write_done(ev_write_t* req, size_t size, int stat)
{
    g_test_a548.w_req_cnt++;

    ASSERT_EQ_D32(stat, EV_SUCCESS);
    struct wdata_pack_a548* w_pack = container_of(req, struct wdata_pack_a548, req);

    ASSERT_EQ_SIZE(size, w_pack->data2_size + sizeof(w_pack->data1));
}

static void _on_test_a548_read_done(ev_read_t* req, size_t size, int stat)
{
    g_test_a548.r_req_cnt++;

    ASSERT_EQ_D32(stat, EV_SUCCESS);
    struct rdata_pack_a548* r_pack = container_of(req, struct rdata_pack_a548, req);

    ASSERT_GT_SIZE(size, sizeof(r_pack->data1));
    size_t body_size = size - sizeof(r_pack->data1);
    ASSERT_EQ_SIZE(r_pack->data1.size, body_size);
    ASSERT_EQ_U64(r_pack->data1.hash, test_hash64(r_pack->data2, body_size, 0));
}

TEST_F(pipe, ipc_mode_dgram)
{
    size_t i;
    for (i = 0; i < TEST_PACK_NUM_A548; i++)
    {
        ev_buf_t bufs[2];

        bufs[0] = ev_buf_make(&g_test_a548.w_req[i].data1, sizeof(g_test_a548.w_req[i].data1));
        bufs[1] = ev_buf_make(g_test_a548.w_req[i].data2, g_test_a548.w_req[i].data2_size);
        ASSERT_EQ_D32(ev_write_init(&g_test_a548.w_req[i].req, bufs, 2, _on_test_a548_write_done), 0);

        bufs[0] = ev_buf_make(&g_test_a548.r_req[i].data1, sizeof(g_test_a548.r_req[i].data1));
        bufs[1] = ev_buf_make(g_test_a548.r_req[i].data2, g_test_a548.r_req[i].data2_size);
        ASSERT_EQ_D32(ev_read_init(&g_test_a548.r_req[i].req, bufs, 2, _on_test_a548_read_done), 0);
    }

    for (i = 0; i < TEST_PACK_NUM_A548; i++)
    {
        ASSERT_EQ_D32(ev_pipe_write(&g_test_a548.c_pipe, &g_test_a548.w_req[i].req), 0);
    }
    for (i = 0; i < TEST_PACK_NUM_A548; i++)
    {
        ASSERT_EQ_D32(ev_pipe_read(&g_test_a548.s_pipe, &g_test_a548.r_req[i].req), 0);
    }

    ASSERT_EQ_D32(ev_loop_run(&g_test_a548.loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_SIZE(g_test_a548.r_req_cnt, TEST_PACK_NUM_A548);
    ASSERT_EQ_SIZE(g_test_a548.w_req_cnt, TEST_PACK_NUM_A548);
}
