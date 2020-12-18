#ifndef __EV_H__
#define __EV_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#	include "ev/win.h"
#else
#	include "ev/unix.h"
#endif

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
 * @param[in] guard		A pointer to the one-time initialization structure.
 * @param[in] cb		A pointer to an application-defined InitOnceCallback function.
 */
void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

#ifdef __cplusplus
}
#endif
#endif
