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
     * @brief (Optional) Child process exit callback.
     */
    ev_process_sigchld_cb           on_exit;

    /**
     * @brief Execute file.
     */
    const char*                     file;

    /**
     * @brief Execute command line.
     */
    char* const*                    argv;

    /**
     * @brief (Optional) Environment list.
     */
    char* const*                    envp;

    /**
     * @brief (Optional) Current working directory.
     */
    const char*                     cwd;

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

    ev_process_sigchld_cb           sigchild_cb;    /**< Process exit callback */
    ev_process_exit_cb              exit_cb;        /**< Exit callback. */

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
 * @brief Exit process handle.
 * @param[in] handle    Process handle.
 * @param[in] cb        Exit callback.
 */
void ev_process_exit(ev_process_t* handle, ev_process_exit_cb cb);

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
 * @brief Get current working directory.
 * @note The trailing slash is always removed.
 * @param[out] buffer   Buffer to store string. The terminating null byte is
 *   always appended.
 * @param[in] size      Buffer size.
 * @return The number of bytes would have been written to the buffer (excluding
 *   the terminating null byte). Thus, a return value of \p size or more means
 *   that the output was truncated.
 * @return #ev_errno_t if error occur.
 */
ssize_t ev_getcwd(char* buffer, size_t size);

/**
 * @brief Gets the executable path.
 * @param[out] buffer   Buffer to store string. The terminating null byte is
 *   always appended.
 * @param[in] size      Buffer size.
 * @return The number of bytes would have been written to the buffer (excluding
 *   the terminating null byte). Thus, a return value of \p size or more means
 *   that the output was truncated.
 * @return #ev_errno_t if error occur.
 */
ssize_t ev_exepath(char* buffer, size_t size);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
