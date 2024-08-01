#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdio.h>

typedef struct dup_pair_s
{
    int*    src;
    int     dst;
} dup_pair_t;

typedef struct spawn_helper_s
{
    /**
     * Channel for internal IPC.
     *
     * pipefd[0] refers to the read end of the pipe. pipefd[1] refers to the
     * write end of the pipe.
     *
     * The data always system errno.
     */
    int         pipefd[2];

    /**
     * The file descriptor always need close after fork().
     */
    struct
    {
        int     fd_stdin;
        int     fd_stdout;
        int     fd_stderr;
    } child;
} spawn_helper_t;

static void _ev_spawn_write_errno_and_exit(spawn_helper_t* helper, int value)
{
    ssize_t n;
    do
    {
        n = write(helper->pipefd[1], &value, sizeof(value));
    } while (n == -1 && errno == EINTR);

    exit(EXIT_FAILURE);
}

static int _ev_process_setup_child_fd(int fd)
{
    int ret;

    /* must in block mode */
    if ((ret = ev__nonblock(fd, 0)) != 0)
    {
        return ret;
    }

    /* cannot have FD_CLOEXEC */
    if ((ret = ev__cloexec(fd, 0)) != 0)
    {
        return ret;
    }

    return 0;
}

static int _ev_spawn_setup_stdio_as_fd(int* handle, int fd)
{
    int dup_fd = dup(fd);
    if (dup_fd < 0)
    {
        int errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    int ret = _ev_process_setup_child_fd(dup_fd);
    if (ret != 0)
    {
        close(dup_fd);
        return ret;
    }

    *handle = dup_fd;
    return 0;
}

static int _ev_spawn_setup_stdio_as_null(int* handle, int mode)
{
    int null_fd = open("/dev/null", mode);
    if (null_fd < 0)
    {
        int errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    int ret = _ev_process_setup_child_fd(null_fd);
    if (ret != 0)
    {
        close(null_fd);
        return ret;
    }

    *handle = null_fd;
    return 0;
}

static int _ev_spawn_setup_stdio_as_pipe(ev_pipe_t* pipe, int* handle, int is_pipe_read)
{
    int errcode;
    ev_os_pipe_t pipfd[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };

    /* fd for #ev_pipe_t should open in nonblock mode */
    int rflags = is_pipe_read ? EV_PIPE_NONBLOCK : 0;
    int wflags = is_pipe_read ? 0 : EV_PIPE_NONBLOCK;

    errcode = ev_pipe_make(pipfd, rflags, wflags);
    if (errcode != 0)
    {
        return errcode;
    }

    errcode = _ev_process_setup_child_fd(is_pipe_read ? pipfd[1] : pipfd[0]);
    if (errcode != 0)
    {
        goto err_exit;
    }

    errcode = ev_pipe_open(pipe, is_pipe_read ? pipfd[0] : pipfd[1]);
    if (errcode != 0)
    {
        goto err_exit;
    }

    *handle = is_pipe_read ? pipfd[1] : pipfd[0];
    return 0;

err_exit:
    ev_pipe_close(pipfd[0]);
    ev_pipe_close(pipfd[1]);
    return errcode;
}

static void _ev_spawn_dup_stdio(spawn_helper_t* helper)
{
    int ret = 0;
    dup_pair_t dup_list[] = {
        { &helper->child.fd_stdin, STDIN_FILENO },
        { &helper->child.fd_stdout, STDOUT_FILENO },
        { &helper->child.fd_stderr, STDERR_FILENO },
    };

    size_t i;
    for (i = 0; i < ARRAY_SIZE(dup_list); i++)
    {
        if (*dup_list[i].src == EV_OS_PIPE_INVALID)
        {
            continue;
        }

        ret = dup2(*dup_list[i].src, dup_list[i].dst);
        if (ret < 0)
        {
            ret = errno;
            goto err_dup;
        }

        close(*dup_list[i].src);
        *dup_list[i].src = EV_OS_PIPE_INVALID;
    }

    return;

err_dup:
    _ev_spawn_write_errno_and_exit(helper, ret);
}

static void _ev_spawn_child(spawn_helper_t* helper, const ev_process_options_t* opt)
{
    int errcode;
    _ev_spawn_dup_stdio(helper);

    if (opt->cwd != NULL && chdir(opt->cwd))
    {
        errcode = errno;
        _ev_spawn_write_errno_and_exit(helper, errcode);
    }

    const char* file = opt->file != NULL ? opt->file : opt->argv[0];

    if (opt->envp == NULL)
    {
        (void)execvp(file, opt->argv);
    }
    else
    {
        (void)execvpe(file, opt->argv, opt->envp);
    }

    /* Error */
    errcode = errno;
    _ev_spawn_write_errno_and_exit(helper, errcode);
}

static int _ev_spawn_parent(ev_process_t* handle, spawn_helper_t* spawn_helper)
{
    int status;
    pid_t pid_ret;
    int child_errno = 0;
    ssize_t r;

    do
    {
        r = read(spawn_helper->pipefd[0], &child_errno, sizeof(child_errno));
    } while (r == -1 && errno == EINTR);

    /* EOF, child exec success */
    if (r == 0)
    {
        return 0;
    }

    if (r == sizeof(child_errno))
    {
        do
        {
            pid_ret = waitpid(handle->pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == handle->pid);

        return ev__translate_sys_error(child_errno);
    }

    /* Something unknown happened to our child before spawn */
    if (r == -1 && errno == EPIPE)
    {
        do
        {
            pid_ret = waitpid(handle->pid, &status, 0); /* okay, got EPIPE */
        } while (pid_ret == -1 && errno == EINTR);
        assert(pid_ret == handle->pid);

        return EV_EPIPE;
    }

    /* unknown error */
    EV_ABORT();

    return EV_EPIPE;
}

EV_LOCAL void ev__init_process_unix(void)
{
    ev_list_init(&g_ev_loop_unix_ctx.process.wait_queue);
    ev_mutex_init(&g_ev_loop_unix_ctx.process.wait_queue_mutex, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    if (sigfillset(&sa.sa_mask) != 0)
    {
        EV_ABORT();
    }

    sa.sa_handler = ev_process_sigchld;

    if (sigaction(SIGCHLD, &sa, NULL) != 0)
    {
        EV_ABORT();
    }
}

static void _ev_process_on_async_close(ev_async_t* async)
{
    ev_process_t* handle = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    if (handle->exit_cb != NULL)
    {
        handle->exit_cb(handle);
    }
}

static void _ev_process_on_sigchild_unix(ev_async_t* async)
{
    ev_process_t* handle = EV_CONTAINER_OF(async, ev_process_t, sigchld);
    if (handle->backend.flags.waitpid)
    {
        return;
    }

    int wstatus;
    pid_t pid_ret;
    do
    {
        pid_ret = waitpid(handle->pid, &wstatus, WNOHANG);
    } while(pid_ret == -1 && errno == EINTR);

    if (pid_ret == 0)
    {
        return;
    }
    if (pid_ret == -1)
    {
        int errcode = errno;

        /**
         * The child died, and we missed it. This probably means someone else
         * stole the waitpid().
         */
        if (errcode == ECHILD)
        {
            handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
            goto fin;
        }
    }

    assert(pid_ret == handle->pid);

    if (WIFEXITED(wstatus))
    {
        handle->exit_status = EV_PROCESS_EXIT_NORMAL;
        handle->exit_code = WEXITSTATUS(wstatus);
    }

    if (WIFSIGNALED(wstatus))
    {
        handle->exit_status = EV_PROCESS_EXIT_SIGNAL;
        handle->exit_code = WTERMSIG(wstatus);
    }

fin:
    handle->backend.flags.waitpid = 1;

    if (handle->sigchild_cb != NULL)
    {
        handle->sigchild_cb(handle, handle->exit_status, handle->exit_code);
    }
}

static int _ev_process_init_process(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;

    handle->sigchild_cb = opt->on_exit;
    handle->exit_cb = NULL;
    handle->pid = EV_OS_PID_INVALID;
    handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
    handle->exit_code = 0;
    memset(&handle->backend.flags, 0, sizeof(handle->backend.flags));

    ret = ev_async_init(loop, &handle->sigchld, _ev_process_on_sigchild_unix);
    if (ret != 0)
    {
        return ret;
    }

    ev_mutex_enter(&g_ev_loop_unix_ctx.process.wait_queue_mutex);
    {
        ev_list_push_back(&g_ev_loop_unix_ctx.process.wait_queue, &handle->node);
    }
    ev_mutex_leave(&g_ev_loop_unix_ctx.process.wait_queue_mutex);

    return 0;
}

static int _ev_process_setup_child_stdin(spawn_helper_t* helper,
        const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_spawn_setup_stdio_as_null(&helper->child.fd_stdin, O_RDONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stdin, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stdin, 0);
    }

    return 0;
}

static int _ev_process_setup_child_stdout(spawn_helper_t* helper,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return  _ev_spawn_setup_stdio_as_null(&helper->child.fd_stdout, O_WRONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stdout, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stdout, 1);
    }

    return 0;
}

static int _ev_process_setup_child_stderr(spawn_helper_t* helper,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return 0;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return  _ev_spawn_setup_stdio_as_null(&helper->child.fd_stderr, O_WRONLY);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_spawn_setup_stdio_as_fd(&helper->child.fd_stderr, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_spawn_setup_stdio_as_pipe(container->data.pipe, &helper->child.fd_stderr, 1);
    }

    return 0;
}

static void _ev_process_close_read_end(spawn_helper_t* helper)
{
    if (helper->pipefd[0] != EV_OS_PIPE_INVALID)
    {
        close(helper->pipefd[0]);
        helper->pipefd[0] = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_write_end(spawn_helper_t* helper)
{
    if (helper->pipefd[1] != EV_OS_PIPE_INVALID)
    {
        close(helper->pipefd[1]);
        helper->pipefd[1] = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stdin(spawn_helper_t* helper)
{
    if (helper->child.fd_stdin != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stdin);
        helper->child.fd_stdin = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stdout(spawn_helper_t* helper)
{
    if (helper->child.fd_stdout != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stdout);
        helper->child.fd_stdout = EV_OS_PIPE_INVALID;
    }
}

static void _ev_process_close_stderr(spawn_helper_t* helper)
{
    if (helper->child.fd_stderr != EV_OS_PIPE_INVALID)
    {
        close(helper->child.fd_stderr);
        helper->child.fd_stderr = EV_OS_PIPE_INVALID;
    }
}

static int _ev_process_init_spawn_helper(spawn_helper_t* helper, const ev_process_options_t* opt)
{
    int ret;
    memset(helper, 0, sizeof(*helper));
    helper->child.fd_stdin = EV_OS_PIPE_INVALID;
    helper->child.fd_stdout = EV_OS_PIPE_INVALID;
    helper->child.fd_stderr = EV_OS_PIPE_INVALID;

    if ((ret = pipe2(helper->pipefd, O_CLOEXEC)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    if ((ret = _ev_process_setup_child_stdin(helper, &opt->stdios[0])) != 0)
    {
        goto err_close_pipe;
    }

    if ((ret = _ev_process_setup_child_stdout(helper, &opt->stdios[1])) != 0)
    {
        goto err_close_stdin;
    }

    if ((ret = _ev_process_setup_child_stderr(helper, &opt->stdios[2])) != 0)
    {
        goto err_close_stdout;
    }

    return 0;

err_close_stdout:
    _ev_process_close_stdout(helper);
err_close_stdin:
    _ev_process_close_stdin(helper);
err_close_pipe:
    _ev_process_close_read_end(helper);
    _ev_process_close_write_end(helper);
    return ret;
}

static void _ev_process_exit_spawn_helper(spawn_helper_t* helper)
{
    _ev_process_close_read_end(helper);
    _ev_process_close_write_end(helper);
    _ev_process_close_stdin(helper);
    _ev_process_close_stdout(helper);
    _ev_process_close_stderr(helper);
}

int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;

    spawn_helper_t spawn_helper;
    if ((ret = _ev_process_init_spawn_helper(&spawn_helper, opt)) != 0)
    {
        return ret;
    }

    if ((ret = _ev_process_init_process(loop, handle, opt)) != 0)
    {
        goto finish;
    }

    handle->pid = fork();
    switch (handle->pid)
    {
    case -1:    /* fork failed */
        ret = errno;
        ret = ev__translate_sys_error(ret);
        goto finish;

    case 0:     /* Child process */
        _ev_process_close_read_end(&spawn_helper);
        _ev_spawn_child(&spawn_helper, opt);
        break;

    default:    /* parent process */
        _ev_process_close_write_end(&spawn_helper);
        ret = _ev_spawn_parent(handle, &spawn_helper);
        goto finish;
    }

    /* should not reach here. */
    EV_ABORT();
    return EV_EPIPE;

finish:
    _ev_process_exit_spawn_helper(&spawn_helper);
    return ret;
}

void ev_process_exit(ev_process_t* handle, ev_process_exit_cb cb)
{
    if (handle->pid != EV_OS_PID_INVALID)
    {
        handle->pid = EV_OS_PID_INVALID;
    }

    ev_mutex_enter(&g_ev_loop_unix_ctx.process.wait_queue_mutex);
    {
        ev_list_erase(&g_ev_loop_unix_ctx.process.wait_queue, &handle->node);
    }
    ev_mutex_leave(&g_ev_loop_unix_ctx.process.wait_queue_mutex);

    handle->exit_cb = cb;
    ev_async_exit(&handle->sigchld, _ev_process_on_async_close);
}

void ev_process_sigchld(int signum)
{
    assert(signum == SIGCHLD); (void)signum;

    ev_list_node_t* it = ev_list_begin(&g_ev_loop_unix_ctx.process.wait_queue);
    for (; it != NULL; it = ev_list_next(it))
    {
        ev_process_t* handle = EV_CONTAINER_OF(it, ev_process_t, node);
        ev_async_wakeup(&handle->sigchld);
    }
}

ssize_t ev_getcwd(char* buffer, size_t size)
{
    size_t str_len;
    int errcode;

    if (buffer != NULL && getcwd(buffer, size) != NULL)
    {
        str_len = strlen(buffer);
        if (buffer[str_len - 1] == '/')
        {
            buffer[str_len - 1] = '\0';
            str_len -= 1;
        }

        return str_len;
    }

    const size_t max_path_size = PATH_MAX + 1;
    char* tmp_buf = ev_malloc(max_path_size);
    if (tmp_buf == NULL)
    {
        return EV_ENOMEM;
    }

    if (getcwd(tmp_buf, max_path_size) == NULL)
    {
        errcode = errno;
        ev_free(tmp_buf);
        return ev__translate_sys_error(errcode);
    }

    str_len = strlen(tmp_buf);
    if (tmp_buf[str_len - 1] == '/')
    {
        tmp_buf[str_len - 1] = '\0';
        str_len -= 1;
    }

    if (buffer != NULL)
    {
        snprintf(buffer, size, "%s", tmp_buf);
    }

    ev_free(tmp_buf);
    return str_len;
}

ssize_t ev_exepath(char* buffer, size_t size)
{
    int errcode;

    size_t tmp_size = PATH_MAX;
    char* tmp_buffer = ev_malloc(tmp_size);
    if (tmp_buffer == NULL)
    {
        errcode = EV_ENOMEM;
        goto error;
    }

    ssize_t ret = readlink("/proc/self/exe", tmp_buffer, tmp_size - 1);
    if (ret < 0)
    {
        errcode = ev__translate_sys_error(errno);
        goto error;
    }
    tmp_buffer[ret] = '\0';

    if (buffer != NULL)
    {
        ret = snprintf(buffer, size, "%s", tmp_buffer);
    }
    ev_free(tmp_buffer);

    return ret;

error:
    if (buffer != NULL && size > 0)
    {
        buffer[0] = '\0';
    }
    ev_free(tmp_buffer);
    return errcode;
}
