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

#define ASSERT_NE_D32(a, b)		ASSERT_TEMPLATE(int32_t, "%"PRId32, !=, a, b)

#define TEST(name)	\
	static void run_test_##name(void);\
	int main(int argc, char* argv[]) {\
		(void)argc; (void)argv;\
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);\
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);\
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
