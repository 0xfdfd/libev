#ifndef __TEST_H__
#define __TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "cutest.h"
#if defined(_MSC_VER)
#   include <windows.h>
#else
#   include <pthread.h>
#endif
#include <stdio.h>

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

#if defined(__GNUC__) || defined(__clang__)
#   define container_of(ptr, type, member) \
        ({ \
            const typeof(((type *)0)->member)*__mptr = (ptr); \
            (type *)((char *)__mptr - offsetof(type, member)); \
        })
#else
#   define container_of(ptr, type, member) \
        ((type *) ((char *) (ptr) - offsetof(type, member)))
#endif

#ifndef ALIGN_ADDR
#define ALIGN_ADDR(addr, align) \
    (((uintptr_t)(addr) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

typedef void (*fn_execute)(void);

typedef struct test_execute_token
{
    ev_os_thread_t  thr;
}test_execute_token_t;

int test_thread_execute(test_execute_token_t* token, fn_execute callback);

/**
 * @return EV_SUCCESS / EV_ETIMEDOUT
 */
int test_thread_wait(test_execute_token_t* token);

#ifdef __cplusplus
}
#endif
#endif
