#ifndef __EV_THREADPOOL_FORWARD_H__
#define __EV_THREADPOOL_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_THREAD_POOL Thread pool
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
    EV_THREADPOOL_WORK_CPU      = 0,

    /**
     * @brief Fast IO. Typically file system operations.
     */
    EV_THREADPOOL_WORK_IO_FAST  = 1,

    /**
     * @brief Slow IO. Typically network operations.
     */
    EV_THREADPOOL_WORK_IO_SLOW  = 2,
};

typedef enum ev_threadpool_work_type ev_threadpool_work_type_t;

struct ev_threadpool;
typedef struct ev_threadpool ev_threadpool_t;

struct ev_threadpool_work;
typedef struct ev_threadpool_work ev_threadpool_work_t;

/**
 * @brief Thread pool task
 * @param[in] work  Work token
 */
typedef void (*ev_threadpool_work_cb)(ev_threadpool_work_t* work);

/**
 * @brief Work done callback in event loop
 * @param[in] work      Work token
 * @param[in] status    Work status
 */
typedef void (*ev_threadpool_work_done_cb)(ev_threadpool_work_t* work, int status);

/**
 * @} EV_THREAD_POOL
 */

#ifdef __cplusplus
}
#endif
#endif
