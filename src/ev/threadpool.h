#ifndef __EV_THREADPOOL_INTERNAL_H__
#define __EV_THREADPOOL_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Work type.
 */
typedef enum ev_work_type
{
    /**
     * @brief CPU work
     */
    EV_THREADPOOL_WORK_CPU      = 0,

    /**
     * @brief Fast IO. Typically file system operations.
     */
    EV_THREADPOOL_WORK_IO_FAST  = 1,

    /**
     * @brief Slow IO. Typically network operations.
     */
    EV_THREADPOOL_WORK_IO_SLOW  = 2,
} ev_work_type_t;

typedef struct ev_threadpool ev_threadpool_t;

/**
 * @brief Thread pool handle type.
 */
struct ev_threadpool
{
    ev_os_thread_t*                 threads;        /**< Threads */
    size_t                          thrnum;         /**< The number of threads */

    ev_list_t                       loop_table;     /**< Loop table */

    ev_mutex_t                      mutex;          /**< Thread pool mutex */
    ev_sem_t                        p2w_sem;        /**< Semaphore for pool to worker */
    int                             looping;        /**< Looping flag */

    ev_queue_node_t                 work_queue[3];  /**< Work queue. Index is #ev_work_type_t */
};

#define EV_THREADPOOL_INVALID   \
    {\
        NULL,\
        0,\
        EV_LIST_INIT,\
        EV_MUTEX_INVALID,\
        EV_SEM_INVALID,\
        0,\
        {\
            EV_QUEUE_NODE_INVALID,\
            EV_QUEUE_NODE_INVALID,\
            EV_QUEUE_NODE_INVALID,\
        },\
    }

EV_LOCAL void ev__loop_link_to_default_threadpool(ev_loop_t* loop);

EV_LOCAL int ev_loop_unlink_threadpool(ev_loop_t* loop);

EV_LOCAL void ev_threadpool_default_cleanup(void);

/**
 * @brief Initialize thread pool
 * @param[out] pool     Thread pool
 * @param[in] opt       Thread option
 * @param[in] storage   Storage to save thread
 * @param[in] num       Storage size
 * @return              #ev_errno_t
 */
EV_LOCAL int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num);

/**
 * @brief Exit thread pool
 * @param[in] pool      Thread pool
 */
EV_LOCAL void ev_threadpool_exit(ev_threadpool_t* pool);

/**
 * @brief Link loop with thread pool.
 *
 * Some actions require a linked thread pool.
 *
 * @param[in] loop      The event loop.
 * @param[in] pool      The Thread pool.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev_loop_link_threadpool(ev_loop_t* loop, ev_threadpool_t* pool);

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
EV_LOCAL int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_work_t* token, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Submit task to threadpool.
 * @param[in] loop      Event loop.
 * @param[in] work      Work token.
 * @param[in] type      Work type.
 * @param[in] work_cb   Work callback.
 * @param[in] done_cb   Work done callback.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__loop_submit_threadpool(ev_loop_t* loop,
    ev_work_t* work, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Process thread pool events.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__threadpool_process(ev_loop_t* loop);

/**
 * @brief Wakeup event loop.
 * @param[in] loop Event loop.
 */
EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif
#endif
