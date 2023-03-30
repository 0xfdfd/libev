#include "ev.h"
#include "handle.h"
#include "loop_win.h"
#include "winsock.h"
#include "tcp_win.h"
#include "misc_win.h"
#include "pipe_win.h"
#include <stdio.h>
#include <assert.h>

static char s_ev_zero[] = "";

static int _ev_pipe_make_s(HANDLE* pip_handle, const char* name, int flags)
{
    DWORD r_open_mode = WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE;
    r_open_mode |= (flags & EV_PIPE_READABLE) ? PIPE_ACCESS_INBOUND : 0;
    r_open_mode |= (flags & EV_PIPE_WRITABLE) ? PIPE_ACCESS_OUTBOUND : 0;
    r_open_mode |= (flags & EV_PIPE_NONBLOCK) ? FILE_FLAG_OVERLAPPED : 0;

    HANDLE pip_r = CreateNamedPipeA(name, r_open_mode,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65535, 65535, 0, NULL);
    if (pip_r != INVALID_HANDLE_VALUE)
    {
        *pip_handle = pip_r;
        return EV_SUCCESS;
    }

    DWORD errcode = GetLastError();
    return ev__translate_sys_error(errcode);
}

static int _ev_pipe_make_c(HANDLE* pipe_handle, const char* name, int flags)
{
    DWORD w_open_mode = WRITE_DAC;
    w_open_mode |= (flags & EV_PIPE_READABLE) ? GENERIC_READ : FILE_READ_ATTRIBUTES;
    w_open_mode |= (flags & EV_PIPE_WRITABLE) ? GENERIC_WRITE : FILE_WRITE_ATTRIBUTES;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = 0;

    DWORD dwFlagsAndAttributes = (flags & EV_PIPE_NONBLOCK) ? FILE_FLAG_OVERLAPPED : 0;
    HANDLE pip_w = CreateFile(name, w_open_mode, 0, &sa, OPEN_EXISTING, dwFlagsAndAttributes, NULL);

    if (pip_w != INVALID_HANDLE_VALUE)
    {
        *pipe_handle = pip_w;
        return EV_SUCCESS;
    }

    DWORD errcode = GetLastError();
    return ev__translate_sys_error(errcode);
}

static void _ev_pipe_r_user_callback_win(ev_pipe_read_req_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe = req->backend.owner;
    ev__handle_event_dec(&pipe->base);

    ev__read_exit(&req->base);
    req->ucb(req, size, stat);
}

static void _ev_pipe_cancel_all_r_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_read_req_t* req;
    if ((req = pipe->backend.ipc_mode.rio.reading.reading) != NULL)
    {
        _ev_pipe_r_user_callback_win(req, req->base.data.size, stat);
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
    }
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_win(req, req->base.data.size, stat);
    }
}

static void _ev_pipe_cancel_all_r_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_read_req_t* req;
    if ((req = pipe->backend.data_mode.rio.r_doing) != NULL)
    {
        _ev_pipe_r_user_callback_win(req, req->base.data.size, stat);
        pipe->backend.data_mode.rio.r_doing = NULL;
    }

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
        _ev_pipe_r_user_callback_win(req, req->base.data.size, stat);
    }
}

static void _ev_pipe_cancel_all_r(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_cancel_all_r_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_r_data_mode(pipe, stat);
    }
}

static void _ev_pipe_w_user_callback_win(ev_pipe_write_req_t* req, size_t size, int stat)
{
    ev_pipe_t* pipe = req->backend.owner;
    ev__handle_event_dec(&pipe->base);

    ev__write_exit(&req->base);
    req->ucb(req, size, stat);
}

static void _ev_pipe_cancel_all_w_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_write_req_t* req;
    if ((req = pipe->backend.data_mode.wio.w_half) != NULL)
    {
        _ev_pipe_w_user_callback_win(req, req->base.size, stat);
        pipe->backend.data_mode.wio.w_half = NULL;
    }
    pipe->backend.data_mode.wio.w_half_idx = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.wio.w_pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_win(req, req->base.size, stat);
    }
}

static void _ev_pipe_cancel_all_w_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_pipe_write_req_t* req;
    if ((req = pipe->backend.ipc_mode.wio.sending.w_req) != NULL)
    {
        _ev_pipe_w_user_callback_win(req, req->base.size, stat);
        pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    }
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending)) != NULL)
    {
        req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
        _ev_pipe_w_user_callback_win(req, req->base.size, stat);
    }
}

static void _ev_pipe_cancel_all_w(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        _ev_pipe_cancel_all_w_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_w_data_mode(pipe, stat);
    }
}

static void _ev_pipe_close_pipe(ev_pipe_t* pipe)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        CloseHandle(pipe->pipfd);
        pipe->pipfd = EV_OS_PIPE_INVALID;
    }
}

/**
 * @brief Abort all pending task and close pipe.
 * The pipe is no longer usable.
 */
static void _ev_pipe_abort(ev_pipe_t* pipe, int stat)
{
    _ev_pipe_close_pipe(pipe);

    _ev_pipe_cancel_all_r(pipe, stat);
    _ev_pipe_cancel_all_w(pipe, stat);
}

static void _ev_pipe_on_close_win(ev_handle_t* handle)
{
    ev_pipe_t* pipe = EV_CONTAINER_OF(handle, ev_pipe_t, base);

    _ev_pipe_abort(pipe, EV_ECANCELED);

    if (pipe->close_cb != NULL)
    {
        pipe->close_cb(pipe);
    }
}

