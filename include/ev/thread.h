#ifndef __EV_THREAD_H__
#define __EV_THREAD_H__

#include "ev/backend.h"
#include "ev/thread_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_Thread
 * @{
 */

/**
 * @brief Thread attribute.
 */
struct ev_thread_opt
{
    struct
    {
        unsigned    have_stack_size : 1;    /**< Enable stack size */
    }flags;
    size_t          stack_size;             /**< Stack size. */
};

/**
 * @brief Thread local storage.
 */
struct ev_tls
{
    ev_os_tls_t     tls;                    /**< Thread local storage */
};

/**
 * @brief Create thread
 * @param[out] thr  Thread handle
 * @param[in] opt   Option
 * @param[in] cb    Thread body
 * @param[in] arg   User data
 * @return          #ev_errno_t
 */
int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg);

/**
 * @brief Exit thread
 * @warning Cannot be called in thread body.
 * @param[in] thr       Thread handle
 * @param[in] timeout   Timeout in milliseconds. #EV_INFINITE_TIMEOUT to wait infinite.
 * @return              #EV_ETIMEDOUT if timed out before thread terminated,
 *                      #EV_SUCCESS if thread terminated.
 */
int ev_thread_exit(ev_os_thread_t* thr, unsigned long timeout);

/**
 * @brief Get self handle
 * @return          Thread handle
 */
ev_os_thread_t ev_thread_self(void);

/**
 * @brief Get current thread id,
 * @return          Thread ID
 */
ev_os_tid_t ev_thread_id(void);

/**
 * @brief Check whether two thread handle points to same thread
 * @param[in] t1    1st thread
 * @param[in] t2    2st thread
 * @return          bool
 */
int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2);

/**
 * @brief Suspends the execution of the calling thread.
 * @param[in] timeout   Timeout in milliseconds.
 */
void ev_thread_sleep(uint32_t timeout);

/**
 * @brief Initialize thread local storage.
 * @param[out] tls  A pointer to thread local storage.
 * @return          #ev_errno_t
 */
int ev_tls_init(ev_tls_t* tls);

/**
 * @brief Destroy thread local storage.
 * @param[in] tls   A initialized thread local storage handler.
 */
void ev_tls_exit(ev_tls_t* tls);

/**
 * @brief Set thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @param[in] val   A thread specific value.
 */
void ev_tls_set(ev_tls_t* tls, void* val);

/**
 * @brief Get thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @return          A thread specific value.
 */
void* ev_tls_get(ev_tls_t* tls);

/**
 * @} EV_Thread
 */

#ifdef __cplusplus
}
#endif
#endif
