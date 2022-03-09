#ifndef __EV_LOOP_UNIX_H__
#define __EV_LOOP_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"

typedef struct ev_loop_unix_ctx
{
    clockid_t   hwtime_clock_id;    /**< Clock id */
    int         iovmax;             /**< The limits instead of readv/writev */
}ev_loop_unix_ctx_t;

/**
 * @brief Global runtime
 */
extern ev_loop_unix_ctx_t g_ev_loop_unix_ctx;

/**
 * @brief Initialize windows context.
 */
API_LOCAL void ev__init_once_unix(void);

#ifdef __cplusplus
}
#endif
#endif