static int _ev_pipe_read_into_req(HANDLE file, ev_pipe_read_req_t* req, size_t minimum_size,
    size_t bufidx, size_t bufpos, size_t* dbufidx, size_t* dbufpos, size_t* total)
{
    int ret = EV_SUCCESS;
    size_t total_size = 0;

    while (bufidx < req->base.data.nbuf && total_size < minimum_size)
    {
        ev_buf_t* buf = &req->base.data.bufs[bufidx];
        void* buffer = (uint8_t*)buf->data + bufpos;
        size_t buffersize = buf->size - bufpos;

        DWORD read_size;
        if (!ReadFile(file, buffer, (DWORD)buffersize, &read_size, NULL))
        {
            int err = GetLastError();
            ret = ev__translate_sys_error(err);
            break;
        }

        total_size += read_size;
        if (read_size < buffersize)
        {
            bufpos += read_size;
            continue;
        }

        bufidx++;
        bufpos = 0;
    }

    if (dbufidx != NULL)
    {
        *dbufidx = bufidx;
    }
    if (dbufpos != NULL)
    {
        *dbufpos = bufpos;
    }
    if (total != NULL)
    {
        *total = total_size;
    }
    return ret;
}

static int _ev_pipe_data_mode_want_read(ev_pipe_t* pipe)
{
    int ret = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL,
        &pipe->backend.data_mode.rio.io.overlapped);

    if (!ret)
    {
        DWORD err = GetLastError();
        if (err != ERROR_IO_PENDING)
        {
            return ev__translate_sys_error(err);
        }
    }

    return EV_SUCCESS;
}

static void _ev_pipe_data_mode_callback_and_mount_next_win(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending);
    if (it == NULL)
    {
        pipe->backend.data_mode.rio.r_doing = NULL;
    }
    else
    {
        pipe->backend.data_mode.rio.r_doing = EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    }

    _ev_pipe_r_user_callback_win(req, req->base.data.size, EV_SUCCESS);
}

static int _ev_pipe_on_data_mode_read_recv(ev_pipe_t* pipe)
{
    int ret = EV_SUCCESS;

    ev_pipe_read_req_t* req;

    size_t bufidx = 0;
    size_t bufpos = 0;

    DWORD avail;
    while ((req = pipe->backend.data_mode.rio.r_doing) != NULL)
    {
        if (!PeekNamedPipe(pipe->pipfd, NULL, 0, NULL, &avail, NULL))
        {
            ret = GetLastError();
            if (ret == ERROR_BROKEN_PIPE)
            {
                ret = EV_EOF;
            }
            else
            {
                ret = ev__translate_sys_error(ret);
            }
            return ret;
        }

        /* no more data to read */
        if (avail == 0)
        {
            _ev_pipe_data_mode_callback_and_mount_next_win(pipe, req);
            break;
        }

        size_t total = 0;
        ret = _ev_pipe_read_into_req(pipe->pipfd, req, avail, bufidx, bufpos,
            &bufidx, &bufpos, &total);
        req->base.data.size += total;

        if (ret != EV_SUCCESS)
        {
            return ret;
        }

        if (req->base.data.size < req->base.data.capacity)
        {
            continue;
        }

        _ev_pipe_data_mode_callback_and_mount_next_win(pipe, req);
    }

    return EV_SUCCESS;
}

static int _ev_pipe_is_success_iocp_request(const ev_iocp_t* iocp)
{
    NTSTATUS status = (NTSTATUS)(iocp->overlapped.Internal);
    return NT_SUCCESS(status);
}

static DWORD _ev_pipe_get_iocp_error(const ev_iocp_t* iocp)
{
    NTSTATUS status = (NTSTATUS)(iocp->overlapped.Internal);
    return ev_winapi.RtlNtStatusToDosError(status);
}

static void _ev_pipe_on_data_mode_read_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;

    int ret;
    ev_pipe_t* pipe = arg;

    if (!_ev_pipe_is_success_iocp_request(iocp))
    {
        int err = _ev_pipe_get_iocp_error(iocp);
        ret = ev__translate_sys_error(err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    /* Do actual read */
    ret = _ev_pipe_on_data_mode_read_recv(pipe);
    if (ret != EV_SUCCESS)
    {
        _ev_pipe_abort(pipe, ret);
        return;
    }

    /* If there are pending read request, we submit another IOCP request */
    if (pipe->backend.data_mode.rio.r_doing != NULL
        || ev_list_size(&pipe->backend.data_mode.rio.r_pending) != 0)
    {
        _ev_pipe_data_mode_want_read(pipe);
        return;
    }
}

static int _ev_pipe_write_file_iocp(HANDLE file, const void* buffer, size_t size, LPOVERLAPPED iocp)
{
    memset(iocp, 0, sizeof(*iocp));
    int result = WriteFile(file, buffer, (DWORD)size, NULL, iocp);
    if (result)
    {
        return EV_SUCCESS;
    }

    int err = GetLastError();
    if (err == ERROR_IO_PENDING)
    {
        return EV_SUCCESS;
    }

    return ev__translate_sys_error(err);
}

static int _ev_pipe_io_wio_submit_half(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    ev_pipe_write_req_t* half_req = pipe->backend.data_mode.wio.w_half;
    size_t half_idx = pipe->backend.data_mode.wio.w_half_idx;

    wio->w_req = half_req;
    wio->w_buf_idx = half_idx;

    int result = _ev_pipe_write_file_iocp(pipe->pipfd, half_req->base.bufs[half_idx].data,
        half_req->base.bufs[half_idx].size, &wio->io.overlapped);
    pipe->backend.data_mode.wio.w_half_idx++;

    /* move half record to doing list */
    if (pipe->backend.data_mode.wio.w_half_idx == half_req->base.nbuf)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &half_req->base.node);
        pipe->backend.data_mode.wio.w_half = NULL;
        pipe->backend.data_mode.wio.w_half_idx = 0;
    }

    return result;
}

