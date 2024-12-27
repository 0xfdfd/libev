#ifndef __EV_ASYNC_INTERNAL_H__
#define __EV_ASYNC_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

struct ev_async
{
    ev_handle_t      base;         /**< Base object */
    ev_async_cb      activate_cb;  /**< Activate callback */
    void            *activate_arg; /**< Activate argument. */
    ev_async_cb      close_cb;     /**< Close callback */
    void*            close_arg;
    EV_ASYNC_BACKEND backend;      /**< Platform related fields */
};

/**
 * @brief Force destroy #ev_async_t.
 * @param[in] handle    A initialized #ev_async_t handler.
 */
EV_LOCAL void ev__async_exit_force(ev_async_t *handle);

#ifdef __cplusplus
}
#endif
#endif
