#include "ev-common.h"
#include <stdlib.h>
#include <assert.h>

static char* _dup_cmd(char* const argv[])
{
    char* cmdline = malloc(MAX_PATH + 1);
    cmdline[0]    = '\0';

    strcat_s(cmdline, MAX_PATH, argv[0]);
    for (int i = 1; argv[i] != NULL; i++)
    {
        strcat_s(cmdline, MAX_PATH, " ");
        strcat_s(cmdline, MAX_PATH, argv[i]);
    }

    return cmdline;
}

static char* _dup_envp(char* const envp[])
{
    if (envp == NULL)
    {
        return NULL;
    }

    size_t malloc_size = 1;
    size_t idx = 0;

    for (idx = 0; envp[idx] != NULL; idx++)
    {
        malloc_size += strlen(envp[idx]) + 1;
    }

    char* envline = malloc(malloc_size);
    envline[malloc_size - 1] = '\0';

    size_t pos = 0;
    for (idx = 0; envp[idx] != NULL; idx++)
    {
        size_t cplen = strlen(envp[idx]) + 1;
        memcpy(envline + pos, envp[idx], cplen);
        pos += cplen;
    }

    return envline;
}

int ev_exec(ev_os_pid_t* pid, const ev_exec_opt_t* opt)
{
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb         = sizeof(STARTUPINFO);
    siStartInfo.hStdError  = INVALID_HANDLE_VALUE;
    siStartInfo.hStdOutput = INVALID_HANDLE_VALUE;
    siStartInfo.hStdInput  = INVALID_HANDLE_VALUE;

    if (opt->use_std_handles)
    {
        siStartInfo.hStdError  = opt->stdios[2];
        siStartInfo.hStdOutput = opt->stdios[1];
        siStartInfo.hStdInput  = opt->stdios[0];
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    }

    char* cmdline = _dup_cmd(opt->argv);
    char* envline = _dup_envp(opt->envp);

    BOOL ret = CreateProcess(NULL, cmdline, NULL, NULL, TRUE, 0, envline, NULL,
        &siStartInfo, &piProcInfo);

    free(cmdline);
    free(envline);

    if (!ret)
    {
        DWORD errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    CloseHandle(piProcInfo.hThread);
    *pid = piProcInfo.hProcess;

    return EV_SUCCESS;
}

int ev_waitpid(ev_os_pid_t pid, uint32_t ms, ev_process_exit_status_t* status)
{
    DWORD errcode;
    DWORD wait_time = (ms == EV_INFINITE_TIMEOUT) ? INFINITE : ms;

    DWORD ret = WaitForSingleObject(pid, wait_time);
    switch(ret)
    {
    case WAIT_TIMEOUT:
        return EV_ETIMEDOUT;

    case WAIT_OBJECT_0:
        break;

    case WAIT_FAILED:
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);

    default:
        abort();
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(pid, &exit_status))
    {
        exit_status = GetLastError();
        exit_status = ev__translate_sys_error(exit_status);
    }

    if (status != NULL)
    {
        status->exit_status = exit_status;
        status->term_signal = 0;
    }

    CloseHandle(pid);
    return EV_SUCCESS;
}


