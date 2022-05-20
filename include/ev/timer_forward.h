#ifndef __EV_TIMER_FORWARD_H__
#define __EV_TIMER_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TIMER Timer
 * @{
 */

struct ev_timer;

/**
 * @brief Typedef of #ev_timer.
 */
typedef struct ev_timer ev_timer_t;

/**
 * @brief Type definition for callback passed to #ev_timer_start().
 * @param[in] timer     A pointer to #ev_timer_t structure
 */
typedef void(*ev_timer_cb)(ev_timer_t* timer);

/**
 * @} EV_TIMER
 */

#ifdef __cplusplus
}
#endif
#endif
