#include "win/loop.h"
#include "win/tcp.h"
#include <stdio.h>

static char s_ev_zero[] = "";

static HANDLE _ev_pipe_make_s(const char* name)
{
    DWORD r_open_mode = FILE_FLAG_OVERLAPPED | WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE | PIPE_ACCESS_DUPLEX;
    HANDLE pip_r = CreateNamedPipe(name, r_open_mode,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65535, 65535, 0, NULL);

    return pip_r;
}

static HANDLE _ev_pipe_make_c(const char* name)
{
    DWORD w_open_mode = GENERIC_READ | GENERIC_WRITE | WRITE_DAC;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = 0;

    HANDLE pip_w = CreateFile(name, w_open_mode, 0, &sa, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    return pip_w;
}

static void _ev_pipe_cancel_all_r_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_read_t* req;
    if ((req = pipe->backend.ipc_mode.rio.reading) != NULL)
    {
        req->data.cb(req, req->data.size, stat);
        pipe->backend.ipc_mode.rio.reading = NULL;
    }
    pipe->backend.ipc_mode.rio.buf_idx = 0;
    pipe->backend.ipc_mode.rio.buf_pos = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending)) != NULL)
    {
        req = container_of(it, ev_read_t, node);
        req->data.cb(req, req->data.size, stat);
    }
}

static void _ev_pipe_cancel_all_r_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_read_t* req;
    if ((req = pipe->backend.data_mode.rio.r_doing) != NULL)
    {
        req->data.cb(req, req->data.size, stat);
        pipe->backend.data_mode.rio.r_doing = NULL;
    }

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending)) != NULL)
    {
        req = container_of(it, ev_read_t, node);
        req->data.cb(req, req->data.size, stat);
    }
}

static void _ev_pipe_cancel_all_r(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_PIPE_IPC)
    {
        _ev_pipe_cancel_all_r_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_r_data_mode(pipe, stat);
    }
}

static void _ev_pipe_cancel_all_w_data_mode(ev_pipe_t* pipe, int stat)
{
    ev_write_t* req;
    if ((req = pipe->backend.data_mode.wio.w_half) != NULL)
    {
        req->data.cb(req, req->data.size, stat);
        pipe->backend.data_mode.wio.w_half = NULL;
    }
    pipe->backend.data_mode.wio.w_half_idx = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.data_mode.wio.w_pending)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        req->data.cb(req, req->data.size, stat);
    }
}

static void _ev_pipe_cancel_all_w_ipc_mode(ev_pipe_t* pipe, int stat)
{
    ev_write_t* req;
    if ((req = pipe->backend.ipc_mode.wio.sending) != NULL)
    {
        req->data.cb(req, req->data.size, stat);
        pipe->backend.ipc_mode.wio.sending = NULL;
    }
    pipe->backend.ipc_mode.wio.buf_idx = 0;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending)) != NULL)
    {
        req = container_of(it, ev_write_t, node);
        req->data.cb(req, req->data.size, stat);
    }
}

static void _ev_pipe_cancel_all_w(ev_pipe_t* pipe, int stat)
{
    if (pipe->base.data.flags & EV_PIPE_IPC)
    {
        _ev_pipe_cancel_all_w_ipc_mode(pipe, stat);
    }
    else
    {
        _ev_pipe_cancel_all_w_data_mode(pipe, stat);
    }
}

static void _ev_pipe_on_close_win(ev_handle_t* handle)
{
    ev_pipe_t* pipe = container_of(handle, ev_pipe_t, base);

    if (pipe->close_cb != NULL)
    {
        pipe->close_cb(pipe);
    }
}

