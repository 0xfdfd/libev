#ifndef __EV_TIMER_INTERNAL_H__
#define __EV_TIMER_INTERNAL_H__

#include "ev.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize timer context.
 * @param[out] loop Event loop
 */
API_LOCAL void ev__init_timer(ev_loop_t* loop);

/**
 * @brief Process timer.
 * @param[in] loop  Event loop
 */
API_LOCAL void ev__process_timer(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
