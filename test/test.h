#ifndef __TEST_H__
#define __TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#	include <windows.h>
#	define ABORT()					DebugBreak()
#else
#	include <stdlib.h>
#	define ABORT()					abort()
#	define _CrtDumpMemoryLeaks()	0
#	define _CrtSetReportMode(...)	(void)0
#	define _CrtSetReportFile(...)	(void)0
#endif

#include <stdint.h>
#include <stdio.h>

#define TEST_EXPAND(x)				x
#define TEST_JOIN(a, b)				TEST_JOIN2(a, b)
#define TEST_JOIN2(a, b)			a##b

#ifdef _MSC_VER // Microsoft compilers
#	define TEST_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#	define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#	define INTERNAL_EXPAND(x) x
#	define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#	define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
#else // Non-Microsoft compilers
#	define TEST_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#	define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
#endif

#define ASSERT(x)	\
	if(!(x)) {\
		fprintf(stderr, "Assertion failed in %s on line %d: %s\n", \
				__FILE__, __LINE__, #x);\
		ABORT();\
	}

#define ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, u_fmt, ...)	\
	do {\
		TYPE _l = (TYPE)(a); TYPE _r = (TYPE)(b);\
		if (CMP(_l, _r)) {\
			break;\
		}\
		printf("%s:%d:failure:" u_fmt "\n"\
			"            expected:    `%s' %s `%s'\n"\
			"              actual:    " FMT " vs " FMT "\n",\
			__FILE__, __LINE__, ##__VA_ARGS__, #a, #OP, #b, _l, _r);\
		fflush(NULL);\
		ABORT();\
	} while(0)

#define ASSERT_EQ_STR(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(const char*, "%s", ==, !strcmp, a, b, ##__VA_ARGS__)
#define ASSERT_NE_STR(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(const char*, "%s", !=,  strcmp, a, b, ##__VA_ARGS__)

#define ASSERT_EQ_PTR(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(const void*, "%p", ==, _ASSERT_INTERNAL_HELPER_EQ, a, b, ##__VA_ARGS__)
#define ASSERT_NE_PTR(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(const void*, "%p", !=, _ASSERT_INTERNAL_HELPER_NE, a, b, ##__VA_ARGS__)

#define ASSERT_EQ_D32(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(int32_t, "%"PRId32, ==, _ASSERT_INTERNAL_HELPER_EQ, a, b, ##__VA_ARGS__)
#define ASSERT_NE_D32(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(int32_t, "%"PRId32, !=, _ASSERT_INTERNAL_HELPER_NE, a, b, ##__VA_ARGS__)
#define ASSERT_LT_D32(a, b, ...)	ASSERT_TEMPLATE_VA(__VA_ARGS__)(int32_t, "%"PRId32,  <, _ASSERT_INTERNAL_HELPER_LT, a, b, ##__VA_ARGS__)

#define ASSERT_TEMPLATE_VA(...)									TEST_JOIN(ASSERT_TEMPLATE_VA_, TEST_ARG_COUNT(__VA_ARGS__))
#define ASSERT_TEMPLATE_VA_0(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_1(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_2(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_3(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_4(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_5(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_6(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_7(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_8(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))
#define ASSERT_TEMPLATE_VA_9(TYPE, FMT, OP, CMP, a, b, ...)		TEST_EXPAND(ASSERT_TEMPLATE(TYPE, FMT, OP, CMP, a, b, __VA_ARGS__))

#define _ASSERT_INTERNAL_HELPER_EQ(a, b)						((a) == (b))
#define _ASSERT_INTERNAL_HELPER_NE(a, b)						((a) != (b))
#define _ASSERT_INTERNAL_HELPER_LT(a, b)						((a) < (b))
#define _ASSERT_INTERNAL_HELPER_LE(a, b)						((a) <= (b))
#define _ASSERT_INTERNAL_HELPER_GT(a, b)						((a) > (b))
#define _ASSERT_INTERNAL_HELPER_GE(a, b)						((a) >= (b))

#define TEST(name)	\
	static void run_test_##name(void);\
	int main(int argc, char* argv[]) {\
		(void)argc; (void)argv;\
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);\
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);\
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);\
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);\
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);\
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);\
		run_test_##name();\
		return _CrtDumpMemoryLeaks();\
	}\
	static void run_test_##name(void)

#ifdef __cplusplus
}
#endif
#endif
