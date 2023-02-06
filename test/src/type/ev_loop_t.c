#include "ev.h"
#include "ev_loop_t.h"

typedef struct loop_find_helper
{
	ev_loop_t*		src;
	int				mismatch;
} loop_find_helper_t;

typedef struct loop_find_helper_2
{
	ev_handle_t*	handle;
	int				matched;
} loop_find_helper_2_t;

typedef struct dump_helper
{
	FILE* stream;
	int ret;
} dump_helper_t;

static int _test_loop_find_helper_2(ev_handle_t* handle, void* arg)
{
	loop_find_helper_2_t* helper = arg;

	if (helper->handle == handle)
	{
		helper->matched = 1;
		return 1;
	}

	return 0;
}

static int _test_loop_find_helper(ev_handle_t* handle, void* arg)
{
	loop_find_helper_t* helper = arg;

	loop_find_helper_2_t helper_2 = { handle, 0 };
	ev_loop_walk(helper->src, _test_loop_find_helper_2, &helper_2);

	if (!helper_2.matched)
	{
		helper->mismatch = 1;
		return 1;
	}

	return 0;
}

/**
 * @return 1 if mismatch, 0 if equal
 */
static int _find_all_in_loop(ev_loop_t* src, ev_loop_t* pat)
{
	loop_find_helper_t helper = { src, 0 };
	ev_loop_walk(pat, _test_loop_find_helper, &helper);
	return helper.mismatch;
}

static int _on_cmp_ev_loop_t(const ev_loop_t** addr1, const ev_loop_t** addr2)
{
	ev_loop_t* loop1 = (ev_loop_t*)*addr1;
	ev_loop_t* loop2 = (ev_loop_t*)*addr2;

	if (_find_all_in_loop(loop1, loop2) || _find_all_in_loop(loop2, loop1))
	{
		return 1;
	}

	return 0;
}

static int _test_dump_handle(ev_handle_t* handle, void* arg)
{
	dump_helper_t* helper = arg;
	helper->ret += fprintf(helper->stream, "%p,", handle);
	return 0;
}

static int _on_dump_ev_loop_t(FILE* stream, const ev_loop_t** addr)
{
	int ret = 0;

	ret += fprintf(stream, "{");

	dump_helper_t helper = { stream, 0 };
	ev_loop_walk((ev_loop_t*)*addr, _test_dump_handle, &helper);

	ret += fprintf(stream, "}");

	return ret + helper.ret;
}

void test_register_ev_loop_t(void)
{
	TEST_REGISTER_TYPE_ONCE(ev_loop_t*, _on_cmp_ev_loop_t, _on_dump_ev_loop_t);
}