static void _ev_pipe_smart_deactive_win(ev_pipe_t* pipe)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        ev__handle_deactive(&pipe->base);
        return;
    }

    if (pipe->base.data.flags & EV_PIPE_IPC)
    {
        if (pipe->backend.ipc_mode.rio.reading != NULL
            || ev_list_size(&pipe->backend.ipc_mode.rio.pending) != 0
            || pipe->backend.ipc_mode.rio.mask.r_pending)
        {
            return;
        }
        if (pipe->backend.ipc_mode.wio.sending != NULL
            || ev_list_size(&pipe->backend.ipc_mode.wio.pending) != 0
            || pipe->backend.ipc_mode.wio.mask.w_pending)
        {
            return;
        }
    }
    else
    {
        if (pipe->backend.data_mode.rio.r_doing != NULL
            || ev_list_size(&pipe->backend.data_mode.rio.r_pending) != 0)
        {
            return;
        }
        if (pipe->backend.data_mode.wio.w_half != NULL
            || ev_list_size(&pipe->backend.data_mode.wio.w_pending) != 0)
        {
            return;
        }
    }

    ev__handle_deactive(&pipe->base);
}

/**
 * @return EV_SUCCESS: read success
 */
static int _ev_pipe_read_one_buffer(ev_pipe_t* pipe, ev_buf_t* buf, size_t* read_size)
{
    int ret = EV_SUCCESS;
    size_t pos = 0;

    while (pos < buf->size)
    {
        DWORD bytes_read = 0;
        void* read_pos = (uint8_t*)buf->data + pos;
        size_t left_size = buf->size - pos;
        if (!ReadFile(pipe->pipfd, read_pos, (DWORD)left_size, &bytes_read, NULL))
        {
            int err = GetLastError();

            if (err == ERROR_BROKEN_PIPE)
            {
                ret = EV_EOF;
                goto fin;
            }
            else if(err == ERROR_IO_PENDING)

            ret = err == ERROR_BROKEN_PIPE ? EV_EOF : ev__translate_sys_error(err);
            goto fin;
        }

        pos += bytes_read;
    }

fin:
    *read_size = pos;
    return ret;
}

static int _ev_pipe_read_into_req(HANDLE file, ev_read_t* req, size_t minimum_size,
    size_t bufidx, size_t bufpos, size_t* dbufidx, size_t* dbufpos, size_t* total)
{
    int ret = EV_SUCCESS;
    size_t total_size = 0;

    while (bufidx < req->data.nbuf && total_size < minimum_size)
    {
        ev_buf_t* buf = &req->data.bufs[bufidx];
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

static void _ev_pipe_close(ev_pipe_t* pipe)
{
    if (pipe->pipfd != EV_OS_PIPE_INVALID)
    {
        CloseHandle(pipe->pipfd);
        pipe->pipfd = EV_OS_PIPE_INVALID;
    }
}

static void _ev_pipe_cancel_all_rw(ev_pipe_t* pipe, int stat)
{
    _ev_pipe_cancel_all_r(pipe, stat);
    _ev_pipe_cancel_all_w(pipe, stat);
}

static void _ev_pipe_abort(ev_pipe_t* pipe, int stat)
{
    _ev_pipe_close(pipe);
    _ev_pipe_cancel_all_rw(pipe, stat);
    ev__handle_deactive(&pipe->base);
}

static int _ev_pipe_data_mode_want_read(ev_pipe_t* pipe)
{
    int ret = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL, &pipe->backend.data_mode.rio.io.overlapped);
    if (!ret)
    {
        int err = GetLastError();
        if (err != ERROR_IO_PENDING)
        {
            return ev__translate_sys_error(err);
        }
    }

    return EV_SUCCESS;
}

static void _ev_pipe_on_data_mode_read_recv(ev_pipe_t* pipe)
{
    int ret = EV_SUCCESS;

    ev_read_t* req;

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
            goto err;
        }
        if (avail == 0)
        {
            if ((ret = _ev_pipe_data_mode_want_read(pipe)) != EV_SUCCESS)
            {
                goto err;
            }
            break;
        }

        size_t total = 0;
        ret = _ev_pipe_read_into_req(pipe->pipfd, req, avail, bufidx, bufpos,
            &bufidx, &bufpos, &total);
        req->data.size += total;

        if (ret != EV_SUCCESS)
        {
            goto err;
        }

        if (req->data.size < req->data.capacity)
        {
            continue;
        }

        ev_list_node_t* it = ev_list_pop_front(&pipe->backend.data_mode.rio.r_pending);
        if (it == NULL)
        {
            pipe->backend.data_mode.rio.r_doing = NULL;
        }
        else
        {
            pipe->backend.data_mode.rio.r_doing = container_of(it, ev_read_t, node);
        }

        req->data.cb(req, req->data.size, EV_SUCCESS);
    }

    _ev_pipe_smart_deactive_win(pipe);
    return;

