#include <assert.h>

ev_loop_win_ctx_t g_ev_loop_win_ctx;

static void _ev_pool_win_handle_req(OVERLAPPED_ENTRY* overlappeds, ULONG count)
{
    ULONG i;
    for (i = 0; i < count; i++)
    {
        if (overlappeds[i].lpOverlapped)
        {
            ev_iocp_t* req = EV_CONTAINER_OF(overlappeds[i].lpOverlapped, ev_iocp_t, overlapped);
            req->cb(req, overlappeds[i].dwNumberOfBytesTransferred, req->arg);
        }
    }
}

static void _ev_check_layout_win(void)
{
    ENSURE_LAYOUT(ev_buf_t, size, data, WSABUF, len, buf);
}

static void _ev_init_once_win(void)
{
    g_ev_loop_win_ctx.net.zero_[0] = '\0';

    _ev_check_layout_win();
    ev__winsock_init();
    ev__winapi_init();
    ev__time_init_win();
}

EV_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
    int repeat;
    BOOL success;
    ULONG count;
    DWORD errcode;
    OVERLAPPED_ENTRY overlappeds[128];

    uint64_t timeout_time = loop->hwtime + timeout;

    for (repeat = 0;; repeat++)
    {
        success = GetQueuedCompletionStatusEx(loop->backend.iocp, overlappeds,
            ARRAY_SIZE(overlappeds), &count, timeout, FALSE);

        /* If success, handle all IOCP request */
        if (success)
        {
            _ev_pool_win_handle_req(overlappeds, count);
            return;
        }

        /* Cannot handle any other error */
        errcode = GetLastError();
        if (errcode != WAIT_TIMEOUT)
        {
            EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
        }

        if (timeout == 0)
        {
            return;
        }

        /**
         * GetQueuedCompletionStatusEx() can occasionally return a little early.
         * Make sure that the desired timeout target time is reached.
         */
        ev__loop_update_time(loop);

        if (timeout_time <= loop->hwtime)
        {
            break;
        }

        timeout = (uint32_t)(timeout_time - loop->hwtime);
        timeout += repeat ? (1U << (repeat - 1)) : 0;
    }
}

EV_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg)
{
    req->cb = callback;
    req->arg = arg;
    memset(&req->overlapped, 0, sizeof(req->overlapped));
}

EV_LOCAL void ev__loop_exit_backend(ev_loop_t* loop)
{
    ev__threadpool_exit_win(loop);

    if (loop->backend.iocp != NULL)
    {
        CloseHandle(loop->backend.iocp);
        loop->backend.iocp = NULL;
    }
}

EV_LOCAL void ev__init_once_win(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_win);
}

EV_LOCAL int ev__loop_init_backend(ev_loop_t* loop)
{
    ev__init_once_win();

    loop->backend.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (loop->backend.iocp == NULL)
    {
        int err = GetLastError();
        return ev__translate_sys_error(err);
    }

    ev__threadpool_init_win(loop);

    return 0;
}

EV_LOCAL void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req)
{
    DWORD errcode;
    if (!PostQueuedCompletionStatus(loop->backend.iocp, 0, 0, &req->overlapped))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

EV_LOCAL int ev__reuse_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    int err;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen) != 0)
    {
        err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}

EV_LOCAL int ev__ipv6only_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, optlen) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return 0;
}
