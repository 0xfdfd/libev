#ifndef __EV_TIMER_H__
#define __EV_TIMER_H__

#include "ev/loop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TIMER Timer
 * @{
 */

/**
 * @brief Type definition for callback passed to #ev_timer_start().
 * @param[in] handle    A pointer to #ev_timer_t structure
 */
typedef void(*ev_timer_cb)(ev_timer_t* timer);

struct ev_timer
{
    ev_handle_t             base;               /**< Base object */
    ev_map_node_t           node;               /**< (#ev_loop_t::timer::heap) */

    ev_timer_cb             close_cb;           /**< Close callback */

    struct
    {
        uint64_t            active;             /**< Active time */
    }data;

    struct
    {
        ev_timer_cb         cb;                 /**< User callback */
        uint64_t            timeout;            /**< Timeout */
        uint64_t            repeat;             /**< Repeat */
    }attr;
};
#define EV_TIMER_INIT       { EV_HANDLE_INIT, EV_MAP_NODE_INIT, NULL, { 0 }, { NULL, 0, 0 } }

/**
 * @brief Initialize the handle.
 * @param[in] loop      A pointer to the event loop
 * @param[out] handle   The structure to initialize
 * @return              #ev_errno_t
 */
int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle);

/**
 * @brief Destroy the timer
 * @warning The timer structure cannot be freed until close_cb is called.
 * @param[in] handle    Timer handle
 * @param[in] close_cb  Close callback
 */
void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb);

/**
 * @brief Start the timer. timeout and repeat are in milliseconds.
 *
 * If timeout is zero, the callback fires on the next event loop iteration. If
 * repeat is non-zero, the callback fires first after timeout milliseconds and
 * then repeatedly after repeat milliseconds.
 *
 * @param[in] handle    Timer handle
 * @param[in] cb        Active callback
 * @param[in] timeout   The first callback timeout
 * @param[in] repeat    Repeat timeout
 * @return              #ev_errno_t
 */
int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat);

/**
 * @brief Stop the timer.
 *
 * the callback will not be called anymore.
 *
 * @param[in] handle    Timer handle
 */
void ev_timer_stop(ev_timer_t* handle);

/**
 * @} EV_TIMER
 */

#ifdef __cplusplus
}
#endif

#endif
