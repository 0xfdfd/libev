#include "test.h"

TEST(misc, system_page)
{
    size_t page_sz = ev_os_page_size();
    ASSERT_EQ_INT(page_sz % 4096, 0, "page_sz:%u", (unsigned)page_sz);
}
