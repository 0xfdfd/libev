#ifndef __TEST_EV_LOOP_T_H__
#define __TEST_EV_LOOP_T_H__

#include "cutest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT_EQ_EVLOOP(a, b, ...)	ASSERT_TEMPLATE(ev_loop_t*, ==, a, b, __VA_ARGS__)
#define ASSERT_NE_EVLOOP(a, b, ...)	ASSERT_TEMPLATE(ev_loop_t*, !=, a, b, __VA_ARGS__)

void test_register_ev_loop_t(void);

#ifdef __cplusplus
}
#endif
#endif
