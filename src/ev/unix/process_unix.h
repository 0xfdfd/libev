#ifndef __EV_PROCESS_UNIX_H__
#define __EV_PROCESS_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_process_ctx_s
{
    ev_list_t       wait_queue;         /**< #ev_process_t::node */
    ev_mutex_t      wait_queue_mutex;   /**< Mutex for wait_queue */
} ev_process_ctx_t;

/**
 * @brief Initialize process context.
 */
EV_LOCAL void ev__init_process_unix(void);

#ifdef __cplusplus
}
#endif
#endif
