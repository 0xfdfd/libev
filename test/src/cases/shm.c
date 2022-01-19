#include "ev.h"
#include "test.h"
#include <string.h>

struct test_3b5b
{
    ev_shm_t smt_token_1;
    ev_shm_t smt_token_2;
};

struct test_3b5b g_test_3b5b;

TEST_FIXTURE_SETUP(shm)
{
    const char* key = "/test";
    const size_t mem_size = 4096;

    memset(&g_test_3b5b, 0, sizeof(g_test_3b5b));

    ASSERT_EQ_D32(ev_shm_init(&g_test_3b5b.smt_token_1, key, mem_size), 0);
    ASSERT_EQ_D32(ev_shm_open(&g_test_3b5b.smt_token_2, key), 0);

    ASSERT_EQ_SIZE(g_test_3b5b.smt_token_1.size, mem_size);
    ASSERT_EQ_SIZE(g_test_3b5b.smt_token_2.size, mem_size);
}

TEST_FIXTURE_TEAREDOWN(shm)
{
    ev_shm_exit(&g_test_3b5b.smt_token_1);
    ev_shm_exit(&g_test_3b5b.smt_token_2);
}

TEST_F(shm, memset)
{
    memset(g_test_3b5b.smt_token_1.addr, 0, g_test_3b5b.smt_token_1.size);
    memset(g_test_3b5b.smt_token_2.addr, 0, g_test_3b5b.smt_token_2.size);
}