/**
 * @return 1: no more buffer need to send
 */
static int _ev_pipe_io_wio_submit_pending(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.data_mode.wio.w_pending);
    if (it == NULL)
    {
        return 1;
    }
    ev_pipe_write_req_t* wreq = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);

    wio->w_req = wreq;
    wio->w_buf_idx = 0;

    int result = _ev_pipe_write_file_iocp(pipe->pipfd, wreq->base.bufs[0].data,
        wreq->base.bufs[0].size, &wio->io.overlapped);

    if (wreq->base.nbuf == 1)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &wreq->base.node);
    }
    else
    {
        pipe->backend.data_mode.wio.w_half = wreq;
        pipe->backend.data_mode.wio.w_half_idx = 1;
    }

    return result;
}

/**
 * @return 0: success, 1: no more buffer need to send, any other value: failure
 */
static int _ev_pipe_io_wio_submit_next(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    if (pipe->backend.data_mode.wio.w_half != NULL)
    {
        return _ev_pipe_io_wio_submit_half(pipe, wio);
    }

    return _ev_pipe_io_wio_submit_pending(pipe, wio);
}

static void _ev_pipe_on_data_mode_write(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_pipe_t* pipe = arg;
    struct ev_pipe_backend_data_mode_wio* wio = EV_CONTAINER_OF(iocp, struct ev_pipe_backend_data_mode_wio, io);

    /* wio will be override, we need to backup value */
    ev_pipe_write_req_t* curr_req = wio->w_req;
    size_t curr_buf_idx = wio->w_buf_idx;

    /* update send size */
    curr_req->base.size += transferred;

    /* override wio with next write request */
    int submit_ret = _ev_pipe_io_wio_submit_next(pipe, wio);
    if (submit_ret != EV_SUCCESS)
    {
        pipe->backend.data_mode.wio.w_io_cnt--;
    }

    /* The last buffer */
    if (curr_buf_idx == curr_req->base.nbuf - 1)
    {
        int stat = NT_SUCCESS(iocp->overlapped.Internal) ? EV_SUCCESS :
            ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
        ev_list_erase(&pipe->backend.data_mode.wio.w_doing, &curr_req->base.node);
        _ev_pipe_w_user_callback_win(curr_req, curr_req->base.size, stat);
    }

    /* If submit error, abort any pending actions */
    if (submit_ret != EV_SUCCESS && submit_ret != 1)
    {
        _ev_pipe_abort(pipe, submit_ret);
    }
}

static void _ev_pipe_init_data_mode_r(ev_pipe_t* pipe)
{
    ev__iocp_init(&pipe->backend.data_mode.rio.io, _ev_pipe_on_data_mode_read_win, pipe);
    ev_list_init(&pipe->backend.data_mode.rio.r_pending);
    pipe->backend.data_mode.rio.r_doing = NULL;
}

static void _ev_pipe_init_data_mode_w(ev_pipe_t* pipe)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pipe->backend.data_mode.wio.iocp); i++)
    {
        ev__iocp_init(&pipe->backend.data_mode.wio.iocp[i].io, _ev_pipe_on_data_mode_write, pipe);
        pipe->backend.data_mode.wio.iocp[i].idx = i;
        pipe->backend.data_mode.wio.iocp[i].w_req = NULL;
        pipe->backend.data_mode.wio.iocp[i].w_buf_idx = 0;
    }
    pipe->backend.data_mode.wio.w_io_idx = 0;
    pipe->backend.data_mode.wio.w_io_cnt = 0;

    ev_list_init(&pipe->backend.data_mode.wio.w_pending);
    ev_list_init(&pipe->backend.data_mode.wio.w_doing);
    pipe->backend.data_mode.wio.w_half = NULL;
    pipe->backend.data_mode.wio.w_half_idx = 0;
}

static int _ev_pipe_read_exactly(HANDLE file, void* buffer, size_t size)
{
    int err;
    DWORD bytes_read, bytes_read_now;

    bytes_read = 0;
    while (bytes_read < size)
    {
        if (!ReadFile(file, (char*)buffer + bytes_read, (DWORD)size - bytes_read, &bytes_read_now, NULL))
        {
            err = GetLastError();
            return ev__translate_sys_error(err);
        }

        bytes_read += bytes_read_now;
    }

    assert(bytes_read == size);
    return EV_SUCCESS;
}

static ev_pipe_read_req_t* _ev_pipe_on_ipc_mode_read_mount_next(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending);
    if (it == NULL)
    {
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
    }
    else
    {
        pipe->backend.ipc_mode.rio.reading.reading =
            EV_CONTAINER_OF(it, ev_pipe_read_req_t, base.node);
    }

    return pipe->backend.ipc_mode.rio.reading.reading;
}

