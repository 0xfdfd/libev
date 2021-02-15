#include "ev.h"
#include "test.h"

TEST(buf)
{
	ev_buf_t b1 = ev_buf_make(NULL, 0);
	{
		ASSERT_EQ_PTR(b1.data, NULL);
		ASSERT_EQ_U64(b1.size, 0);
	}

	ev_buf_t b2[2];
	{
		ev_buf_make_n(b2, 2, NULL, 1, &b1, 2);
		ASSERT_EQ_PTR(b2[0].data, NULL);
		ASSERT_EQ_U64(b2[0].size, 1);
		ASSERT_EQ_PTR(b2[1].data, &b1);
		ASSERT_EQ_U64(b2[1].size, 2);
	}
}
