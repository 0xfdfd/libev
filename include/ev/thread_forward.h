#ifndef __EV_THREAD_FORWARD_H__
#define __EV_THREAD_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_Thread Thread
 * @{
 */

/**
 * @brief Infinite timeout.
 */
#define EV_THREAD_WAIT_INFINITE ((unsigned long)-1)

struct ev_thread_opt;

/**
 * @brief Typedef of #ev_thread_opt.
 */
typedef struct ev_thread_opt ev_thread_opt_t;

struct ev_tls;

/**
 * @brief Typedef of #ev_tls.
 */
typedef struct ev_tls ev_tls_t;

/**
 * @brief Thread callback
 * @param[in] arg       User data
 */
typedef void (*ev_thread_cb)(void* arg);

/**
 * @} EV_Thread
 */

#ifdef __cplusplus
}
#endif
#endif