err:
    _ev_pipe_abort(pipe, ret);
    return;
}

static void _ev_pipe_on_data_mode_read(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_pipe_t* pipe = arg;

    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        int ret = ev__translate_sys_error(winsock_err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    _ev_pipe_on_data_mode_read_recv(pipe);
}

static int _ev_pipe_io_wio_submit_half(ev_pipe_t* pipe, struct ev_pipe_backend_data_mode_wio* wio)
{
    ev_write_t* half_req = pipe->backend.data_mode.wio.w_half;
    size_t half_idx = pipe->backend.data_mode.wio.w_half_idx;

    wio->w_req = half_req;
    wio->w_buf_idx = half_idx;
    int result = WriteFile(pipe->pipfd, half_req->data.bufs[half_idx].data,
        half_req->data.bufs[half_idx].size, NULL, &wio->io.overlapped);
    pipe->backend.data_mode.wio.w_half_idx++;

    /* move half record to doing list */
    if (pipe->backend.data_mode.wio.w_half_idx == half_req->data.nbuf)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &half_req->node);
        pipe->backend.data_mode.wio.w_half = NULL;
        pipe->backend.data_mode.wio.w_half_idx = 0;
    }

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
    ev_write_t* wreq = container_of(it, ev_write_t, node);

    wio->w_req = wreq;
    wio->w_buf_idx = 0;
    int result = WriteFile(pipe->pipfd, wreq->data.bufs[0].data, wreq->data.bufs[0].size,
        NULL, &wio->io.overlapped);
    int err = GetLastError();

    if (wreq->data.nbuf == 1)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &wreq->node);
    }
    else
    {
        pipe->backend.data_mode.wio.w_half = wreq;
        pipe->backend.data_mode.wio.w_half_idx = 1;
    }

    if (!result && err != ERROR_IO_PENDING)
    {
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
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
    struct ev_pipe_backend_data_mode_wio* wio = container_of(iocp, struct ev_pipe_backend_data_mode_wio, io);

    /* wio will be override, we need to backup value */
    ev_write_t* curr_req = wio->w_req;
    size_t curr_buf_idx = wio->w_buf_idx;

    /* update send size */
    curr_req->data.size += transferred;

    /* override wio with next write request */
    int submit_ret = _ev_pipe_io_wio_submit_next(pipe, wio);
    if (submit_ret != EV_SUCCESS)
    {
        pipe->backend.data_mode.wio.w_io_cnt--;
    }

    /* The last buffer */
    if (curr_buf_idx == curr_req->data.nbuf - 1)
    {
        int stat = NT_SUCCESS(iocp->overlapped.Internal) ? EV_SUCCESS :
            ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
        ev_list_erase(&pipe->backend.data_mode.wio.w_doing, &curr_req->node);
        curr_req->data.cb(curr_req, curr_req->data.size, stat);
    }

    /* If submit error, abort any pending actions */
    if (submit_ret != EV_SUCCESS && submit_ret != 1)
    {
        _ev_pipe_abort(pipe, submit_ret);
    }
    _ev_pipe_smart_deactive_win(pipe);
}

