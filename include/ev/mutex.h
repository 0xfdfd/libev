#ifndef __EV_MUTEX_H__
#define __EV_MUTEX_H__

#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_Mutex Mutex
 * @{
 */

/**
 * @brief Initialize the mutex.
 * @param[out] handle   Mutex handle
 * @param[in] recursive Force recursive mutex. Set to non-zero to force create a
 *   recursive mutex. However, a value of zero does not means it is a non-
 *   recursive mutex, it is implementation depend.
 * @return              #ev_errno_t
 */
int ev_mutex_init(ev_os_mutex_t* handle, int recursive);

/**
 * @brief Destroy the mutex object referenced by \p handle
 * @param[in] handle    Mutex object
 */
void ev_mutex_exit(ev_os_mutex_t* handle);

/**
 * @brief The mutex object referenced by \p handle shall be locked.
 * @param[in] handle    Mutex object
 */
void ev_mutex_enter(ev_os_mutex_t* handle);

/**
 * @brief Release the mutex object referenced by \p handle.
 * @param[in] handle    Mutex object
 */
void ev_mutex_leave(ev_os_mutex_t* handle);

/**
 * @brief If the mutex object referenced by \p handle is currently locked, the
 *   call shall return immediately.
 * @param[in] handle    Mutex object
 * @return              #EV_EBUSY: The \p handle could not be acquired because it was already locked.
 *                      #EV_SUCCESS: a lock on the mutex object referenced by \p handle is acquired.
 */
int ev_mutex_try_enter(ev_os_mutex_t* handle);

/**
 * @} EV_Mutex
 */

#ifdef __cplusplus
}
#endif
#endif
