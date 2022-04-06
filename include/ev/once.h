#ifndef __EV_ONCE_H__
#define __EV_ONCE_H__

#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ONCE Once
 * @{
 */

struct ev_once;
typedef struct ev_once ev_once_t;

/**
 * @brief An application-defined callback function.
 *
 * Specify a pointer to this function when calling the #ev_once_execute function.
 */
typedef void(*ev_once_cb)(void);

/**
 * @brief Executes the specified function one time.
 *
 * No other threads that specify the same one-time initialization structure can
 * execute the specified function while it is being executed by the current thread.
 *
 * @param[in] guard     A pointer to the one-time initialization #EV_ONCE_INIT structure.
 * @param[in] cb        A pointer to an application-defined #ev_once_cb function.
 */
void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @} EV_ONCE
 */

#ifdef __cplusplus
}
#endif
#endif
