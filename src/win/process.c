#include "ev-common.h"
#include "async.h"
#include <stdlib.h>
#include <assert.h>

typedef struct ev_startup_info
{
    STARTUPINFO start_info;

    char*       cmdline;
    char*       envline;
}ev_startup_info_t;

typedef struct stdio_pair_s
{
    HANDLE*     dst;
    DWORD       type;
} stdio_pair_t;

static int _dup_cmd(char** buf, char* const argv[])
{
    char* cmdline = malloc(MAX_PATH + 1);
    if (cmdline == NULL)
    {
        return EV_ENOMEM;
    }

    cmdline[0]    = '\0';

    strcat_s(cmdline, MAX_PATH, argv[0]);
    for (int i = 1; argv[i] != NULL; i++)
    {
        strcat_s(cmdline, MAX_PATH, " ");
        strcat_s(cmdline, MAX_PATH, argv[i]);
    }

    *buf = cmdline;
    return EV_SUCCESS;
}

static int _dup_envp(char**buf, char* const envp[])
{
    if (envp == NULL)
    {
        *buf = NULL;
        return EV_SUCCESS;
    }

    size_t malloc_size = 1;
    size_t idx = 0;

    for (idx = 0; envp[idx] != NULL; idx++)
    {
        malloc_size += strlen(envp[idx]) + 1;
    }

    char* envline = malloc(malloc_size);
    if (envline == NULL)
    {
        return EV_ENOMEM;
    }

    envline[malloc_size - 1] = '\0';

    size_t pos = 0;
    for (idx = 0; envp[idx] != NULL; idx++)
    {
        size_t cplen = strlen(envp[idx]) + 1;
        memcpy(envline + pos, envp[idx], cplen);
        pos += cplen;
    }

    *buf = envline;
    return EV_SUCCESS;
}

static int _ev_process_setup_stdio_as_null(HANDLE* handle, DWORD dwDesiredAccess)
{
    DWORD errcode;
    HANDLE nul_file = CreateFileW(L"NUL:", dwDesiredAccess,
        FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (nul_file == INVALID_HANDLE_VALUE)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    *handle = nul_file;
    return EV_SUCCESS;
}

static int _ev_process_setup_stdio_as_fd(HANDLE* duph, HANDLE handle)
{
    *duph = handle;
    return EV_SUCCESS;
}

static int _ev_process_setup_stdio_as_pipe_win(ev_pipe_t* pipe, HANDLE* handle, int is_pipe_read)
{
    int ret;
    ev_os_pipe_t pipfd[2] = { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID };

    /* fd for #ev_pipe_t should open in nonblock mode */
    int rflags = is_pipe_read ? EV_PIPE_NONBLOCK : 0;
    int wflags = is_pipe_read ? 0 : EV_PIPE_NONBLOCK;

    if ((ret = ev_pipe_make(pipfd, rflags, wflags)) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = ev_pipe_open(pipe, is_pipe_read ? pipfd[0] : pipfd[1])) != EV_SUCCESS)
    {
        goto err;
    }

    *handle = is_pipe_read ? pipfd[1] : pipfd[0];

    return EV_SUCCESS;

err:
    ev_pipe_close(pipfd[0]);
    ev_pipe_close(pipfd[1]);
    return ret;
}

static int _ev_process_dup_stdin_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return EV_SUCCESS;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdInput, GENERIC_READ);
    }
    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdInput, container->data.fd);
    }
    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdInput, 0);
    }

    return EV_SUCCESS;
}

static int _ev_process_dup_stdout_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return EV_SUCCESS;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdOutput, GENERIC_WRITE);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdOutput, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdOutput, 1);
    }

    return EV_SUCCESS;
}

