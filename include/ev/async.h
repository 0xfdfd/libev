#ifndef __EV_ASYNC_H__
#define __EV_ASYNC_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ASYNC Async
 * @{
 */

struct ev_async;

/**
 * @brief Typedef of #ev_async.
 */
typedef struct ev_async ev_async_t;

/**
 * @brief Type definition for callback passed to #ev_async_init().
 * @param[in] async     A pointer to #ev_async_t structure
 */
typedef void(*ev_async_cb)(ev_async_t* async);

/**
 * @brief Async handle type.
 */
struct ev_async
{
    ev_handle_t             base;               /**< Base object */
    ev_async_cb             active_cb;          /**< Active callback */
    ev_async_cb             close_cb;           /**< Close callback */
    EV_ASYNC_BACKEND        backend;            /**< Platform related fields */
};

/**
 * @brief Static initializer for #ev_async_t.
 * @note A static initialized #ev_async_t is not a valid handle.
 */
#define EV_ASYNC_INVALID    \
    {\
        EV_HANDLE_INVALID,\
        NULL,\
        NULL,\
        EV_ASYNC_PLT_INVALID,\
    }

/**
 * @brief Initialize the handle.
 *
 * A NULL callback is allowed.
 *
 * @param[in] loop      Event loop
 * @param[out] handle   A pointer to the structure
 * @param[in] cb        Active callback
 * @return              #ev_errno_t
 */
EV_API int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb);

/**
 * @brief Destroy the structure.
 * @param[in] handle    Async handle
 * @param[in] close_cb  Close callback
 */
EV_API void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb);

/**
 * @brief Wake up the event loop and call the async handle's callback.
 * @note MT-Safe
 * @param[in] handle    Async handle
 */
EV_API void ev_async_wakeup(ev_async_t* handle);

/**
 * @} EV_ASYNC
 */

#ifdef __cplusplus
}
#endif
#endif
