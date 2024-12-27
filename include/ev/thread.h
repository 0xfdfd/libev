#ifndef __EV_THREAD_H__
#define __EV_THREAD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_Thread Thread
 * @{
 */

/**
 * @brief Thread callback
 * @param[in] arg       User data
 */
typedef void (*ev_thread_cb)(void *arg);

/**
 * @brief Thread attribute.
 */
typedef struct ev_thread_opt
{
    struct
    {
        unsigned have_stack_size : 1; /**< Enable stack size */
    } flags;
    size_t stack_size; /**< Stack size. */
} ev_thread_opt_t;

/**
 * @brief Thread.
 */
typedef struct ev_thread ev_thread_t;

/**
 * @brief Thread local storage.
 */
typedef struct ev_thread_key ev_thread_key_t;

/**
 * @brief Create thread
 * @param[out] thr  Thread handle
 * @param[in] opt   Option
 * @param[in] cb    Thread body
 * @param[in] arg   User data
 * @return          #ev_errno_t
 */
EV_API int ev_thread_init(ev_thread_t **thr, const ev_thread_opt_t *opt,
                          ev_thread_cb cb, void *arg);

/**
 * @brief Exit thread
 * @warning Cannot be called in thread body.
 * @param[in] thr       Thread handle
 * @param[in] timeout   Timeout in milliseconds. #EV_INFINITE_TIMEOUT to wait
 * infinite.
 * @return              #EV_ETIMEDOUT if timed out before thread terminated,
 *                      #EV_SUCCESS if thread terminated.
 */
EV_API int ev_thread_exit(ev_thread_t *thr, unsigned long timeout);

/**
 * @brief Get current thread id,
 * @return          Thread ID
 */
EV_API ev_os_tid_t ev_thread_id(void);

/**
 * @brief Suspends the execution of the calling thread.
 * @param[in] timeout   Timeout in milliseconds.
 */
EV_API void ev_thread_sleep(uint32_t timeout);

/**
 * @brief Initialize thread local storage.
 * @param[out] key  A pointer to thread local storage.
 * @return          #ev_errno_t
 */
EV_API int ev_thread_key_init(ev_thread_key_t **key);

/**
 * @brief Destroy thread local storage.
 * @param[in] key   A initialized thread local storage handler.
 */
EV_API void ev_thread_key_exit(ev_thread_key_t *key);

/**
 * @brief Set thread local value.
 * @param[in] key   A initialized thread local storage handler.
 * @param[in] val   A thread specific value.
 */
EV_API void ev_thread_key_set(ev_thread_key_t *key, void *val);

/**
 * @brief Get thread local value.
 * @param[in] key   A initialized thread local storage handler.
 * @return          A thread specific value.
 */
EV_API void *ev_thread_key_get(ev_thread_key_t *key);

/**
 * @} EV_Thread
 */

#ifdef __cplusplus
}
#endif
#endif