static int _ev_process_dup_stderr_win(ev_startup_info_t* info,
    const ev_process_stdio_container_t* container)
{
    if (container->flag == EV_PROCESS_STDIO_IGNORE)
    {
        return EV_SUCCESS;
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_NULL)
    {
        return _ev_process_setup_stdio_as_null(&info->start_info.hStdError, GENERIC_WRITE);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_FD)
    {
        return _ev_process_setup_stdio_as_fd(&info->start_info.hStdOutput, container->data.fd);
    }

    if (container->flag & EV_PROCESS_STDIO_REDIRECT_PIPE)
    {
        return _ev_process_setup_stdio_as_pipe_win(container->data.pipe, &info->start_info.hStdError, 1);
    }

    return EV_SUCCESS;
}

static void _ev_process_close_stdin_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdInput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdInput);
        info->start_info.hStdInput = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_close_stdout_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdOutput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdOutput);
        info->start_info.hStdOutput = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_close_stderr_win(ev_startup_info_t* info)
{
    if (info->start_info.hStdError != INVALID_HANDLE_VALUE)
    {
        CloseHandle(info->start_info.hStdError);
        info->start_info.hStdError = INVALID_HANDLE_VALUE;
    }
}

static void _ev_process_cleanup_cmdline(ev_startup_info_t* start_info)
{
    if (start_info->cmdline != NULL)
    {
        free(start_info->cmdline);
        start_info->cmdline = NULL;
    }
}

static void _ev_process_cleanup_envp(ev_startup_info_t* start_info)
{
    if (start_info->envline != NULL)
    {
        free(start_info->envline);
        start_info->envline = NULL;
    }
}

static void _ev_process_cleanup_start_info(ev_startup_info_t* start_info)
{
    _ev_process_close_stdin_win(start_info);
    _ev_process_close_stdout_win(start_info);
    _ev_process_close_stderr_win(start_info);
    _ev_process_cleanup_cmdline(start_info);
    _ev_process_cleanup_envp(start_info);
}

static int _ev_process_inherit_stdio(ev_startup_info_t* info)
{
    stdio_pair_t stdio_pair_list[] = {
        { &info->start_info.hStdInput, STD_INPUT_HANDLE },
        { &info->start_info.hStdOutput, STD_OUTPUT_HANDLE },
        { &info->start_info.hStdError, STD_ERROR_HANDLE },
    };

    BOOL dupret;
    DWORD errcode;
    HANDLE current_process = GetCurrentProcess();

    size_t i;
    for (i = 0; i < ARRAY_SIZE(stdio_pair_list); i++)
    {
        if (*(stdio_pair_list[i].dst) != INVALID_HANDLE_VALUE)
        {
            /* The stdio handle must be inherited */
            if (!SetHandleInformation(*(stdio_pair_list[i].dst), HANDLE_FLAG_INHERIT, 1))
            {
                errcode = GetLastError();
                return ev__translate_sys_error(errcode);
            }
            continue;
        }

        dupret = DuplicateHandle(current_process, GetStdHandle(stdio_pair_list[i].type),
            current_process, stdio_pair_list[i].dst, 0, TRUE, DUPLICATE_SAME_ACCESS);
        if (!dupret)
        {
            errcode = GetLastError();
            return ev__translate_sys_error(errcode);
        }
    }

    return EV_SUCCESS;
}

static int _ev_process_dup_stdio_win(ev_startup_info_t* info, const ev_process_options_t* opt)
{
    int ret;

    if ((ret = _ev_process_dup_stdin_win(info, &opt->stdios[0])) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = _ev_process_dup_stdout_win(info, &opt->stdios[1])) != EV_SUCCESS)
    {
        goto err;
    }

    if ((ret = _ev_process_dup_stderr_win(info, &opt->stdios[2])) != EV_SUCCESS)
    {
        goto err;
    }

    if (info->start_info.hStdInput == INVALID_HANDLE_VALUE
        && info->start_info.hStdOutput == INVALID_HANDLE_VALUE
        && info->start_info.hStdError == INVALID_HANDLE_VALUE)
    {
        return EV_SUCCESS;
    }

    info->start_info.dwFlags |= STARTF_USESTDHANDLES;
    if ((ret = _ev_process_inherit_stdio(info)) != EV_SUCCESS)
    {
        goto err;
    }

    return EV_SUCCESS;

err:
    _ev_process_cleanup_start_info(info);
    return ret;
}

