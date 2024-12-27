#ifndef __EV_PROCESS_UNIX_H__
#define __EV_PROCESS_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize process context.
 */
EV_LOCAL void ev__init_process_unix(void);

EV_LOCAL void ev__exit_process_unix(void);

#ifdef __cplusplus
}
#endif
#endif
