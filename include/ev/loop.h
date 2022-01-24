#ifndef __EV_LOOP_H__
#define __EV_LOOP_H__

#include "ev/defs.h"
#include "ev/list.h"
#include "ev/mutex.h"
#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_EVENT_LOOP Event loop
 * @{
 */

typedef enum ev_loop_mode
{
    /**
     * @brief Runs the event loop until there are no more active and referenced
     *   handles or requests.
     *
     * Returns non-zero if #ev_loop_stop() was called and there are
     * still active handles or requests. Returns zero in all other cases.
     */
    EV_LOOP_MODE_DEFAULT,

    /**
     * @brief Poll for I/O once.
     *
     * Note that this function blocks if there are no pending callbacks. Returns
     * zero when done (no active handles or requests left), or non-zero if more
     * callbacks are expected (meaning you should run the event loop again sometime
     * in the future).
     */
    EV_LOOP_MODE_ONCE,

    /**
     * @brief Poll for i/o once but don't block if there are no pending callbacks.
     *
     * Returns zero if done (no active handles or requests left), or non-zero if
     * more callbacks are expected (meaning you should run the event loop again
     * sometime in the future).
     */
    EV_LOOP_MODE_NOWAIT,
}ev_loop_mode_t;

struct ev_loop
{
    uint64_t                        hwtime;             /**< A fast clock time in milliseconds */

    struct
    {
        ev_list_t                   idle_list;          /**< (#ev_handle::node) All idle handles */
        ev_list_t                   active_list;        /**< (#ev_handle::node) All active handles */
    }handles;

    struct
    {
        ev_list_t                   pending;            /**< (#ev_todo_t::node) Pending task */
    }todo;

    struct
    {
        ev_map_t                    heap;               /**< (#ev_timer_t::node) Timer heap */
    }timer;

    struct
    {
        struct
        {
            ev_mutex_t              mutex;
            ev_queue_node_t    queue;              /**< Async handle queue. #ev_async_t::backend::node */
        }async;

        struct
        {
            ev_mutex_t              mutex;
            ev_list_t               queue;              /**< #ev_todo_t::node */
        }work;
    }wakeup;

    struct
    {
        unsigned                    b_stop : 1;         /**< Flag: need to stop */
    }mask;

    ev_loop_plt_t                   backend;            /**< Platform related implementation */
};
#define EV_LOOP_INIT        \
    {\
        0,                                      /* .hwtime */\
        { EV_LIST_INVALID, EV_LIST_INVALID },         /* .handles */\
        { EV_LIST_INVALID },                       /* .todo */\
        { EV_MAP_INIT(NULL, NULL) },            /* .timer */\
        { 0 },                                  /* .mask */\
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
 * @} EV_EVENT_LOOP
 */

#ifdef __cplusplus
}
#endif
#endif