static int _ev_pipe_on_ipc_mode_read_information(ev_pipe_t* pipe, ev_ipc_frame_hdr_t* hdr)
{
    if (hdr->hdr_exsz != sizeof(ev_pipe_win_ipc_info_t))
    {
        return EV_EPROTO;
    }

    void* buffer = (uint8_t*)pipe->backend.ipc_mode.rio.buffer + sizeof(ev_ipc_frame_hdr_t);
    size_t buffer_size = sizeof(pipe->backend.ipc_mode.rio.buffer) - sizeof(ev_ipc_frame_hdr_t);
    assert(buffer_size >= sizeof(ev_pipe_win_ipc_info_t));

    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, buffer_size);
    assert(ret == EV_SUCCESS); (void)ret;

    ev_pipe_read_req_t* req;
    ev_pipe_win_ipc_info_t* ipc_info = buffer;

    switch (ipc_info->type)
    {
    case EV_PIPE_WIN_IPC_INFO_TYPE_STATUS:
        pipe->backend.ipc_mode.peer_pid = ipc_info->data.as_status.pid;
        break;

    case EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO:
        if ((req = pipe->backend.ipc_mode.rio.reading.reading) == NULL)
        {
            req = _ev_pipe_on_ipc_mode_read_mount_next(pipe);
        }
        assert(req != NULL);
        req->handle.os_socket = WSASocketW(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
            FROM_PROTOCOL_INFO, &ipc_info->data.as_protocol_info, 0, WSA_FLAG_OVERLAPPED);
        if (req->handle.os_socket == INVALID_SOCKET)
        {
            int errcode = WSAGetLastError();
            return ev__translate_sys_error(errcode);
        }
        break;

    default:
        abort();
        break;
    }

    return EV_SUCCESS;
}

static int _ev_pipe_on_ipc_mode_read_remain(ev_pipe_t* pipe)
{
    DWORD avail, errcode;
    while (pipe->backend.ipc_mode.rio.remain_size > 0
        && PeekNamedPipe(pipe->pipfd, NULL, 0, NULL, &avail, NULL) && avail > 0)
    {
        ev_pipe_read_req_t* req = pipe->backend.ipc_mode.rio.reading.reading;
        if (req == NULL)
        {
            req = _ev_pipe_on_ipc_mode_read_mount_next(pipe);
            if (req == NULL)
            {
                return EV_SUCCESS;
            }
        }

        size_t buf_idx = pipe->backend.ipc_mode.rio.reading.buf_idx;
        ev_buf_t* buf = &req->base.data.bufs[buf_idx];
        void* buffer = (uint8_t*)buf->data + pipe->backend.ipc_mode.rio.reading.buf_pos;

        DWORD buffer_size = buf->size - pipe->backend.ipc_mode.rio.reading.buf_pos;
        buffer_size = EV_MIN(buffer_size, pipe->backend.ipc_mode.rio.remain_size);
        buffer_size = EV_MIN(buffer_size, avail);

        DWORD read_size;
        if (!ReadFile(pipe->pipfd, buffer, buffer_size, &read_size, NULL))
        {
            errcode = GetLastError();
            goto err;
        }
        pipe->backend.ipc_mode.rio.remain_size -= read_size;
        pipe->backend.ipc_mode.rio.reading.buf_pos += read_size;
        req->base.data.size += read_size;

        /* Read the whole frame */
        if (pipe->backend.ipc_mode.rio.remain_size == 0)
        {
            pipe->backend.ipc_mode.rio.reading.reading = NULL;
            _ev_pipe_r_user_callback_win(req, req->base.data.size, EV_SUCCESS);
            continue;
        }

        /* Remain data to read */
        if (pipe->backend.ipc_mode.rio.reading.buf_pos < buf->size)
        {
            continue;
        }

        /* Move to next buffer */
        pipe->backend.ipc_mode.rio.reading.buf_pos = 0;
        pipe->backend.ipc_mode.rio.reading.buf_idx++;
        if (pipe->backend.ipc_mode.rio.reading.buf_idx < req->base.data.nbuf)
        {
            continue;
        }

        /* Buffer is full, need to notify user */
        pipe->backend.ipc_mode.rio.reading.reading = NULL;
        _ev_pipe_r_user_callback_win(req, req->base.data.size, EV_SUCCESS);
    }

    return EV_SUCCESS;

err:
    return ev__translate_sys_error(errcode);
}

static int _ev_pipe_on_ipc_mode_read_first(ev_pipe_t* pipe)
{
    /* Read */
    void* buffer = pipe->backend.ipc_mode.rio.buffer;
    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, sizeof(ev_ipc_frame_hdr_t));
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    if (!ev__ipc_check_frame_hdr(buffer, sizeof(ev_ipc_frame_hdr_t)))
    {
        return EV_EPROTO;
    }

    ev_ipc_frame_hdr_t* hdr = buffer;
    if (hdr->hdr_flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        if ((ret = _ev_pipe_on_ipc_mode_read_information(pipe, hdr)) != EV_SUCCESS)
        {
            return ret;
        }
    }

    pipe->backend.ipc_mode.rio.remain_size = hdr->hdr_dtsz;
    return _ev_pipe_on_ipc_mode_read_remain(pipe);
}

