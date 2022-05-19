#ifndef __EV_TODO_INTERNAL_H__
#define __EV_TODO_INTERNAL_H__

#include "ev/todo.h"
#include "ev-platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize todo context.
 * @param[out] loop Event loop
 */
API_LOCAL void ev__init_todo(ev_loop_t* loop);

/**
 * @brief Process work token.
 * @param[in] loop  Event loop
 */
API_LOCAL void ev__process_todo(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
