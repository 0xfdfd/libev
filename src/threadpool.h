#ifndef __EV_THREADPOOL_INTERNAL_H__
#define __EV_THREADPOOL_INTERNAL_H__

#include "ev.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL int ev_loop_unlink_threadpool(ev_loop_t* loop);

/**
 * @brief Submit task into thread pool
 * @warning This function is NOT MT-Safe and must be called in the thread where
 *   \p loop is running.
 * @param[in] pool      Thread pool
 * @param[in] loop      Which event loop to call \p done_cb
 * @param[in] token     Work token
 * @param[in] type      Work type
 * @param[in] work_cb   Work callback
 * @param[in] done_cb   Work done callback
 * @return              #ev_errno_t
 */
API_LOCAL int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_threadpool_work_t* token, ev_threadpool_work_type_t type,
    ev_threadpool_work_cb work_cb, ev_threadpool_work_done_cb done_cb);

/**
 * @brief Submit task to threadpool.
 * @param[in] loop      Event loop.
 * @param[in] work      Work token.
 * @param[in] type      Work type.
 * @param[in] work_cb   Work callback.
 * @param[in] done_cb   Work done callback.
 * @return              #ev_errno_t
 */
API_LOCAL int ev__loop_submit_threadpool(ev_loop_t* loop,
    ev_threadpool_work_t* work, ev_threadpool_work_type_t type,
    ev_threadpool_work_cb work_cb, ev_threadpool_work_done_cb done_cb);

/**
 * @brief Process thread pool events.
 * @param[in] loop Event loop.
 */
API_LOCAL void ev__threadpool_process(ev_loop_t* loop);

/**
 * @brief Wakeup event loop.
 * @param[in] loop Event loop.
 */
API_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
