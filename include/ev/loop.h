#ifndef __EV_LOOP_H__
#define __EV_LOOP_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_EVENT_LOOP Event loop
 * @{
 */

/**
 * @brief Running mode of event loop.
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
} ev_loop_mode_t;

typedef struct ev_work ev_work_t;

/**
 * @brief Thread pool task
 * @param[in] work  Work token
 */
typedef void (*ev_work_cb)(ev_work_t* work);

/**
 * @brief Work done callback in event loop
 * @param[in] work      Work token
 * @param[in] status    Work status
 */
typedef void (*ev_work_done_cb)(ev_work_t* work, int status);

/**
 * @brief Thread pool work token.
 */
struct ev_work
{
    ev_handle_t                     base;           /**< Base object */
    ev_queue_node_t                 node;           /**< List node */

    struct
    {
        struct ev_threadpool*       pool;           /**< Thread pool */

        /**
         * @brief Work status.
         * + #EV_ELOOP:     In queue but not called yet.
         * + #EV_EBUSY:     Already in process
         * + #EV_ECANCELED: Canceled
         * + #EV_SUCCESS:   Done
         */
        int                         status;

        ev_work_cb                  work_cb;        /**< work callback */
        ev_work_done_cb             done_cb;        /**< done callback */
    }data;
};
#define EV_WORK_INVALID \
    {\
        EV_HANDLE_INVALID,\
        EV_QUEUE_NODE_INVALID,\
        { NULL, EV_EINPROGRESS, NULL, NULL },\
    }

/**
 * @brief Typedef of #ev_loop.
 */
typedef struct ev_loop ev_loop_t;

/**
 * @brief Type definition for callback passed to #ev_loop_walk().
 * @param[in] handle    Object handle.
 * @param[in] arg       User defined argument.
 * @return              0 to continue, otherwise stop walk.
 */
typedef int (*ev_walk_cb)(ev_handle_t* handle, void* arg);

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

    ev_list_t                       backlog_queue;      /**< Backlog queue */
    ev_list_t                       endgame_queue;      /**< Close queue */

    /**
     * @brief Timer context
     */
    struct
    {
        ev_map_t                    heap;               /**< #ev_timer_t::node. Timer heap */
    }timer;

    struct
    {
        struct ev_threadpool*       pool;               /**< Thread pool */
        ev_list_node_t              node;               /**< node for #ev_threadpool_t::loop_table */

        ev_mutex_t                  mutex;              /**< Work queue lock */
        ev_list_t                   work_queue;         /**< Work queue */
    } threadpool;

    struct
    {
        unsigned                    b_stop : 1;         /**< Flag: need to stop */
    }mask;

    EV_LOOP_BACKEND                 backend;            /**< Platform related implementation */
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
        EV_LIST_INIT,                           /* .backlog_queue */\
        EV_LIST_INIT,                           /* .endgame_queue */\
        { EV_MAP_INIT(NULL, NULL) },            /* .timer */ \
        {/* .threadpool */\
            NULL,                               /* .pool */\
            EV_LIST_NODE_INIT,                  /* .node */\
            EV_MUTEX_INVALID,                   /* .mutex */\
            EV_LIST_INIT,                       /* .work_queue */\
        },\
        { 0 },                                  /* .mask */\
        EV_LOOP_PLT_INIT,                       /* .backend */\
    }

/**
 * @brief Initializes the given structure.
 * @param[out] loop     Event loop handler
 * @return              #ev_errno_t
 */
EV_API int ev_loop_init(ev_loop_t* loop);

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
EV_API int ev_loop_exit(ev_loop_t* loop);

/**
 * @brief Stop the event loop, causing ev_loop_run() to end as soon as possible.
 *
 * This will happen not sooner than the next loop iteration. If this function
 * was called before blocking for i/o, the loop won't block for i/o on this
 * iteration.
 *
 * @param[in] loop      Event loop handler
 */
EV_API void ev_loop_stop(ev_loop_t* loop);

/**
 * @brief This function runs the event loop.
 * 
 * The \p mode can be one of:
 *
 * + #EV_LOOP_MODE_DEFAULT: Run the event loop until one of following conditions
 *   is met:
 *   1. there are no more active and referenced handles or requests. 
 *   2. \p timeout is reached.
 *
 * + #EV_LOOP_MODE_ONCE: Poll for I/O once. Returns either events are tiggered
 *   or \p timeout is reached.
 *
 * + #EV_LOOP_MODE_NOWAIT: Poll for i/o once but don't block if there are no
 *   pending callbacks. Parameter \p timeout is ignored.
 * 
 * @param[in] loop      Event loop handler
 * @param[in] mode      Running mode.
 * @param[in] timeout   Timeout in milliseconds. Use #EV_INFINITE_TIMEOUT to wait
 *   infinite.
 * @return              Returns zero when no active handles or requests left,
 *                      otherwise return non-zero
 * @see ev_loop_mode_t
 */
EV_API int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode, uint32_t timeout);

/**
 * @brief Submit task into thread pool.
 * @param[in] loop      Event loop.
 * @param[in] token     Work token.
 * @param[in] work_cb   Work callback in thread pool.
 * @param[in] done_cb   Work done callback in event loop.
 * @return              #ev_errno_t
 */
EV_API int ev_loop_queue_work(ev_loop_t* loop, ev_work_t* token,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Cancel task.
 * @note No matter the task is canceled or not, the task always callback in the
 *   event loop.
 * @param[in] token     Work token
 * @return              #ev_errno_t
 */
EV_API int ev_loop_cancel(ev_work_t* token);

/**
 * @brief Walk the list of handles.
 * \p cb will be executed with the given arg.
 * @param[in] loop      The event loop.
 * @param[in] cb        Walk callback.
 * @param[in] arg       User defined argument.
 */
EV_API void ev_loop_walk(ev_loop_t* loop, ev_walk_cb cb, void* arg);

/**
 * @} EV_EVENT_LOOP
 */

#ifdef __cplusplus
}
#endif
#endif
