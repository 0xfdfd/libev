#ifndef __EV_THREAD_H__
#define __EV_THREAD_H__

#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_Thread Thread
 * @{
 */

#define EV_THREAD_WAIT_INFINITE ((unsigned)-1)

/**
 * @brief Thread callback
 * @param[in] arg       User data
 */
typedef void (*ev_thread_cb)(void* arg);

typedef struct ev_thread_opt
{
    struct
    {
        unsigned    have_stack_size : 1;    /**< Enable stack size */
    }flags;
    size_t          stack_size;             /**< Stack size. */
}ev_thread_opt_t;

/**
 * @brief Create thread
 * @param[out] thr  Thread handle
 * @param[in] opt   Option
 * @param[in] cb    Thread body
 * @param[in] arg   User data
 * @return          #ev_errnot_t
 */
int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt, ev_thread_cb cb, void* arg);

/**
 * @brief Exit thread
 * @warning Cannot be called in thread body.
 * @param[in] thr       Thread handle
 * @param[in] timeout   Timeout in milliseconds. #EV_THREAD_WAIT_INFINITE to wait infinite.
 * @return              #EV_ETIMEDOUT if timed out before thread terminated,
 *                      #EV_SUCCESS if thread terminated.
 */
int ev_thread_exit(ev_os_thread_t* thr, unsigned timeout);

/**
 * @brief Get self handle
 * @return          Thread handle
 */
ev_os_thread_t ev_thread_self(void);

/**
 * @brief Check whether two thread handle points to same thread
 * @param[in] t1    1st thread
 * @param[in] t2    2st thread
 * @return          bool
 */
int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2);

/**
 * @brief Suspends the execution of the calling thread.
 * @param[in] req   Timeout in milliseconds.
 * @param[out] rem  The remaining time if interrupted by a signal
 * @return          #ev_errno_t
 */
int ev_thread_sleep(unsigned req, unsigned* rem);

/**
 * @} EV_Thread
 */

#ifdef __cplusplus
}
#endif
#endif
