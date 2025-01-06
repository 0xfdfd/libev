#include "ev.h"
#include "test.h"
#include <string.h>

#if defined(__linux__)
#include <sys/mman.h>
#endif

#define TEST_SHMEM_KEY "/e35955c2"
#define TEST_SHMEM_SIZE (4096)

struct test_3b5b
{
    ev_shmem_t *smt_token_1;
    ev_shmem_t *smt_token_2;
};

struct test_3b5b *g_test_3b5b = NULL;

TEST_FIXTURE_SETUP(shm)
{
    g_test_3b5b = ev_calloc(1, sizeof(*g_test_3b5b));

    ASSERT_EQ_INT(ev_shmem_init(&g_test_3b5b->smt_token_1, TEST_SHMEM_KEY,
                                TEST_SHMEM_SIZE),
                  0, "%s(%d)", ev_strerror(_L), _L);
    ASSERT_EQ_INT(ev_shmem_open(&g_test_3b5b->smt_token_2, TEST_SHMEM_KEY), 0);

    ASSERT_EQ_SIZE(ev_shmem_size(g_test_3b5b->smt_token_1), TEST_SHMEM_SIZE);
    ASSERT_EQ_SIZE(ev_shmem_size(g_test_3b5b->smt_token_2), TEST_SHMEM_SIZE);
}

TEST_FIXTURE_TEARDOWN(shm)
{
    ev_shmem_exit(g_test_3b5b->smt_token_1);
    ev_shmem_exit(g_test_3b5b->smt_token_2);

    ev_free(g_test_3b5b);
    g_test_3b5b = NULL;

#if defined(__linux__)
    shm_unlink(TEST_SHMEM_KEY);
#endif
}

TEST_F(shm, memset)
{
    memset(ev_shmem_addr(g_test_3b5b->smt_token_1), 0,
           ev_shmem_size(g_test_3b5b->smt_token_1));
    memset(ev_shmem_addr(g_test_3b5b->smt_token_2), 0,
           ev_shmem_size(g_test_3b5b->smt_token_2));
}
