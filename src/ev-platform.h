#ifndef __EV_PLATFORM_INTERNAL_H__
#define __EV_PLATFORM_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#   include <WS2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#include "ev.h"

#if defined(__GNUC__) || defined(__clang__)
#   define API_LOCAL    __attribute__((visibility ("hidden")))
#else
#   define API_LOCAL
#endif

/**
 * @brief Initialize backend
 * @param[in] loop  loop handler
 * @return          #ev_errno_t
 */
API_LOCAL int ev__loop_init_backend(ev_loop_t* loop);

/**
 * @brief Destroy backend
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_exit_backend(ev_loop_t* loop);

/**
 * @brief Update loop time
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_update_time(ev_loop_t* loop);

/**
 * @brief Wait for IO event and process
 * @param[in] loop  loop handler
 * @param[in] timeout   timeout in milliseconds
 */
API_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout);

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr    System error
 * @return              #ev_errno_t
 */
API_LOCAL int ev__translate_sys_error(int syserr);

#ifdef __cplusplus
}
#endif
#endif
