#ifndef __EV_ONCE_H__
#define __EV_ONCE_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_ONCE Once
 * @{
 */

/**
 * @brief Typedef of #ev_once.
 */
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
 * @see #EV_ONCE_INIT
 * @param[in] guard     A pointer to the one-time initialized structure.
 * @param[in] cb        A pointer to an application-defined #ev_once_cb function.
 */
EV_API void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @} EV_ONCE
 */

#ifdef __cplusplus
}
#endif
#endif
