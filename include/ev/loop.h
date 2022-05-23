#ifndef __EV_LOOP_H__
#define __EV_LOOP_H__

#include "ev/defs.h"
#include "ev/list.h"
#include "ev/queue.h"
#include "ev/mutex.h"
#include "ev/backend.h"
#include "ev/threadpool_forward.h"
#include "ev/loop_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_EVENT_LOOP
 * @{
 */

/**
 * @brief Event loop type.
 */
struct ev_loop
{
    uint64_t                        hwtime;             /**< A fast clock time in milliseconds */

    struct
    {
        ev_list_t                   idle_list;          /**< (#ev_handle::node) All idle handles */
        ev_list_t                   active_list;        /**< (#ev_handle::node) All active handles */
    }handles;                                           /**< table for handles */

    struct
    {
        ev_list_t                   pending;            /**< (#ev_todo_token_t::node) Pending task */
    }todo;

    /**
     * @brief Timer context
     */
    struct
    {
        ev_map_t                    heap;               /**< #ev_timer_t::node. Timer heap */
    }timer;

    struct
    {
        ev_threadpool_t*            pool;               /**< Thread pool */
        ev_list_node_t              node;               /**< node for #ev_threadpool_t::loop_table */
    }threadpool;

    struct
    {
        unsigned                    b_stop : 1;         /**< Flag: need to stop */
    }mask;

    ev_loop_plt_t                   backend;            /**< Platform related implementation */
};

/**
 * @brief Static initializer for #ev_loop_t.
 * @note A static initialized #ev_loop_t is not a workable event loop, please
 *   initialize with #ev_loop_init().
 */
#define EV_LOOP_INVALID        \
    {\
        0,                                      /* .hwtime */\
        { EV_LIST_INIT, EV_LIST_INIT },         /* .handles */\
        { EV_LIST_INIT },                       /* .todo */\
        { EV_MAP_INIT(NULL, NULL) },            /* .timer */\
        { 0 },                                  /* .mask */\
        NULL,                                   /* .threadpool */\
        EV_LOOP_PLT_INIT,                       /* .backend */\
    }

/**
 * @brief Initializes the given structure.
 * @param[out] loop     Event loop handler
 * @return              #ev_errno_t
 */
int ev_loop_init(ev_loop_t* loop);

/**
 * @brief Releases all internal loop resources.
 *
 * Call this function only when the loop has finished executing and all open
 * handles and requests have been closed, or it will return #EV_EBUSY. After
 * this function returns, the user can free the memory allocated for the loop.
 *
 * @param[in] loop      Event loop handler.
 * @return #ev_errno_t
 */
int ev_loop_exit(ev_loop_t* loop);

/**
 * @brief Stop the event loop, causing uv_run() to end as soon as possible.
 *
 * This will happen not sooner than the next loop iteration. If this function
 * was called before blocking for i/o, the loop won't block for i/o on this
 * iteration.
 *
 * @param[in] loop      Event loop handler
 */
void ev_loop_stop(ev_loop_t* loop);

/**
 * @brief This function runs the event loop.
 *
 * Checkout #ev_loop_mode_t for mode details.
 * @param[in] loop      Event loop handler
 * @param[in] mode      Running mode
 * @return              Returns zero when no active handles or requests left,
 *                      otherwise return non-zero
 * @see ev_loop_mode_t
 */
int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode);

/**
 * @brief Link loop with thread pool.
 * 
 * Some actions require a linked thread pool.
 * 
 * @param[in] loop      The event loop.
 * @param[in] pool      The Thread pool.
 * @return              #ev_errno_t
 */
int ev_loop_link_threadpool(ev_loop_t* loop, ev_threadpool_t* pool);

/**
 * @} EV_EVENT_LOOP
 */

#ifdef __cplusplus
}
#endif
#endif
