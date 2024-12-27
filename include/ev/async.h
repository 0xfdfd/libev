#ifndef __EV_ASYNC_H__
#define __EV_ASYNC_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ASYNC Async
 * @{
 */

/**
 * @brief Async handle type.
 */
typedef struct ev_async ev_async_t;

/**
 * @brief Type definition for callback passed to #ev_async_init().
 * @param[in] async   A pointer to #ev_async_t structure
 * @param[in] arg     User defined argument.
 */
typedef void (*ev_async_cb)(ev_async_t *async, void *arg);

/**
 * @brief Initialize the handle.
 *
 * A NULL callback is allowed.
 *
 * @param[in] loop          Event loop
 * @param[out] handle       A pointer to the structure
 * @param[in] activate_cb   Activate callback
 * @param[in] activate_arg  Activate argument.
 * @return                  #ev_errno_t
 */
EV_API int ev_async_init(ev_loop_t *loop, ev_async_t **handle,
                         ev_async_cb activate_cb, void *activate_arg);

/**
 * @brief Destroy the structure.
 * @param[in] handle    Async handle.
 * @param[in] close_cb  [Optional] Close callback.
 * @param[in] close_arg Close argument.
 * @note When \p close_cb is called, the \p handle object is already released.
 *   Do not access it.
 */
EV_API void ev_async_exit(ev_async_t *handle, ev_async_cb close_cb,
                          void *close_arg);

/**
 * @brief Wake up the event loop and call the async handle's callback.
 * @note MT-Safe
 * @param[in] handle    Async handle
 */
EV_API void ev_async_wakeup(ev_async_t *handle);

/**
 * @} EV_ASYNC
 */

#ifdef __cplusplus
}
#endif
#endif
