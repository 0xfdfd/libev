#ifndef __TEST_SOCK_PAIR_H__
#define __TEST_SOCK_PAIR_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"

/**
 * @brief cannot be called in ev callback
 */
void test_sockpair(ev_loop_t* loop, ev_tcp_t* s_sock, ev_tcp_t* c_sock);

#ifdef __cplusplus
}
#endif
#endif
