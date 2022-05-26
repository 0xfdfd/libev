#ifndef __TEST_H__
#define __TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "cutest.h"
#include "utils/file.h"
#include "tools/memcheck.h"
#if defined(_MSC_VER)
#   include <windows.h>
#else
#   include <pthread.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#   define strdup(s)    _strdup(s)
#endif

/**
 * @brief #TEST_F() with timeout
 */
#define TEST_T(fixture_name, case_name, timeout)    \
    static void TEST_##fixture_name##_##case_name##_timeout(void);\
    TEST_F(fixture_name, case_name) {\
        const uint64_t timeout_ms = (timeout);\
        test_execute_token_t token;\
        ASSERT_EQ_D32(test_thread_execute(&token,\
            TEST_##fixture_name##_##case_name##_timeout), 0);\
        cutest_timestamp_t t_start;\
        cutest_timestamp_get(&t_start);\
        int ret;\
        while ((ret = test_thread_wait(&token)) == EV_ETIMEDOUT) {\
            cutest_timestamp_t t_current, t_diff;\
            cutest_timestamp_get(&t_current);\
            cutest_timestamp_dif(&t_current, &t_start, &t_diff);\
            uint64_t spend_time_ms = t_diff.sec * 1000 + t_diff.usec / 1000;\
            ASSERT_LE_U64(spend_time_ms, timeout_ms);\
            ev_thread_sleep(1);\
        }\
        ASSERT_EQ_D32(ret, 0, "task not finished");\
    }\
    static void TEST_##fixture_name##_##case_name##_timeout(void)

#ifndef ALIGN_ADDR
#define ALIGN_ADDR(addr, align) \
    (((uintptr_t)(addr) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define TEST_PRINT(fmt, ...)   \
    printf("[%s:%d] " fmt "\n", cutest_pretty_file(__FILE__), __LINE__, ##__VA_ARGS__)

typedef void (*fn_execute)(void);

typedef struct test_execute_token
{
    ev_os_thread_t  thr;
}test_execute_token_t;

/**
 * @brief Hook for tests.
 */
extern cutest_hook_t test_hook;

int test_thread_execute(test_execute_token_t* token, fn_execute callback);

/**
 * @return EV_SUCCESS / EV_ETIMEDOUT
 */
int test_thread_wait(test_execute_token_t* token);

const char* test_strerror(int errcode);

#ifdef __cplusplus
}
#endif
#endif
