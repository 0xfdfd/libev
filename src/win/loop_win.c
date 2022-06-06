#include "ev/errno.h"
#include "ev/once.h"
#include "misc.h"
#include "allocator.h"
#include "loop_win.h"
#include "thread_win.h"
#include "winapi.h"
#include "work.h"
#include <assert.h>

ev_loop_win_ctx_t g_ev_loop_win_ctx;

static void _ev_net_init_win(void)
{
    int ret; (void)ret;
    WSADATA wsa_data;
    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0)
    {
        assert(ret == 0);
    }

    g_ev_loop_win_ctx.net.zero_[0] = '\0';
    if ((ret = ev_ipv4_addr("0.0.0.0", 0, &g_ev_loop_win_ctx.net.addr_any_ip4)) != EV_SUCCESS)
    {
        assert(ret == EV_SUCCESS);
    }
    if ((ret = ev_ipv6_addr("::", 0, &g_ev_loop_win_ctx.net.addr_any_ip6)) != EV_SUCCESS)
    {
        assert(ret == EV_SUCCESS);
    }
}

static void _ev_time_init_win(void)
{
    LARGE_INTEGER perf_frequency;

    /* Retrieve high-resolution timer frequency
     * and precompute its reciprocal.
     */
    if (QueryPerformanceFrequency(&perf_frequency))
    {
        g_ev_loop_win_ctx.hrtime_frequency_ = perf_frequency.QuadPart;
    }
    else
    {
        BREAK_ABORT();
    }
}

static uint64_t _ev_hrtime_win(unsigned int scale)
{
    LARGE_INTEGER counter;
    double scaled_freq;
    double result;

    assert(scale != 0);
    if (!QueryPerformanceCounter(&counter))
    {
        BREAK_ABORT();
    }
    assert(counter.QuadPart != 0);

    /* Because we have no guarantee about the order of magnitude of the
     * performance counter interval, integer math could cause this computation
     * to overflow. Therefore we resort to floating point math.
     */
    scaled_freq = (double)g_ev_loop_win_ctx.hrtime_frequency_ / scale;
    result = (double)counter.QuadPart / scaled_freq;
    return (uint64_t)result;
}

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
    _ev_check_layout_win();
    _ev_net_init_win();
    ev__winapi_init();
    _ev_time_init_win();
    ev__thread_init_win();
}

uint64_t ev__clocktime(void)
{
    return _ev_hrtime_win(1000);
}

void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
    int repeat;
    BOOL success;
    ULONG count;
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
        if (GetLastError() != WAIT_TIMEOUT)
        {
            BREAK_ABORT();
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

void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg)
{
    req->cb = callback;
    req->arg = arg;
    memset(&req->overlapped, 0, sizeof(req->overlapped));
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
    ev__exit_work(loop);

    if (loop->backend.iocp != NULL)
    {
        CloseHandle(loop->backend.iocp);
        loop->backend.iocp = NULL;
    }
}

void ev__init_once_win(void)
{
    static ev_once_t once = EV_ONCE_INIT;
    ev_once_execute(&once, _ev_init_once_win);
}

int ev__loop_init_backend(ev_loop_t* loop)
{
    ev__init_once_win();

    loop->backend.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (loop->backend.iocp == NULL)
    {
        int err = GetLastError();
        return ev__translate_sys_error(err);
    }

    ev__init_work(loop);

    return EV_SUCCESS;
}

void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req)
{
    if (!PostQueuedCompletionStatus(loop->backend.iocp, 0, 0, &req->overlapped))
    {
        EV_ABORT();
    }
}

int ev__reuse_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    int err;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen) != 0)
    {
        err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

int ev__ipv6only_win(SOCKET sock, int opt)
{
    DWORD optval = !!opt;
    int optlen = sizeof(optval);

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, optlen) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}
