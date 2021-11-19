#include "loop.h"
#include <stdio.h>

static HANDLE _ev_pipe_make_s(const char* name)
{
    DWORD r_open_mode = FILE_FLAG_OVERLAPPED | WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE | PIPE_ACCESS_INBOUND;
    HANDLE pip_r = CreateNamedPipe(name, r_open_mode,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65535, 65535, 0, NULL);

    return pip_r;
}

static HANDLE _ev_pipe_make_c(const char* name)
{
    DWORD w_open_mode = GENERIC_WRITE | WRITE_DAC;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = 0;

    HANDLE pip_w = CreateFile(name, w_open_mode, 0, &sa, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    return pip_w;
}

static void _ev_pipe_on_close_win(ev_handle_t* handle)
{
    // TODO
    (void)handle;
}

int ev_pipe_make(ev_os_handle_t fds[2])
{
    static long volatile s_pipe_serial_no = 0;

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "\\\\.\\pipe\\LOCAL\\libev\\RemoteExeAnon.%08lx.%08lx",
        (long)GetCurrentProcessId(), InterlockedIncrement(&s_pipe_serial_no));

    HANDLE pip_r = _ev_pipe_make_s(buffer);
    if (pip_r == INVALID_HANDLE_VALUE)
    {
        return ev__translate_sys_error(GetLastError());
    }

    HANDLE pip_w = _ev_pipe_make_c(buffer);
    if (pip_w == INVALID_HANDLE_VALUE)
    {
        CloseHandle(pip_r);
        return ev__translate_sys_error(GetLastError());
    }

    if (!ConnectNamedPipe(pip_r, NULL))
    {
        CloseHandle(pip_r);
        CloseHandle(pip_w);
        return ev__translate_sys_error(GetLastError());
    }

    fds[0] = pip_r;
    fds[1] = pip_w;

    return EV_SUCCESS;
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe)
{
    ev__handle_init(loop, &pipe->base, _ev_pipe_on_close_win);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_HANDLE_INVALID;

    ev_list_init(&pipe->backend.stream.r_queue);
    ev_list_init(&pipe->backend.stream.r_queue_done);
    ev_list_init(&pipe->backend.stream.w_queue);
    ev_list_init(&pipe->backend.stream.w_queue_done);

    return EV_SUCCESS;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_handle_t handle)
{
    if (pipe->pipfd != EV_OS_HANDLE_INVALID)
    {
        return EV_EEXIST;
    }
    if (handle == EV_OS_HANDLE_INVALID)
    {
        return EV_EBADF;
    }

    IO_STATUS_BLOCK io_status;
    FILE_ACCESS_INFORMATION access;
    NTSTATUS nt_status = NtQueryInformationFile(handle, &io_status, &access, sizeof(access), FileAccessInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return EV_EINVAL;
    }

    DWORD mode = PIPE_READMODE_BYTE | PIPE_WAIT;
    if (!SetNamedPipeHandleState(handle, &mode, NULL, NULL))
    {
        DWORD err = GetLastError();
        if (err != ERROR_ACCESS_DENIED)
        {
            return EV_EBADF;
        }

        DWORD current_mode = 0;
        if (!GetNamedPipeHandleState(handle, &current_mode, NULL, NULL, NULL, NULL, 0))
        {
            return ev__translate_sys_error(GetLastError());
        }
        if (current_mode & PIPE_NOWAIT)
        {
            return ev__translate_sys_error(ERROR_ACCESS_DENIED);
        }
    }

    FILE_MODE_INFORMATION mode_info;
    nt_status = NtQueryInformationFile(handle, &io_status, &mode_info, sizeof(mode_info), FileModeInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return ev__translate_sys_error(GetLastError());
    }

    if (CreateIoCompletionPort(handle, pipe->base.loop->backend.iocp, (ULONG_PTR)pipe, 0) == NULL)
    {
        return ev__translate_sys_error(GetLastError());
    }

    pipe->pipfd = handle;

    return EV_SUCCESS;
}

static void _ev_pipe_on_write_iocp(ev_iocp_t* req, size_t transferred)
{

}

int ev_pipe_write(ev_pipe_t* pipe, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
    int ret;
    ret = ev__write_init_win(req, bufs, nbuf, pipe, EV_EINPROGRESS, _ev_pipe_on_write_iocp, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev_list_push_back(&pipe->backend.stream.w_queue, &req->node);
}
