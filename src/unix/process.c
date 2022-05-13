#define _GNU_SOURCE
#include "ev-common.h"
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

static void _ev_spawn_dup_stdio(const ev_exec_opt_t* opt)
{
    if (!opt->use_std_handles)
    {
        return;
    }

    if (opt->stdios[0] != EV_OS_PIPE_INVALID)
    {
        dup2(opt->stdios[0], STDIN_FILENO);
    }
    if (opt->stdios[1] != EV_OS_PIPE_INVALID)
    {
        dup2(opt->stdios[1], STDOUT_FILENO);
    }
    if (opt->stdios[2] != EV_OS_PIPE_INVALID)
    {
        dup2(opt->stdios[2], STDERR_FILENO);
    }
}

static void _ev_spawn_child(int pipfd, const ev_exec_opt_t* opt)
{
    _ev_spawn_dup_stdio(opt);

    if (opt->envp == NULL)
    {
        (void)execvp(opt->argv[0], opt->argv);
    }
    else
    {
        (void)execvpe(opt->argv[0], opt->argv, opt->envp);
    }

    /* Error */
    int val = errno;
    ssize_t n;
    do
    {
        n = write(pipfd, &val, sizeof(val));
    } while (n == -1 && errno == EINTR);

    exit(EXIT_FAILURE);
}

static int _ev_spawn_parent(ev_os_pid_t pid, int pipfd)
{
    int status;
    pid_t pid_ret;
    int child_errno;
    ssize_t r;

    do
    {
        r = read(pipfd, &child_errno, sizeof(child_errno));
    } while (r == -1 && errno == EINTR);

    /* EOF, child exec success */
    if (r == 0)
    {
        return EV_SUCCESS;
    }

    if (r == sizeof(child_errno))
    {
        do
        {
            pid_ret = waitpid(pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == pid);

        return ev__translate_sys_error(child_errno);
    }

    /* Something unknown happened to our child before spawn */
    if (r == -1 && errno == EPIPE)
    {
        do
        {
            pid_ret = waitpid(pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == pid);

        return EV_EPIPE;
    }

    /* unknown error */
    abort();

    return EV_UNKNOWN;
}

int ev_exec(ev_os_pid_t* pid, const ev_exec_opt_t* opt)
{
    int ret;

    /**
     * pipefd[0] refers to the read end of the pipe. pipefd[1] refers to the
     * write end of the pipe.
     */
    int pipefd[2];

    /* Create pipe for interprocess communication */
    if ((ret = pipe2(pipefd, O_CLOEXEC)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    ev_os_pid_t fork_pid = fork();
    switch (fork_pid)
    {
    case -1:    /* fork failed */
        ret = errno;
        return ev__translate_sys_error(ret);

    case 0:     /* Child process */
        close(pipefd[0]);
        _ev_spawn_child(pipefd[1], opt);
        break;

    default:    /* parent process */
        *pid = fork_pid;
        close(pipefd[1]);
        ret = _ev_spawn_parent(fork_pid, pipefd[0]);
        close(pipefd[0]);
        return ret;
    }

    /* should not reach here. */
    abort();
    return EV_UNKNOWN;
}

int ev_waitpid(ev_os_pid_t pid, uint32_t ms, ev_process_exit_status_t* status)
{
    pid_t wait_ret;
    int wstatus;

    if (ms == EV_INFINITE_TIMEOUT)
    {
        wait_ret = waitpid(pid, &wstatus, 0);
        goto fin;
    }

    const uint64_t start_time = ev__clocktime();
    const uint64_t end_time = start_time + ms;

    for(;;)
    {
        wait_ret = waitpid(pid, &wstatus, WNOHANG);
        if (wait_ret != 0)
        {
            break;
        }

        uint64_t now_time = ev__clocktime();
        if (now_time > end_time)
        {
            return EV_ETIMEDOUT;
        }

        uint64_t dif_time = end_time - now_time;
        if (dif_time > 10)
        {
            dif_time = 10;
        }

        ev_thread_sleep(dif_time);
    }

fin:
    if (wait_ret < 0)
    {
        int errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    if (status == NULL)
    {
        return EV_SUCCESS;
    }

    status->exit_status = 0;
    if (WIFEXITED(wait_ret))
    {
        status->exit_status = WEXITSTATUS(wait_ret);
    }

    status->term_signal = 0;
    if (WIFSIGNALED(wait_ret))
    {
        status->term_signal = WTERMSIG(wait_ret);
    }

    return EV_SUCCESS;
}
