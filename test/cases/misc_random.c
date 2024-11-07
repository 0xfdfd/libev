#include "test.h"

TEST(misc, random)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));

    ev_random(NULL, NULL, buff, sizeof(buff), 0, NULL);

    size_t i;
    uint64_t sum = 0;
    for (i = 0; i < ARRAY_SIZE(buff); i++)
    {
        sum += buff[i];
    }

    ASSERT_NE_UINT64(sum, 0);
}
