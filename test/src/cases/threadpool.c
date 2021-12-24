#include "test.h"
#include <string.h>

struct test_757a
{
    ev_loop_t               loop;
    ev_threadpool_t         pool;
    ev_os_thread_t          threads[4];

    ev_threadpool_work_t    token;
    ev_os_thread_t          thread_info;
};

struct test_757a            g_test_757a;

TEST_FIXTURE_SETUP(threadpool)
{
    memset(&g_test_757a, 0, sizeof(g_test_757a));
    g_test_757a.thread_info = ev_thread_self();

    ASSERT_EQ_D32(ev_loop_init(&g_test_757a.loop), 0);
    ASSERT_EQ_D32(ev_threadpool_init(&g_test_757a.pool, NULL, g_test_757a.threads, ARRAY_SIZE(g_test_757a.threads)), 0);
}

TEST_FIXTURE_TEAREDOWN(threadpool)
{
    ev_threadpool_exit(&g_test_757a.pool);
    ASSERT_EQ_D32(ev_loop_run(&g_test_757a.loop, EV_LOOP_MODE_ONCE), 0);
    ASSERT_EQ_D32(ev_loop_exit(&g_test_757a.loop), 0);
}

static void _test_threadpool_on_work(ev_threadpool_work_t* work)
{
    ASSERT_EQ_PTR(work, &g_test_757a.token);
}

static void _test_threadpool_on_work_done(ev_threadpool_work_t* work, int status)
{
    ev_os_thread_t curr_thread = ev_thread_self();
    ASSERT_EQ_D32(ev_thread_equal(&curr_thread, &g_test_757a.thread_info), 0);
    ASSERT_EQ_D32(status, EV_SUCCESS);
    ASSERT_EQ_PTR(work, &g_test_757a.token);
}

TEST_F(threadpool, normal)
{
    int submit_ret = ev_threadpool_submit(&g_test_757a.pool, &g_test_757a.loop,
        &g_test_757a.token, EV_THREADPOOL_WORK_CPU,
        _test_threadpool_on_work, _test_threadpool_on_work_done);
    ASSERT_EQ_D32(submit_ret, 0);

    ASSERT_EQ_D32(ev_loop_run(&g_test_757a.loop, EV_LOOP_MODE_DEFAULT), 0);
}
