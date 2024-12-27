#ifndef __EV_TIMER_INTERNAL_H__
#define __EV_TIMER_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

struct ev_timer
{
    ev_handle_t   base; /**< Base object */
    ev_map_node_t node; /**< #ev_loop_t::timer::heap */

    ev_timer_cb close_cb;  /**< Close callback */
    void       *close_arg; /**< User defined argument. */

    struct
    {
        uint64_t active; /**< Active time */
    } data;

    struct
    {
        ev_timer_cb cb;      /**< User callback */
        void       *arg;     /**< User defined argument. */
        uint64_t    timeout; /**< Timeout */
        uint64_t    repeat;  /**< Repeat */
    } attr;
};

/**
 * @brief Initialize timer context.
 * @param[out] loop Event loop
 */
EV_LOCAL void ev__init_timer(ev_loop_t *loop);

/**
 * @brief Process timer.
 * @param[in] loop  Event loop
 * @return          Active counter
 */
EV_LOCAL size_t ev__process_timer(ev_loop_t *loop);

#ifdef __cplusplus
}
#endif
#endif