static int _ev_pipe_ipc_mode_want_read(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.rio.mask.rio_pending)
    {
        return EV_SUCCESS;
    }

    int result = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL, &pipe->backend.ipc_mode.rio.io.overlapped);
    if (!result)
    {
        int err = GetLastError();
        if (err != ERROR_IO_PENDING)
        {
            return ev__translate_sys_error(err);
        }
    }

    pipe->backend.ipc_mode.rio.mask.rio_pending = 1;
    return EV_SUCCESS;
}

static void _ev_pipe_on_ipc_mode_read(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_pipe_t* pipe = arg;
    /* Clear IOCP pending flag */
    pipe->backend.ipc_mode.rio.mask.rio_pending = 0;

    /* Check error */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        int ret = ev__translate_sys_error(winsock_err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    int ret;
    if (pipe->backend.ipc_mode.rio.remain_size != 0)
    {
        ret = _ev_pipe_on_ipc_mode_read_remain(pipe);
    }
    else
    {
        ret = _ev_pipe_on_ipc_mode_read_first(pipe);
    }

    if (ret != EV_SUCCESS)
    {
        _ev_pipe_abort(pipe, ret);
        return;
    }

    if (pipe->backend.ipc_mode.rio.reading.reading != NULL
        || ev_list_size(&pipe->backend.ipc_mode.rio.pending))
    {
        _ev_pipe_ipc_mode_want_read(pipe);
        return;
    }
}

/**
 * @breif Initialize buffer as #ev_ipc_frame_hdr_t
 *
 * Write sizeof(ev_ipc_frame_hdr_t) bytes
 */
static void _ev_pipe_init_ipc_frame_hdr(uint8_t* buffer, size_t bufsize, uint8_t flags, uint32_t dtsz)
{
    assert(bufsize >= sizeof(ev_ipc_frame_hdr_t)); (void)bufsize;

    uint16_t exsz = 0;
    if (flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        exsz = sizeof(ev_pipe_win_ipc_info_t);
    }

    ev__ipc_init_frame_hdr((ev_ipc_frame_hdr_t*)buffer, flags, exsz, dtsz);
}

/**
 * @brief Send IPC data
 */
static int _ev_pipe_ipc_mode_write_data(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    uint8_t flags = 0;
    ev_pipe_win_ipc_info_t ipc_info;
    size_t hdr_size = 0;

    assert(pipe->backend.ipc_mode.wio.sending.w_req == NULL);

    if (req->handle.role != EV_ROLE_UNKNOWN)
    {
        flags |= EV_IPC_FRAME_FLAG_INFORMATION;

        DWORD target_pid = pipe->backend.ipc_mode.peer_pid;
        if (target_pid == EV_INVALID_PID_WIN)
        {
            return EV_EAGAIN;
        }

        memset(&ipc_info, 0, sizeof(ipc_info));
        ipc_info.type = EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO;

        if (WSADuplicateSocketW(req->handle.u.os_socket, target_pid, &ipc_info.data.as_protocol_info))
        {
            int err = WSAGetLastError();
            return ev__translate_sys_error(err);
        }

        void* buffer = (uint8_t*)pipe->backend.ipc_mode.wio.buffer + sizeof(ev_ipc_frame_hdr_t);
        memcpy(buffer, &ipc_info, sizeof(ipc_info));
        hdr_size += sizeof(ipc_info);
    }

    _ev_pipe_init_ipc_frame_hdr(pipe->backend.ipc_mode.wio.buffer,
        sizeof(pipe->backend.ipc_mode.wio.buffer), flags, (uint32_t)req->base.capacity);
    hdr_size += sizeof(ev_ipc_frame_hdr_t);

    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
        hdr_size, &pipe->backend.ipc_mode.wio.io.overlapped);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    pipe->backend.ipc_mode.wio.sending.w_req = req;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    return EV_SUCCESS;
}

static int _ev_pipe_ipc_mode_send_next(ev_pipe_t* pipe)
{
    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending);
    if (it == NULL)
    {
        return EV_SUCCESS;
    }

    ev_pipe_write_req_t* next_req = EV_CONTAINER_OF(it, ev_pipe_write_req_t, base.node);
    int ret = _ev_pipe_ipc_mode_write_data(pipe, next_req);
    if (ret != EV_SUCCESS)
    {
        ev_list_push_front(&pipe->backend.ipc_mode.wio.pending, it);
    }
    return ret;
}

static int _ev_pipe_on_ipc_mode_write_process(ev_pipe_t* pipe, size_t transferred)
{
    if (pipe->backend.ipc_mode.wio.sending.w_req == NULL)
    {/* This is a builtin status notify */
        return _ev_pipe_ipc_mode_send_next(pipe);
    }

    pipe->backend.ipc_mode.wio.sending.donecnt++;

    if (pipe->backend.ipc_mode.wio.sending.donecnt == 1)
    {/* Frame header send success */
        /* Do nothing */
    }
    else
    {
        pipe->backend.ipc_mode.wio.sending.w_req->base.size += transferred;

        if (pipe->backend.ipc_mode.wio.sending.donecnt > pipe->backend.ipc_mode.wio.sending.w_req->base.nbuf)
        {
            goto finish_request;
        }
    }

    size_t send_buf_idx = pipe->backend.ipc_mode.wio.sending.donecnt - 1;
    ev_pipe_write_req_t* req = pipe->backend.ipc_mode.wio.sending.w_req;
    ev_buf_t* buf = &req->base.bufs[send_buf_idx];

    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, buf->data, buf->size,
        &pipe->backend.ipc_mode.wio.io.overlapped);
    return ret;

