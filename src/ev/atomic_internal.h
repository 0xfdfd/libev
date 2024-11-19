#ifndef __EV_ATOMIC_INTERNAL_H__
#define __EV_ATOMIC_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Cleanup atomic context.
 */
EV_LOCAL void ev__atomic_exit(void);

#ifdef __cplusplus
}
#endif
#endif
