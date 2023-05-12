#include "test.h"
#include <string.h>

struct test_223a
{
    ev_queue_node_t     q;
};

struct test_223a        g_test_223a;

TEST_FIXTURE_SETUP(queue)
{
    memset(&g_test_223a.q, 0, sizeof(g_test_223a.q));
    ev_queue_init(&g_test_223a.q);
}

TEST_FIXTURE_TEARDOWN(queue)
{
}

TEST_F(queue, empty)
{
    ASSERT_EQ_INT(ev_queue_empty(&g_test_223a.q), 1);
}

TEST_F(queue, head)
{
    ASSERT_EQ_PTR(ev_queue_head(&g_test_223a.q), NULL);
}

TEST_F(queue, next)
{
    {
        ASSERT_EQ_PTR(ev_queue_next(&g_test_223a.q, &g_test_223a.q), NULL);
    }
    {
        ev_queue_node_t node;
        ev_queue_push_back(&g_test_223a.q, &node);

        ev_queue_node_t* it = ev_queue_head(&g_test_223a.q);
        ASSERT_NE_PTR(it, NULL);

        ASSERT_EQ_PTR(ev_queue_next(&g_test_223a.q, it), NULL);
    }
}

TEST_F(queue, push_front)
{
    ev_queue_node_t node[2];
    ev_queue_push_front(&g_test_223a.q, &node[0]);
    ev_queue_push_front(&g_test_223a.q, &node[1]);

    ASSERT_EQ_PTR(ev_queue_pop_back(&g_test_223a.q), &node[0]);
    ASSERT_EQ_PTR(ev_queue_pop_back(&g_test_223a.q), &node[1]);
    ASSERT_EQ_PTR(ev_queue_pop_back(&g_test_223a.q), NULL);
}
