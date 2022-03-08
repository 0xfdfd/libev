#ifndef __EV_LOOP_FORWARD_H__
#define __EV_LOOP_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_EVENT_LOOP Event loop
 * @{
 */

enum ev_loop_mode;
typedef enum ev_loop_mode ev_loop_mode_t;

struct ev_loop;
typedef struct ev_loop ev_loop_t;

/**
 * @} EV_EVENT_LOOP
 */

#ifdef __cplusplus
}
#endif
#endif