finish_request:
    req = pipe->backend.ipc_mode.wio.sending.w_req;
    pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;

    _ev_pipe_w_user_callback_win(req, req->base.size, EV_SUCCESS);

    if ((ret = _ev_pipe_ipc_mode_send_next(pipe)) != EV_SUCCESS)
    {
        return ret;
    }

    return EV_SUCCESS;
}

static void _ev_pipe_on_ipc_mode_write(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    int ret = EV_SUCCESS;
    ev_pipe_t* pipe = arg;
    pipe->backend.ipc_mode.wio.mask.iocp_pending = 0;

    /* Check status */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        ret = ev__translate_sys_error(winsock_err);
        goto err;
    }

    if ((ret = _ev_pipe_on_ipc_mode_write_process(pipe, transferred)) != EV_SUCCESS)
    {
        goto err;
    }
    return;

err:
    if (ret != EV_SUCCESS)
    {
        pipe->backend.ipc_mode.wio.w_err = ret;
    }
    _ev_pipe_abort(pipe, ret);
}

static void _ev_pipe_init_as_ipc(ev_pipe_t* pipe)
{
    pipe->backend.ipc_mode.iner_err = EV_SUCCESS;
    pipe->backend.ipc_mode.peer_pid = 0;

    /* rio */
    memset(&pipe->backend.ipc_mode.rio.mask, 0, sizeof(pipe->backend.ipc_mode.rio.mask));
    pipe->backend.ipc_mode.rio.reading.reading = NULL;
    pipe->backend.ipc_mode.rio.reading.buf_idx = 0;
    pipe->backend.ipc_mode.rio.reading.buf_pos = 0;
    ev_list_init(&pipe->backend.ipc_mode.rio.pending);
    pipe->backend.ipc_mode.rio.r_err = 0;
    pipe->backend.ipc_mode.rio.remain_size = 0;
    ev__iocp_init(&pipe->backend.ipc_mode.rio.io, _ev_pipe_on_ipc_mode_read, pipe);

    /* wio */
    memset(&pipe->backend.ipc_mode.wio.mask, 0, sizeof(pipe->backend.ipc_mode.wio.mask));
    pipe->backend.ipc_mode.wio.sending.w_req = NULL;
    pipe->backend.ipc_mode.wio.sending.donecnt = 0;
    ev_list_init(&pipe->backend.ipc_mode.wio.pending);
    pipe->backend.ipc_mode.wio.w_err = 0;
    ev__iocp_init(&pipe->backend.ipc_mode.wio.io, _ev_pipe_on_ipc_mode_write, pipe);
}

static void _ev_pipe_init_as_data(ev_pipe_t* pipe)
{
    _ev_pipe_init_data_mode_r(pipe);
    _ev_pipe_init_data_mode_w(pipe);
}

static int _ev_pipe_notify_status(ev_pipe_t* pipe)
{
    _ev_pipe_init_ipc_frame_hdr(pipe->backend.ipc_mode.wio.buffer,
        sizeof(pipe->backend.ipc_mode.wio.buffer), EV_IPC_FRAME_FLAG_INFORMATION, 0);

    ev_pipe_win_ipc_info_t ipc_info;
    memset(&ipc_info, 0, sizeof(ev_pipe_win_ipc_info_t));
    ipc_info.type = EV_PIPE_WIN_IPC_INFO_TYPE_STATUS;
    ipc_info.data.as_status.pid = GetCurrentProcessId();
    memcpy(pipe->backend.ipc_mode.wio.buffer + sizeof(ev_ipc_frame_hdr_t), &ipc_info, sizeof(ipc_info));

    DWORD send_size = sizeof(ev_ipc_frame_hdr_t) + sizeof(ev_pipe_win_ipc_info_t);

    pipe->backend.ipc_mode.wio.mask.iocp_pending = 1;
    int ret = _ev_pipe_write_file_iocp(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
        send_size, &pipe->backend.ipc_mode.wio.io.overlapped);
    return ret;
}

static size_t _ev_pipe_get_and_forward_w_idx(ev_pipe_t* pipe)
{
    size_t ret = pipe->backend.data_mode.wio.w_io_idx;
    if (pipe->backend.data_mode.wio.w_io_idx == ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) - 1)
    {
        pipe->backend.data_mode.wio.w_io_idx = 0;
    }
    else
    {
        pipe->backend.data_mode.wio.w_io_idx++;
    }
    pipe->backend.data_mode.wio.w_io_cnt++;

    return ret;
}

static size_t _ev_pipe_revert_w_idx(ev_pipe_t* pipe)
{
    if (pipe->backend.data_mode.wio.w_io_idx == 0)
    {
        pipe->backend.data_mode.wio.w_io_idx = ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) - 1;
    }
    else
    {
        pipe->backend.data_mode.wio.w_io_idx--;
    }
    pipe->backend.data_mode.wio.w_io_cnt--;

    return pipe->backend.data_mode.wio.w_io_idx;
}

/**
 * @brief Write in DATA mode.
 * 
 * In DATA mode, every #ev_pipe_backend_t::w_io::iocp can be used to provide maximum
 * performance.
 */
