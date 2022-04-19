#ifndef __EV_PROCESS_H__
#define __EV_PROCESS_H__

#include "ev/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PROCESS Process
 * @{
 */

struct ev_exec_opt_s;

/**
 * @brief Typedef of #ev_exec_opt_s.
 */
typedef struct ev_exec_opt_s ev_exec_opt_t;

/**
 * @brief Process executable options.
 */
struct ev_exec_opt_s
{
    /**
     * @brief Execute command line.
     */
    char* const*    argv;

    /**
     * @brief Environment list.
     */
    char* const*    envp;

    /**
     * @brief Set it non-zero to enable stdio redirect.
     * @see ev_exec_opt_t::stdios
     */
    int             use_std_handles;

    /**
     * @brief Pipe for redirect stdin / stdout / stderr.
     * @note This field requires #ev_exec_opt_t::use_std_handles set to 1.
     */
    ev_os_pipe_t    stdios[3];
} ;

/**
 * @brief Execute process.
 * @param[out] pid      Child Process Identifier.
 * @param[in] opt       Process options.
 * @return              #ev_errno_t
 */
int ev_exec(ev_os_pid_t* pid, const ev_exec_opt_t* opt);

/**
 * @brief Wait for process exit.
 * @param[in] pid       Child Process Identifier.
 */
void ev_waitpid(ev_os_pid_t pid);

/**
 * @} EV_PROCESS
 */

#ifdef __cplusplus
}
#endif
#endif
