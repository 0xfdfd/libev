#ifndef __EV_PROCESS_H__
#define __EV_PROCESS_H__

#include "ev/backend.h"
#include "ev/pipe_forward.h"
#include "ev/process_forward.h"
#include "ev/async.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_PROCESS
 * @{
 */

/**
 * @brief Process stdio container.
 */
struct ev_process_stdio_container_s
{
    /**
     * @brief Bit-OR of #ev_process_stdio_flags_t controls how a stdio should
     *   be transmitted to the child process.
     */
    ev_process_stdio_flags_t        flag;

    /**
     * @brief Set data according to ev_process_stdio_container_t::flag.
     */
    union
    {
        /**
         * @brief Valid if #ev_process_stdio_container_t::flag set to
         *   #EV_PROCESS_STDIO_REDIRECT_FD.
         * You must close it when no longer needed.
         */
        ev_os_pipe_t                fd;

        /**
         * @brief Valid if #ev_process_stdio_container_t::flag set to
         *   #EV_PROCESS_STDIO_REDIRECT_PIPE.
         */
        ev_pipe_t*                  pipe;
    } data;
};

/**
 * @brief Process executable options.
 */
struct ev_process_options_s
{
    /**
     * @brief (Optional) Process exit callback.
     */
    ev_process_exit_cb              on_exit;

    /**
     * @brief Execute command line.
     */
    char* const*                    argv;

    /**
     * @brief (Optional) Environment list.
     */
    char* const*                    envp;

    /**
     * @brief (Optional) Pipe for redirect stdin / stdout / stderr.
     */
    ev_process_stdio_container_t    stdios[3];
};

/**
 * @brief Process context.
 */
struct ev_process_s
{
    ev_list_node_t                  node;           /**< List node */
    ev_process_exit_cb              exit_cb;        /**< Exit callback */
    ev_os_pid_t                     pid;            /**< Process ID */
    ev_process_exit_status_t        exit_status;    /**< Exit status */
    int                             exit_code;      /**< Exit code or termainl signal  */
    ev_async_t                      sigchld;        /**< SIGCHLD notifier */
    ev_process_backend_t            backend;
};

/**
 * @brief Execute process.
 * @param[in] loop      Event loop handler.
 * @param[out] handle   Child Process Identifier.
 * @param[in] opt       Process options.
 * @return              #ev_errno_t
 */
int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt);

/**
 * @brief Notify when process receive SIGCHLD.
 * 
 * Signal SIGCHLD must be register by user application to known child process
 * exit status. If not register, waitpid() may fail with `ECHILD`.
 *
 * By default we register SIGCHLD on initialize so you don't need to care about
 * it. But there are some situation you might to register SIGCHLD yourself, so
 * when you do it, remember to call #ev_process_sigchld() in your SIGCHLD
 * handler.
 *
 * @param[in] signum    Must always SIGCHLD.
 */
void ev_process_sigchld(int signum);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
