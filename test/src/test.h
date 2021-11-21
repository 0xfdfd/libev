#ifndef __TEST_H__
#define __TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "cutest.h"
#if defined(_MSC_VER)
#	include <windows.h>
#else
#	include <pthread.h>
#endif

/**
 * @brief #TEST_F() with timeout
 */
#define TEST_T(fixture_name, case_name, timeout)	\
	static void TEST_##fixture_name##_##case_name##_timeout(void);\
	TEST_F(fixture_name, case_name) {\
		const uint64_t timeout_ms = (timeout);\
		test_execute_token_t token;\
		ASSERT_EQ_D32(test_thread_execute(&token,\
			TEST_##fixture_name##_##case_name##_timeout), 0);\
		cutest_timestamp_t t_start;\
		cutest_timestamp_get(&t_start);\
		int ret;\
		while ((ret = test_thread_wait(&token)) > 0) {\
			cutest_timestamp_t t_current, t_diff;\
			cutest_timestamp_get(&t_current);\
			cutest_timestamp_dif(&t_current, &t_start, &t_diff);\
			uint64_t spend_time_ms = t_diff.sec * 1000 + t_diff.usec / 1000;\
			ASSERT_LE_U64(spend_time_ms, timeout_ms);\
			test_thread_sleep(1);\
		}\
		ASSERT_EQ_D32(ret, 0, "task not finished");\
	}\
	static void TEST_##fixture_name##_##case_name##_timeout(void)

typedef void (*fn_execute)(void);

typedef struct test_execute_token
{
#if defined(_MSC_VER)
	HANDLE		thr_handle;
#else
	pthread_t	thr_handle;
#endif
}test_execute_token_t;

int test_thread_execute(test_execute_token_t* token, fn_execute callback);

/**
 * @return 0: thread finish, -1:error, 1: not finish
 */
int test_thread_wait(test_execute_token_t* token);

void test_thread_sleep(unsigned ms);

#ifdef __cplusplus
}
#endif
#endif
