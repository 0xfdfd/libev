#ifndef __EV_THREAD_WIN_INTERNAL_H__
#define __EV_THREAD_WIN_INTERNAL_H__

#include "ev.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize thread context.
 */
EV_LOCAL void ev__thread_init_win(void);

#ifdef __cplusplus
}
#endif
#endif
