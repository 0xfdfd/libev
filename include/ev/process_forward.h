#ifndef __EV_PROCESS_FORWARD_H__
#define __EV_PROCESS_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PROCESS Process
 * @{
 */

enum ev_process_exit_status_e
{
    /**
     * @brief The child terminated, but we don't known how or why.
     */
    EV_PROCESS_EXIT_UNKNOWN,

    /**
     * @brief The child terminated normally, that is, by calling exit(3) or
     *   _exit(2), or by returning from main().
     */
    EV_PROCESS_EXIT_NORMAL,

    /**
     * @brief The child process was terminated by a signal.
     */
    EV_PROCESS_EXIT_SIGNAL,
};

/**
 * @brief Typedef of #ev_process_exit_status_e.
 */
typedef enum ev_process_exit_status_e ev_process_exit_status_t;

enum ev_process_stdio_flags_e
{
    /**
     * @brief Ignore this field.
     */
    EV_PROCESS_STDIO_IGNORE         = 0x00,

    /**
     * @brief Redirect stdio from/to `/dev/null`.
     */
    EV_PROCESS_STDIO_REDIRECT_NULL  = 0x01,

    /**
     * @brief Redirect stdio from/to file descriptor.
     * @note The fd will not closed automatically, so you need to do it by
     *   yourself.
     */
    EV_PROCESS_STDIO_REDIRECT_FD    = 0x02,

    /**
     * @brief Redirect stdio from/to #ev_pipe_t.
     *
     * The #ev_process_stdio_container_t::data::pipe field must point to a
     * #ev_pipe_t object that has been initialized with #ev_pipe_init() but not
     * yet opened or connected.
     */
    EV_PROCESS_STDIO_REDIRECT_PIPE  = 0x04,
};

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
 * @brief Typedef of #ev_process_options_s.
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