static void _ev_pipe_init_data_mode_r(ev_pipe_t* pipe, ev_iocp_cb callback)
{
    ev__iocp_init(&pipe->backend.data_mode.rio.io, callback, pipe);
    ev_list_init(&pipe->backend.data_mode.rio.r_pending);
    pipe->backend.data_mode.rio.r_doing = NULL;
}

static void _ev_pipe_init_data_mode_w(ev_pipe_t* pipe, ev_iocp_cb callback)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pipe->backend.data_mode.wio.iocp); i++)
    {
        ev__iocp_init(&pipe->backend.data_mode.wio.iocp[i].io, callback, pipe);
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

/**
 * @return boll
 */
static int _ev_pipe_check_frame_hdr(void* buffer, size_t size)
{
    ev_ipc_frame_hdr_t* hdr = buffer;
    if (size < sizeof(ev_ipc_frame_hdr_t))
    {
        return 0;
    }

    if (hdr->hdr_magic != EV_IPC_FRAME_HDR_MAGIC)
    {
        return 0;
    }

    return 1;
}

static int _ev_pipe_on_ipc_mode_read_information(ev_pipe_t* pipe, ev_ipc_frame_hdr_t* hdr)
{
    assert(hdr->hdr_exsz == sizeof(ev_pipe_win_ipc_info_t)); (void)hdr;

    void* buffer = (uint8_t*)pipe->backend.ipc_mode.rio.buffer + sizeof(ev_ipc_frame_hdr_t);
    size_t buffer_size = sizeof(pipe->backend.ipc_mode.rio.buffer) - sizeof(ev_ipc_frame_hdr_t);
    assert(buffer_size >= sizeof(ev_pipe_win_ipc_info_t));

    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, buffer_size);
    assert(ret == EV_SUCCESS); (void)ret;

    ev_pipe_win_ipc_info_t* ipc_info = buffer;

    switch (ipc_info->type)
    {
    case EV_PIPE_WIN_IPC_INFO_TYPE_STATUS:
        pipe->backend.ipc_mode.peer_pid = ipc_info->data.as_status.pid;
        break;

    case EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO:
        pipe->backend.ipc_mode.rio.reading->handle.os_socket = WSASocketW(
            FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
            &ipc_info->data.as_protocol_info, 0, WSA_FLAG_OVERLAPPED);
        if (pipe->backend.ipc_mode.rio.reading->handle.os_socket == INVALID_SOCKET)
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

static void _ev_pipe_on_ipc_mode_read_remain(ev_pipe_t* pipe)
{
    DWORD avail, errcode;
    while (pipe->backend.ipc_mode.rio.remain_size > 0
        && PeekNamedPipe(pipe->pipfd, NULL, 0, NULL, &avail, NULL) && avail > 0)
    {
        ev_read_t* req = pipe->backend.ipc_mode.rio.reading;
        if (req == NULL)
        {
            return;
        }

        size_t buf_idx = pipe->backend.ipc_mode.rio.buf_idx;
        ev_buf_t* buf = &req->data.bufs[buf_idx];
        void* buffer = (uint8_t*)buf->data + pipe->backend.ipc_mode.rio.buf_pos;

        DWORD buffer_size = buf->size - pipe->backend.ipc_mode.rio.buf_pos;
        buffer_size = EV_MIN(buffer_size, pipe->backend.ipc_mode.rio.remain_size);
        buffer_size = EV_MIN(buffer_size, avail);

        DWORD read_size;
        if (!ReadFile(pipe->pipfd, buffer, buffer_size, &read_size, NULL))
        {
            errcode = GetLastError();
            goto err;
        }

        pipe->backend.ipc_mode.rio.remain_size -= read_size;
        req->data.size += read_size;
        pipe->backend.ipc_mode.rio.buf_pos += read_size;
        if (pipe->backend.ipc_mode.rio.buf_pos < buf->size)
        {
            continue;
        }

        pipe->backend.ipc_mode.rio.buf_pos = 0;
        pipe->backend.ipc_mode.rio.buf_idx++;
        if (pipe->backend.ipc_mode.rio.buf_idx < req->data.nbuf)
        {
            continue;
        }

        req->data.cb(req, req->data.size, EV_SUCCESS);
        pipe->backend.ipc_mode.rio.buf_idx = 0;

        ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.rio.pending);
        if (it == NULL)
        {
            pipe->backend.ipc_mode.rio.reading = NULL;
            break;
        }

        pipe->backend.ipc_mode.rio.reading = container_of(it, ev_read_t, node);
    }

    return;

err:
    _ev_pipe_abort(pipe, ev__translate_sys_error(errcode));
}

static void _ev_pipe_on_ipc_mode_read_first(ev_pipe_t* pipe)
{
    /* Read */
    void* buffer = pipe->backend.ipc_mode.rio.buffer;
    int ret = _ev_pipe_read_exactly(pipe->pipfd, buffer, sizeof(ev_ipc_frame_hdr_t));
    if (ret != EV_SUCCESS)
    {
        goto err;
    }

    if (!_ev_pipe_check_frame_hdr(buffer, sizeof(ev_ipc_frame_hdr_t)))
    {
        ret = EV_EPIPE;
        goto err;
    }

    ev_ipc_frame_hdr_t* hdr = buffer;
    if (hdr->hdr_flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        if ((ret = _ev_pipe_on_ipc_mode_read_information(pipe, hdr)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    pipe->backend.ipc_mode.rio.remain_size = hdr->hdr_dtsz;
    _ev_pipe_on_ipc_mode_read_remain(pipe);

    return;

err:
    if (ret != EV_SUCCESS)
    {
        pipe->backend.ipc_mode.rio.r_err = ret;
    }
    _ev_pipe_abort(pipe, ret);
}

static void _ev_pipe_on_ipc_mode_read(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_pipe_t* pipe = arg;
    /* Clear IOCP pending flag */
    pipe->backend.ipc_mode.rio.mask.r_pending = 0;

    /* Check error */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        int ret = ev__translate_sys_error(winsock_err);
        _ev_pipe_abort(pipe, ret);

        return;
    }

    if (pipe->backend.ipc_mode.rio.remain_size != 0)
    {
        _ev_pipe_on_ipc_mode_read_remain(pipe);
    }
    else
    {
        _ev_pipe_on_ipc_mode_read_first(pipe);
    }

    _ev_pipe_smart_deactive_win(pipe);
}

/**
 * @breif Initialize buffer as #ev_ipc_frame_hdr_t
 *
 * Write sizeof(ev_ipc_frame_hdr_t) bytes
 */
static void _ev_pipe_init_ipc_frame_hdr(uint8_t* buffer, size_t bufsize, uint8_t flags, size_t dtsz)
{
    assert(bufsize >= sizeof(ev_ipc_frame_hdr_t)); (void)bufsize;

    ev_ipc_frame_hdr_t* hdr = (ev_ipc_frame_hdr_t*)buffer;

    size_t exsz = sizeof(ev_pipe_win_ipc_info_t);
    assert(exsz <= UINT16_MAX);
    assert(dtsz <= UINT32_MAX);

    hdr->hdr_magic = EV_IPC_FRAME_HDR_MAGIC;
    hdr->hdr_flags = flags;
    hdr->hdr_version = 0;
    if (flags & EV_IPC_FRAME_FLAG_INFORMATION)
    {
        hdr->hdr_exsz = (uint16_t)exsz;
    }
    else
    {
        hdr->hdr_exsz = 0;
    }

    hdr->hdr_dtsz = (uint32_t)dtsz;
    hdr->reserved = 0;
}

/**
 * @brief Send IPC data
 */
static int _ev_pipe_ipc_mode_write_data(ev_pipe_t* pipe, ev_write_t* req)
{
    uint8_t flags = 0;
    ev_pipe_win_ipc_info_t ipc_info;
    size_t hdr_size = 0;

    assert(pipe->backend.ipc_mode.wio.sending == NULL);

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
        sizeof(pipe->backend.ipc_mode.wio.buffer), flags, req->data.capacity);
    hdr_size += sizeof(ev_ipc_frame_hdr_t);

    int ret = WriteFile(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer, (DWORD)hdr_size, NULL,
        &pipe->backend.ipc_mode.wio.io.overlapped);
    if (!ret)
    {
        DWORD errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    pipe->backend.ipc_mode.wio.sending = req;
    pipe->backend.ipc_mode.wio.buf_idx = 0;

    return EV_SUCCESS;
}

static void _ev_pipe_on_ipc_mode_write(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    int ret = EV_SUCCESS;
    ev_pipe_t* pipe = arg;
    pipe->backend.ipc_mode.wio.mask.w_pending = 0;

    /* Check status */
    if (!NT_SUCCESS(iocp->overlapped.Internal))
    {
        int winsock_err = ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal);
        ret = ev__translate_sys_error(winsock_err);
        goto err;
    }

    if (pipe->backend.ipc_mode.wio.sending == NULL)
    {
        ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending);
        if (it == NULL)
        {
            return;
        }
        pipe->backend.ipc_mode.wio.sending = container_of(it, ev_write_t, node);
        pipe->backend.ipc_mode.wio.buf_idx = 0;
    }
    else if (pipe->backend.ipc_mode.wio.buf_idx != 0)
    {
        pipe->backend.ipc_mode.wio.sending->data.size += transferred;

        if (pipe->backend.ipc_mode.wio.buf_idx >= pipe->backend.ipc_mode.wio.sending->data.nbuf)
        {
            goto fin;
        }
    }

    ev_write_t* req = pipe->backend.ipc_mode.wio.sending;
    ev_buf_t* buf = &req->data.bufs[pipe->backend.ipc_mode.wio.buf_idx];
    ret = WriteFile(pipe->pipfd, buf->data, buf->size, NULL,
        &pipe->backend.ipc_mode.wio.io.overlapped);
    if (!ret)
    {
        ret = GetLastError();
        ret = ev__translate_sys_error(ret);
        goto err;
    }

    pipe->backend.ipc_mode.wio.buf_idx++;
    return;

fin:
    req = pipe->backend.ipc_mode.wio.sending;
    pipe->backend.ipc_mode.wio.sending = NULL;
    pipe->backend.ipc_mode.wio.buf_idx = 0;

    ev_list_node_t* it = ev_list_pop_front(&pipe->backend.ipc_mode.wio.pending);
    if (it != NULL)
    {
        ev_write_t* next_req = container_of(it, ev_write_t, node);
        if ((ret = _ev_pipe_ipc_mode_write_data(pipe, next_req)) != EV_SUCCESS)
        {
            next_req->data.cb(next_req, next_req->data.size, ret);
            goto err;
        }
    }

    req->data.cb(req, req->data.size, EV_SUCCESS);
    _ev_pipe_smart_deactive_win(pipe);
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
    pipe->backend.ipc_mode.peer_pid = 0;

    ev__iocp_init(&pipe->backend.ipc_mode.rio.io, _ev_pipe_on_ipc_mode_read, pipe);
    memset(&pipe->backend.ipc_mode.rio.mask, 0, sizeof(pipe->backend.ipc_mode.rio.mask));

    ev__iocp_init(&pipe->backend.ipc_mode.wio.io, _ev_pipe_on_ipc_mode_write, pipe);
    pipe->backend.ipc_mode.wio.sending = NULL;
    pipe->backend.ipc_mode.wio.buf_idx = 0;
    ev_list_init(&pipe->backend.ipc_mode.wio.pending);
}

static void _ev_pipe_init_as_data(ev_pipe_t* pipe)
{
    _ev_pipe_init_data_mode_r(pipe, _ev_pipe_on_data_mode_read);
    _ev_pipe_init_data_mode_w(pipe, _ev_pipe_on_data_mode_write);
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
    if (!WriteFile(pipe->pipfd, pipe->backend.ipc_mode.wio.buffer,
        send_size, NULL, &pipe->backend.ipc_mode.wio.io.overlapped))
    {
        int err = GetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

static int _ev_pipe_ipc_mode_want_read(ev_pipe_t* pipe)
{
    if (pipe->backend.ipc_mode.rio.mask.r_pending)
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

    pipe->backend.ipc_mode.rio.mask.r_pending = 1;
    return EV_SUCCESS;
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
static int _ev_pipe_data_mode_write(ev_pipe_t* pipe, ev_write_t* req)
{
    int result;
    int flag_failure = 0;
    DWORD err = 0;

    ev__write_init_win(req, pipe, EV_EINPROGRESS, NULL, NULL);

    size_t available_iocp_cnt = ARRAY_SIZE(pipe->backend.data_mode.wio.iocp) -
        pipe->backend.data_mode.wio.w_io_cnt;
    if (available_iocp_cnt == 0)
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_pending, &req->node);
        return EV_SUCCESS;
    }

    size_t idx;
    size_t nbuf = EV_MIN(available_iocp_cnt, req->data.nbuf);
    for (idx = 0; idx < nbuf; idx++)
    {
        size_t pos = _ev_pipe_get_and_forward_w_idx(pipe);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_req == NULL);
        assert(pipe->backend.data_mode.wio.iocp[pos].w_buf_idx == 0);

        pipe->backend.data_mode.wio.iocp[pos].w_req = req;
        pipe->backend.data_mode.wio.iocp[pos].w_buf_idx = idx;

        result = WriteFile(pipe->pipfd, req->data.bufs[idx].data, req->data.bufs[idx].size,
            NULL, &pipe->backend.data_mode.wio.iocp[pos].io.overlapped);
        /* write success */
        if (result)
        {
            continue;
        }

        /* ERROR_IO_PENDING is not a failure */
        err = GetLastError();
        if (err == ERROR_IO_PENDING)
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

    if (nbuf < req->data.nbuf)
    {
        pipe->backend.data_mode.wio.w_half = req;
        pipe->backend.data_mode.wio.w_half_idx = nbuf;
    }
    else
    {
        ev_list_push_back(&pipe->backend.data_mode.wio.w_doing, &req->node);
    }

    ev__handle_active(&pipe->base);
    return EV_SUCCESS;
}

/**
 * @brief Write in IPC mode.
 *
 * In IPC mode, we only use #ev_pipe_backend_t::w_io::iocp[0] for simplify implementation.
 */
static int _ev_pipe_ipc_mode_write(ev_pipe_t* pipe, ev_write_t* req)
{
    /* Check total send size, limited by IPC protocol */
    if (req->data.capacity > UINT32_MAX)
    {
        return EV_E2BIG;
    }

    /* If we have pending IOCP request, add it to queue */
    if (pipe->backend.ipc_mode.wio.mask.w_pending
        || pipe->backend.ipc_mode.wio.sending != NULL)
    {
        ev_list_push_back(&pipe->backend.ipc_mode.wio.pending, &req->node);
        return EV_SUCCESS;
    }

    return _ev_pipe_ipc_mode_write_data(pipe, req);
}

static int _ev_pipe_read_ipc_mode(ev_pipe_t* pipe, ev_read_t* req)
{
    if (pipe->backend.ipc_mode.rio.reading == NULL)
    {
        pipe->backend.ipc_mode.rio.reading = req;
    }
    else
    {
        ev_list_push_back(&pipe->backend.ipc_mode.rio.pending, &req->node);
    }

    return _ev_pipe_ipc_mode_want_read(pipe);
}

static int _ev_pipe_read_data_mode(ev_pipe_t* pipe, ev_read_t* req)
{
    if (pipe->backend.data_mode.rio.r_doing != NULL)
    {
        ev_list_push_back(&pipe->backend.data_mode.rio.r_pending, &req->node);
        return EV_SUCCESS;
    }

    pipe->backend.data_mode.rio.r_doing = req;
    return _ev_pipe_data_mode_want_read(pipe);
}

int ev_pipe_make(ev_os_pipe_t fds[2])
{
    static long volatile s_pipe_serial_no = 0;

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "\\\\.\\pipe\\LOCAL\\libev\\RemoteExeAnon.%08lx.%08lx",
        (long)GetCurrentProcessId(), InterlockedIncrement(&s_pipe_serial_no));

    int err;
    HANDLE pip_r = _ev_pipe_make_s(buffer);
    if (pip_r == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    HANDLE pip_w = _ev_pipe_make_c(buffer);
    if (pip_w == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        CloseHandle(pip_r);
        return ev__translate_sys_error(err);
    }

    if (!ConnectNamedPipe(pip_r, NULL))
    {
        err = GetLastError();
        if (err != ERROR_PIPE_CONNECTED)
        {
            CloseHandle(pip_r);
            CloseHandle(pip_w);
            return ev__translate_sys_error(err);
        }
    }

    fds[0] = pip_r;
    fds[1] = pip_w;

    return EV_SUCCESS;
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc)
{
    ev__handle_init(loop, &pipe->base, EV_ROLE_EV_PIPE, _ev_pipe_on_close_win);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_PIPE_INVALID;
    pipe->base.data.flags |= ipc ? EV_PIPE_IPC : 0;

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
    _ev_pipe_close(pipe);

    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle)
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
    NTSTATUS nt_status = g_ev_loop_win_ctx.NtQueryInformationFile(handle,
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
    nt_status = g_ev_loop_win_ctx.NtQueryInformationFile(handle,
        &io_status, &mode_info, sizeof(mode_info), FileModeInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return ev__translate_sys_error(GetLastError());
    }

    if (CreateIoCompletionPort(handle, pipe->base.data.loop->backend.iocp, (ULONG_PTR)pipe, 0) == NULL)
    {
        return ev__translate_sys_error(GetLastError());
    }
    pipe->pipfd = handle;

    if (!(pipe->base.data.flags & EV_PIPE_IPC))
    {
        return EV_SUCCESS;
    }

    /* In IPC mode, we need to setup communication */
    _ev_pipe_notify_status(pipe);
    _ev_pipe_ipc_mode_want_read(pipe);
    ev__handle_active(&pipe->base);

    return EV_SUCCESS;
}

int ev_pipe_write(ev_pipe_t* pipe, ev_write_t* req)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    int ret;
    if (pipe->base.data.flags & EV_PIPE_IPC)
    {
        ret = _ev_pipe_ipc_mode_write(pipe, req);
    }
    else
    {
        ret = _ev_pipe_data_mode_write(pipe, req);
    }
    
    if (ret == EV_SUCCESS)
    {
        ev__handle_active(&pipe->base);
    }

    return ret;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_read_t* req)
{
    if (pipe->pipfd == EV_OS_PIPE_INVALID)
    {
        return EV_EBADF;
    }

    ev__read_init_win(req, pipe, EV_EINPROGRESS, NULL, NULL);

    int ret;
    if (pipe->base.data.flags & EV_PIPE_IPC)
    {
        ret = _ev_pipe_read_ipc_mode(pipe, req);
    }
    else
    {
        ret = _ev_pipe_read_data_mode(pipe, req);
    }

    if (ret == EV_SUCCESS)
    {
        ev__handle_active(&pipe->base);
    }

    return ret;
}

int ev_pipe_accept(ev_pipe_t* pipe, ev_read_t* req,
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
