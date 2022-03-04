#include "ev.h"
#include "test.h"

TEST(list, migrate)
{
    ev_list_t src = EV_LIST_INIT;
    ev_list_t dst = EV_LIST_INIT;
    ev_list_node_t node[2];

    ev_list_push_back(&dst, &node[0]);
    ev_list_push_back(&src, &node[0]);

    ev_list_migrate(&dst, &src);

    ASSERT_EQ_D32(ev_list_size(&dst), 2);
    ASSERT_EQ_D32(ev_list_size(&src), 0);
}

TEST(list, migrate_empty)
{
    ev_list_t src = EV_LIST_INIT;
    ev_list_t dst = EV_LIST_INIT;

    ev_list_migrate(&dst, &src);

    ASSERT_EQ_D32(ev_list_size(&dst), 0);
    ASSERT_EQ_D32(ev_list_size(&src), 0);
}

TEST(list, migrate_empty_src)
{
    ev_list_t src = EV_LIST_INIT;
    ev_list_t dst = EV_LIST_INIT;
    ev_list_node_t node;

    ev_list_push_back(&dst, &node);
    ev_list_migrate(&dst, &src);

    ASSERT_EQ_D32(ev_list_size(&dst), 1);
    ASSERT_EQ_D32(ev_list_size(&src), 0);
}

TEST(list, migrate_empty_dst)
{
    ev_list_t src = EV_LIST_INIT;
    ev_list_t dst = EV_LIST_INIT;
    ev_list_node_t node;

    ev_list_push_back(&src, &node);
    ev_list_migrate(&dst, &src);

    ASSERT_EQ_D32(ev_list_size(&dst), 1);
    ASSERT_EQ_D32(ev_list_size(&src), 0);
}
