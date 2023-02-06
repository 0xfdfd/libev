#include "ev.h"
#include "ssize_t.h"
#include "cutest.h"

static int _on_cmp_ssize_t(const ssize_t* addr1, const ssize_t* addr2)
{
	ssize_t v1 = *addr1, v2 = *addr2;
	if (v1 == v2)
	{
		return 0;
	}
	return v1 < v2 ? -1 : 1;
}

static int _on_dump_ssize_t(FILE* stream, const ssize_t* addr)
{
	return fprintf(stream, "%" PRId64, (int64_t)*addr);
}

void test_register_ssize_t(void)
{
	TEST_REGISTER_TYPE_ONCE(ssize_t, _on_cmp_ssize_t, _on_dump_ssize_t);
}
