#ifndef __EV_TIMER_INTERNAL_H__
#define __EV_TIMER_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize timer context.
 * @param[out] loop Event loop
 */
EV_LOCAL void ev__init_timer(ev_loop_t* loop);

/**
 * @brief Process timer.
 * @param[in] loop  Event loop
 * @return          Active counter
 */
EV_LOCAL size_t ev__process_timer(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif
#endif