static void _ev_process_on_async_exit(ev_async_t* async)
{
    ev_process_t* process = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    if (process->exit_cb != NULL)
    {
        process->exit_cb(process, process->exit_status, process->exit_code);
    }
}

static void _ev_process_on_sigchild_win(ev_async_t* async)
{
    ev_process_t* process = EV_CONTAINER_OF(async, ev_process_t, sigchld);

    if (process->backend.wait_handle != INVALID_HANDLE_VALUE)
    {
        if (!UnregisterWait(process->backend.wait_handle))
        {
            BREAK_ABORT();
        }
        process->backend.wait_handle = INVALID_HANDLE_VALUE;
    }

    DWORD status;
    process->exit_status = EV_PROCESS_EXIT_NORMAL;
    if (GetExitCodeProcess(process->pid, &status))
    {
        process->exit_code = status;
    }
    else
    {
        status = GetLastError();
        process->exit_code = ev__translate_sys_error(status);
    }

    ev_async_exit(async, _ev_process_on_async_exit);
}

static int _ev_process_setup_start_info(ev_startup_info_t* start_info,
    const ev_process_options_t* opt)
{
    int ret;
    ZeroMemory(start_info, sizeof(*start_info));
    start_info->start_info.cb = sizeof(start_info->start_info);
    start_info->start_info.hStdError = INVALID_HANDLE_VALUE;
    start_info->start_info.hStdOutput = INVALID_HANDLE_VALUE;
    start_info->start_info.hStdInput = INVALID_HANDLE_VALUE;

    ret = _dup_cmd(&start_info->cmdline, opt->argv);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = _dup_envp(&start_info->envline, opt->envp);
    if (ret != EV_SUCCESS)
    {
        goto err_free_cmdline;
    }

    ret = _ev_process_dup_stdio_win(start_info, opt);
    if (ret != EV_SUCCESS)
    {
        goto err_free_envp;
    }

    return EV_SUCCESS;

err_free_envp:
    _ev_process_cleanup_envp(start_info);
err_free_cmdline:
    _ev_process_cleanup_cmdline(start_info);
    return ret;
}

static VOID NTAPI _ev_process_on_object_exit(PVOID data, BOOLEAN didTimeout)
{
    ev_process_t* process = data;

    assert(didTimeout == FALSE); (void) didTimeout;
    assert(process != NULL);

    ev_async_wakeup(&process->sigchld);
}

static void _ev_process_init_win(ev_process_t* handle, const ev_process_options_t* opt)
{
    handle->exit_cb = opt->on_exit;
    handle->pid = EV_OS_PID_INVALID;
    handle->exit_status = EV_PROCESS_EXIT_UNKNOWN;
    handle->exit_code = 0;
    handle->backend.wait_handle = INVALID_HANDLE_VALUE;
}

int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle, const ev_process_options_t* opt)
{
    int ret;
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ev_startup_info_t start_info;
    ret = _ev_process_setup_start_info(&start_info, opt);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    _ev_process_init_win(handle, opt);

    ret = ev_async_init(loop, &handle->sigchld, _ev_process_on_sigchild_win);
    if (ret != EV_SUCCESS)
    {
        _ev_process_cleanup_start_info(&start_info);
        return ret;
    }

    ret = CreateProcess(NULL, start_info.cmdline, NULL, NULL, TRUE, 0,
        start_info.envline, NULL, &start_info.start_info, &piProcInfo);

    handle->pid = piProcInfo.hProcess;
    _ev_process_cleanup_start_info(&start_info);

    if (!ret)
    {
        DWORD errcode = GetLastError();
        ev__async_exit_force(&handle->sigchld);
        return ev__translate_sys_error(errcode);
    }

    ret = RegisterWaitForSingleObject(&handle->backend.wait_handle, handle->pid,
        _ev_process_on_object_exit, handle, INFINITE, WT_EXECUTEINWAITTHREAD | WT_EXECUTEONLYONCE);
    if (!ret)
    {
        BREAK_ABORT();
    }

    CloseHandle(piProcInfo.hThread);

    return EV_SUCCESS;
}
