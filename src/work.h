#ifndef __EV_WORK_INTERNAL_H__
#define __EV_WORK_INTERNAL_H__

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL void ev__init_work(ev_loop_t* loop);

API_LOCAL void ev__exit_work(ev_loop_t* loop);

/**
 * @brief Submit task to event loop with multi-thread support.
 * @note MT-Safe
 * @note Use this function in threads that \p loop not running. If the thread
 *   has \p loop running, use #ev_todo_submit().
 * @param[in] loop  Event loop
 * @param[in] token Todo token
 * @param[in] cb    Callback
 */
API_LOCAL void ev__work_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb);

#ifdef __cplusplus
}
#endif

#endif
