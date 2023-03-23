#include "test.h"
#include <string.h>

struct test_757a
{
    ev_loop_t               loop;           /**< Event loop */
    ev_threadpool_t         pool;           /**< Thread pool */
    ev_os_thread_t          threads[1];     /**< Thread storage */

    ev_threadpool_work_t    token;          /**< Work token */
    ev_os_tid_t             thread_id;      /**< Thread ID */
    int                     cnt_work;       /**< Work counter */
    int                     cnt_done;       /**< Done counter */
};

struct test_757a            g_test_757a;

TEST_FIXTURE_SETUP(threadpool)
{
    memset(&g_test_757a, 0, sizeof(g_test_757a));
    g_test_757a.thread_id = ev_thread_id();

    ASSERT_EQ_INT(ev_loop_init(&g_test_757a.loop), 0);
    ASSERT_EQ_INT(ev_threadpool_init(&g_test_757a.pool, NULL, g_test_757a.threads, ARRAY_SIZE(g_test_757a.threads)), 0);
    ASSERT_EQ_INT(ev_loop_link_threadpool(&g_test_757a.loop, &g_test_757a.pool), 0);
}

TEST_FIXTURE_TEARDOWN(threadpool)
{
    ASSERT_EQ_INT(ev_loop_run(&g_test_757a.loop, EV_LOOP_MODE_ONCE), 0);
    ASSERT_EQ_EVLOOP(&g_test_757a.loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_757a.loop), 0);
    ev_threadpool_exit(&g_test_757a.pool);
}

static void _test_threadpool_on_work(ev_threadpool_work_t* work)
{
    ASSERT_EQ_PTR(work, &g_test_757a.token);
    g_test_757a.cnt_work++;

    ev_os_tid_t curr_thread = ev_thread_id();
    ASSERT_NE_ULONG(curr_thread, g_test_757a.thread_id);
}

static void _test_threadpool_on_work_done(ev_threadpool_work_t* work, int status)
{
    ev_os_tid_t curr_thread = ev_thread_id();
    ASSERT_EQ_ULONG(curr_thread, g_test_757a.thread_id);
    ASSERT_EQ_INT(status, EV_SUCCESS);
    ASSERT_EQ_PTR(work, &g_test_757a.token);
    g_test_757a.cnt_done++;
}

TEST_F(threadpool, normal)
{
    ASSERT_EQ_INT(g_test_757a.cnt_work, 0);

    {
        int submit_ret = ev_loop_queue_work(&g_test_757a.loop, &g_test_757a.token,
            _test_threadpool_on_work, _test_threadpool_on_work_done);
        ASSERT_EQ_INT(submit_ret, 0);
    }

    ASSERT_EQ_INT(g_test_757a.cnt_done, 0);
    ASSERT_EQ_INT(ev_loop_run(&g_test_757a.loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_INT(g_test_757a.cnt_work, 1);
    ASSERT_EQ_INT(g_test_757a.cnt_done, 1);
}
