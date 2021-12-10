#include "ev.h"
#include "test.h"
#include <string.h>

TEST(shm, init)
{
    ev_shm_t smt_token;
    ASSERT_EQ_D32(ev_shm_init(&smt_token, "/test", 4096), 0);

    memset(smt_token.addr, 0, smt_token.size);
    ev_shm_exit(&smt_token);
}
