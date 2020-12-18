#ifndef __TEST_TASK_H__
#define __TEST_TASK_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#	include <windows.h>
#	define ABORT()		DebugBreak(); abort()
#else
#	define ABORT()		abort()
#endif

#define ASSERT(x)	\
	if(!(x)) {\
		fprintf(stderr, "Assertion failed in %s on line %d: %s\n", \
				__FILE__, __LINE__, #x);\
		ABORT();\
	}

#define ASSERT_TEMPLATE(TYPE, FMT, OP, a, b)	\
	do {\
		TYPE _a = a;\
		TYPE _b = b;\
		if (_a OP _b) {\
			break;\
		}\
		fprintf(stderr, "%s:%d failure:\n"\
						"            expected:    `%s` %s `%s`\n"\
						"              actual:    " FMT " vs " FMT "\n",\
			__FILE__, __LINE__, #a, #OP, #b, _a, _b);\
		fflush(NULL);\
		ABORT();\
	} while (0)

#define ASSERT_EQ_PTR(a, b)		ASSERT_TEMPLATE(void*, "%p", ==, a, b)
#define ASSERT_EQ_D32(a, b)		ASSERT_TEMPLATE(int32_t, "%"PRId32, ==, a, b)

#define TEST(name)	\
	static void run_test_##name(void);\
	int main(int argc, char* argv[]) {\
		(void)argc; (void)argv;\
		run_test_##name();\
		return 0;\
	}\
	static void run_test_##name(void)

#ifdef __cplusplus
}
#endif
#endif
