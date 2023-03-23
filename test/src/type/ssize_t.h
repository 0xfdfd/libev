#ifndef __TEST_TYPE_SSIZE_T_H__
#define __TEST_TYPE_SSIZE_T_H__

#include "cutest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT_EQ_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, ==, a, b, __VA_ARGS__)
#define ASSERT_NE_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, !=, a, b, __VA_ARGS__)
#define ASSERT_GT_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, >,  a, b, __VA_ARGS__)
#define ASSERT_GE_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, >=, a, b, __VA_ARGS__)
#define ASSERT_LT_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, <,  a, b, __VA_ARGS__)
#define ASSERT_LE_SSIZE(a, b, ...)	ASSERT_TEMPLATE(ssize_t, <=, a, b, __VA_ARGS__)

/**
 * @brief Register typeof `ssize_t'.
 */
void test_register_ssize_t(void);

#ifdef __cplusplus
}
#endif
#endif