static int _ev_pipe_data_mode_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    int result;
    int flag_failure = 0;
    DWORD err = 0;

    req->backend.owner = pipe;
    req->backend.stat = EV_EINPROGRESS;

    size_t available_iocp_cnt = ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) -
        pipe->backend.data_mode.wio.w_io_cnt;
    if (available_iocp_cnt == 0)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_pending, &req->base.node);
        return EV_SUCCESS;
    }

    size_t idx;
    size_t nbuf = EV_MIN(available_iocp_cnt, req->base.nbuf);
    for (idx = 0; idx < nbuf; idx++)
    {
        size_t pos = _ev_pipe_get_and_forward_w_idx(pipe);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_req == NULL);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_buf_idx == 0);

        pipe->backend.data_mode.wio.iocp[pos].w_req = req;
        pipe->backend.data_mode.wio.iocp[pos].w_buf_idx = idx;

        result = _ev_pipe_write_file_iocp(pipe->pipfd, req->base.bufs[idx].data,
            req->base.bufs[idx].size, &pipe->backend.data_mode.wio.iocp[pos].io.overlapped);
        /* write success */
        if (result == EV_SUCCESS)
        {
            continue;
        }

        flag_failure = 1;
        break;
    }

    if (flag_failure)
    {
        size_t i;
        for (i = 0; i <= idx; i++)
        {
            size_t pos = _ev_pipe_revert_w_idx(pipe);
            CancelIoEx(pipe->pipfd, &pipe->backend.data_mode.wio.iocp[pos].io.overlapped);
            pipe->backend.data_mode.wio.iocp[pos].w_req = NULL;
            pipe->backend.data_mode.wio.iocp[pos].w_buf_idx = 0;
        }
        return ev__translate_sys_error(err);
    }

    if (nbuf < req->base.nbuf)
    {
        pipe->backend.data_mode.wio.w_half = req;
        pipe->backend.data_mode.wio.w_half_idx = nbuf;
    }
    else
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &req->base.node);
    }

    ev__handle_event_add(&pipe->base);
    return EV_SUCCESS;
}

/**
 * @brief Write in IPC mode.
 *
 * In IPC mode, we only use #ev_pipe_backend_t::w_io::iocp[0] for simplify implementation.
 */
static int _ev_pipe_ipc_mode_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req)
{
    if (pipe->backend.ipc_mode.iner_err != EV_SUCCESS)
    {
        return pipe->backend.ipc_mode.iner_err;
    }

    /* Check total send size, limited by IPC protocol */
    if (req->base.capacity > UINT32_MAX)
    {
        return EV_E2BIG;
    }

    /* If we have pending IOCP request, add it to queue */
    if (pipe->backend.ipc_mode.wio.mask.iocp_pending
        || pipe->backend.ipc_mode.wio.sending.w_req != NULL)
    {
        ev_list_push_back(&pipe->backend.ipc_mode.wio.pending, &req->base.node);
        return EV_SUCCESS;
    }

    return _ev_pipe_ipc_mode_write_data(pipe, req);
}

static int _ev_pipe_read_ipc_mode(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    int ret;
    if (pipe->backend.ipc_mode.iner_err != EV_SUCCESS)
    {
        return pipe->backend.ipc_mode.iner_err;
    }

    ev_list_push_back(&pipe->backend.ipc_mode.rio.pending, &req->base.node);

    if ((ret = _ev_pipe_ipc_mode_want_read(pipe)) != EV_SUCCESS)
    {
        ev_list_erase(&pipe->backend.ipc_mode.rio.pending, &req->base.node);
    }

    return ret;
}

static int _ev_pipe_read_data_mode(ev_pipe_t* pipe, ev_pipe_read_req_t* req)
{
    if (pipe->backend.data_mode.rio.r_doing != NULL)
    {
        ev_list_push_back(&pipe->backend.data_mode.rio.r_pending, &req->base.node);
        return EV_SUCCESS;
    }

    pipe->backend.data_mode.rio.r_doing = req;
    return _ev_pipe_data_mode_want_read(pipe);
}

static int _ev_pipe_init_read_token_win(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_pipe_read_cb cb)
{
    int ret;

    if ((ret = ev__pipe_read_init(req, bufs, nbuf, cb)) != EV_SUCCESS)
    {
        return ret;
    }

    req->backend.owner = pipe;
    req->backend.stat = EV_EINPROGRESS;

    return EV_SUCCESS;
}

static int _ev_pipe_make_win(ev_os_pipe_t fds[2], int rflags, int wflags,
        const char *name)
{
    int err;
    HANDLE pip_r = INVALID_HANDLE_VALUE;
    HANDLE pip_w = INVALID_HANDLE_VALUE;

    err = _ev_pipe_make_s(&pip_r, name, rflags);
    if (err != EV_SUCCESS)
    {
        goto err_close_rw;
    }

    err = _ev_pipe_make_c(&pip_w, name, wflags);
    if (pip_w == INVALID_HANDLE_VALUE)
    {
        goto err_close_rw;
    }

    if (!ConnectNamedPipe(pip_r, NULL))
    {
        err = GetLastError();
        if (err != ERROR_PIPE_CONNECTED)
        {
            err = ev__translate_sys_error(err);
            goto err_close_rw;
        }
    }

    fds[0] = pip_r;
    fds[1] = pip_w;

    return EV_SUCCESS;

err_close_rw:
    if (pip_r != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pip_r);
    }
    if (pip_w != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pip_w);
    }
    return err;
}

