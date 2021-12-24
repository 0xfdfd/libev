#ifndef __EV_SEM_H__
#define __EV_SEM_H__

#include "ev/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_SEMAPHORE Semaphore
 * @{
 */

typedef struct ev_sem
{
    union
    {
        int         i;
        ev_os_sem_t r;
    }u;
}ev_sem_t;

/**
 * @brief Initializes the unnamed semaphore at the address pointed to by \p sem.
 * @param[out] sem      Semaphore to be initialized.
 * @param[in] value     Initial value
 * @return              #ev_errno_t
 */
int ev_sem_init(ev_sem_t* sem, unsigned value);

/**
 * @brief Destroy the unnamed semaphore at the address pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
void ev_sem_exit(ev_sem_t* sem);

/**
 * @brief Increments (unlocks)  the  semaphore pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
void ev_sem_post(ev_sem_t* sem);

/**
 * @brief Decrements (locks) the semaphore pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
void ev_sem_wait(ev_sem_t* sem);

/**
 * @brief If the decrement cannot be immediately performed, then call returns an
 *   error #EV_EAGAIN instead of blocking.
 * @param[in] sem       Semaphore handle
 * @return              #EV_SUCCESS if success, #EV_EAGAIN if failed.
 */
int ev_sem_try_wait(ev_sem_t* sem);

/**
 * @} EV_SEMAPHORE
 */

#ifdef __cplusplus
}
#endif

#endif