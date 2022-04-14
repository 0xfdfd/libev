#ifndef __EV_THREAD_POOL_H__
#define __EV_THREAD_POOL_H__

#include "ev/loop_forward.h"
#include "ev/thread.h"
#include "ev/mutex.h"
#include "ev/sem.h"
#include "ev/list.h"
#include "ev/async.h"
#include "ev/todo.h"
#include "ev/threadpool_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_THREAD_POOL
 * @{
 */

/**
 * @brief Work type.
 */
enum ev_threadpool_work_type
{
    /**
     * @brief CPU work
     */
    EV_THREADPOOL_WORK_CPU,

    /**
     * @brief Fast IO. Typically file system operations.
     */
    EV_THREADPOOL_WORK_IO_FAST,

    /**
     * @brief Slow IO. Typically network operations.
     */
    EV_THREADPOOL_WORK_IO_SLOW,
};

/**
 * @brief Thread pool handle type.
 */
struct ev_threadpool
{
    ev_mutex_t                      mutex;          /**< Thread pool mutex */
    ev_sem_t                        ent_sem;        /**< Semaphore for thread entrypoint */
    ev_sem_t                        p2w_sem;        /**< Semaphore for pool to worker */
    ev_sem_t                        w2p_sem;        /**< Semaphore for worker to pool */

    ev_os_thread_t*                 threads;        /**< Threads */
    size_t                          thrnum;         /**< The number of threads */

    int                             looping;        /**< Looping flag */

    ev_queue_node_t                 cpu_queue;      /**< work queue for #EV_THREADPOOL_WORK_CPU */
    ev_queue_node_t                 io_fast_queue;  /**< work queue for #EV_THREADPOOL_WORK_IO_FAST */
    ev_queue_node_t                 io_slow_queue;  /**< work queue for #EV_THREADPOOL_WORK_IO_SLOW */
};

/**
 * @brief Thread pool work token.
 */
struct ev_threadpool_work
{
    ev_handle_t                     base;           /**< Base object */

    ev_queue_node_t                 node;           /**< List node */
    ev_todo_t                       token;          /**< Callback token */
    struct
    {
        ev_threadpool_t*            pool;           /**< Thread pool */
        ev_loop_t*                  loop;           /**< Event loop */

        /**
         * @brief Work status.
         * + #EV_ELOOP:     In queue but not called yet.
         * + #EV_EBUSY:     Already in process
         * + #EV_ECANCELED: Canceled
         * + #EV_SUCCESS:   Done
         */
        int                         status;

        ev_threadpool_work_cb       work_cb;        /**< work callback */
        ev_threadpool_work_done_cb  done_cb;        /**< done callback */
    }data;
};

/**
 * @brief Initialize thread pool
 * @param[out] pool     Thread pool
 * @param[in] opt       Thread option
 * @param[in] storage   Storage to save thread
 * @param[in] num       Storage size
 * @return              #ev_errno_t
 */
int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num);

/**
 * @brief Exit thread pool
 * @param[in] pool      Thread pool
 */
void ev_threadpool_exit(ev_threadpool_t* pool);

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
int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_threadpool_work_t* token, ev_threadpool_work_type_t type,
    ev_threadpool_work_cb work_cb, ev_threadpool_work_done_cb done_cb);

/**
 * @brief Cancel task.
 * @note No matter the task is canceled or not, the task always callback in the
 *   event loop.
 * @param[in] token     Work token
 * @return              #ev_errno_t
 */
int ev_threadpool_cancel(ev_threadpool_work_t* token);

/**
 * @} EV_THREAD_POOL
 */

#ifdef __cplusplus
}
#endif

#endif