static int _ev_pipe_open_check_win(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        return EV_EEXIST;
    }
    if (handle == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    IO_STATUS_BLOCK io_status;
    FILE_ACCESS_INFORMATION access;
    NTSTATUS nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status, &access, sizeof(access), FileAccessInformation);
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
    nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status, &mode_info, sizeof(mode_info), FileModeInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return ev__translate_sys_error(GetLastError());
    }

    return EV_SUCCESS;
}

int ev_pipe_make(ev_os_pipe_t fds[2], int rflags, int wflags)
{
    static long volatile s_pipe_serial_no = 0;
    char buffer[128];

    fds[0] = EV_OS_PIPE_INVALID;
    fds[1] = EV_OS_PIPE_INVALID;
    if ((rflags & EV_PIPE_IPC) != (wflags & EV_PIPE_IPC))
    {
        return EV_EINVAL;
    }

    snprintf(buffer, sizeof(buffer), "\\\\.\\pipe\\LOCAL\\libev\\RemoteExeAnon.%08lx.%08lx",
        (long)GetCurrentProcessId(), InterlockedIncrement(&s_pipe_serial_no));

    rflags |= EV_PIPE_READABLE;
    wflags |= EV_PIPE_WRITABLE;

    int is_ipc = rflags & EV_PIPE_IPC;
    if (is_ipc)
    {
        rflags |= EV_PIPE_WRITABLE;
        wflags |= EV_PIPE_READABLE;
    }
    else
    {
        rflags &= ~EV_PIPE_WRITABLE;
        wflags &= ~EV_PIPE_READABLE;
    }

    return _ev_pipe_make_win(fds, rflags, wflags, buffer);
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;
    pipe->base.data.flags |= ipc ? EV_HANDLE_PIPE_IPC : 0;

    if (ipc)
    {
        _ev_pipe_init_as_ipc(pipe);
    }
    else
    {
        _ev_pipe_init_as_data(pipe);
    }

    return EV_SUCCESS;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    _ev_pipe_close_pipe(pipe);

    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base, _ev_pipe_on_close_win);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
{
    int ret;

    if ((ret = _ev_pipe_open_check_win(pipe, handle)) != EV_SUCCESS)
    {
        return ret;
    }

    if (CreateIoCompletionPort(handle, pipe->base.loop->backend.iocp, (ULONG_PTR)pipe, 0) == NULL)
    {
        return ev__translate_sys_error(GetLastError());
    }
    pipe->pipfd = handle;

    if (!(pipe->base.data.flags & EV_HANDLE_PIPE_IPC))
    {
        return EV_SUCCESS;
    }

    /**
     * TODO:
     * In IPC mode, we need to setup communication.
     * 
     * Here we may have problem that if the pipe is read-only / write-only, we
     * cannot unbind IOCP beacuse windows not support that.
     * 
     * There are may ways to avoid it:
     * 1. Avoid handeshake procedure. We need handeshake because we need child
     *   process information to call DuplicateHandle(). But accroding to
     *   https://stackoverflow.com/questions/46348163/how-to-transfer-the-duplicated-handle-to-the-child-process
     *   we can call DuplicateHandle() in child process, as long as we wait for
     *   peer response.
     * 2. If handle is not readable, we return success but mark it as error, and
     *   notify user error in future operation.
     */
    if ((ret = _ev_pipe_notify_status(pipe)) != EV_SUCCESS)
    {
        pipe->backend.ipc_mode.iner_err = ret;
        return EV_SUCCESS;
    }

    if ((ret = _ev_pipe_ipc_mode_want_read(pipe)) != EV_SUCCESS)
    {
        pipe->backend.ipc_mode.iner_err = ret;
        return EV_SUCCESS;
    }

    return EV_SUCCESS;
}

int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size,
    ev_pipe_write_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = ev__pipe_write_init_ext(req, cb, bufs, nbuf, handle_role, handle_addr, handle_size);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    req->backend.owner = pipe;
    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_ipc_mode_write(pipe, req);
    }
    else
    {
        ret = _ev_pipe_data_mode_write(pipe, req);
    }

    if (ret != EV_SUCCESS)
    {
        ev__write_exit(&req->base);
        ev__handle_event_dec(&pipe->base);
    }

    return ret;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_read_cb cb)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret = _ev_pipe_init_read_token_win(pipe, req, bufs, nbuf, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_event_add(&pipe->base);

    if (pipe->base.data.flags & EV_HANDLE_PIPE_IPC)
    {
        ret = _ev_pipe_read_ipc_mode(pipe, req);
    }
    else
    {
        ret = _ev_pipe_read_data_mode(pipe, req);
    }

    if (ret != EV_SUCCESS)
    {
        ev__handle_event_dec(&pipe->base);
        ev__read_exit(&req->base);
    }

    return ret;
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size)
{
    (void)pipe;
    if (req->handle.os_socket == EV_OS_SOCKET_INVALID)
    {
        return EV_ENOENT;
    }

    if (handle_role != EV_ROLE_EV_TCP
        || handle_addr == NULL
        || handle_size != sizeof(ev_tcp_t))
    {
        return EV_EINVAL;
    }

    int ret = ev__tcp_open_win((ev_tcp_t*)handle_addr, req->handle.os_socket);
    req->handle.os_socket = EV_OS_SOCKET_INVALID;

    return ret;
}

void ev_pipe_close(ev_os_pipe_t fd)
{
    CloseHandle(fd);
}
