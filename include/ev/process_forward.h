#ifndef __EV_PROCESS_FORWARD_H__
#define __EV_PROCESS_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PROCESS Process
 * @{
 */

enum ev_process_exit_status_e;

/**
 * @brief Typedef of #ev_process_exit_status_e.
 */
typedef enum ev_process_exit_status_e ev_process_exit_status_t;

enum ev_process_stdio_flags_e;

/**
 * @brief Typedef of #ev_process_stdio_flags_e.
 */
typedef enum ev_process_stdio_flags_e ev_process_stdio_flags_t;

struct ev_process_stdio_container_s;

/**
 * @brief Typedef of #ev_process_stdio_container_s.
 */
typedef struct ev_process_stdio_container_s ev_process_stdio_container_t;

struct ev_process_options_s;

/**
 * @brief Typedef of #ev_exec_opt_s.
 */
typedef struct ev_process_options_s ev_process_options_t;

struct ev_process_s;

/**
 * @brief Typedef of #ev_process_s.
 */
typedef struct ev_process_s ev_process_t;

/**
 * @brief Process exit callback
 * @param[in] handle        Process handle.
 * @param[in] exit_status   Exit status
 *   +EV_PROCESS_EXIT_UNKNOWN: \p exit_code is meaninglessness.
 *   +EV_PROCESS_EXIT_NORMAL: \p exit_code is the exit status of the child.
 *     This consists of the least significant 8 bits of the status argument
 *     that the child specified in a call to exit(3) or _exit(2) or as the
 *     argument for a return state‐ment in main().
 *   +EV_PROCESS_EXIT_SIGNAL: \p exit_code is the number of the signal that
 *     caused the child process to terminate.
 * @param[in] exit_code     Exit code.
 */
typedef void (*ev_process_exit_cb)(ev_process_t* handle,
        ev_process_exit_status_t exit_status, int exit_code);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
