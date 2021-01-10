#include <assert.h>
#include "ev-platform.h"
#include "loop.h"
#include "tcp.h"

/* Frequency of the high-resolution clock. */
static uint64_t hrtime_frequency_ = 0;

static void _ev_time_init_win(void)
{
	LARGE_INTEGER perf_frequency;

	/* Retrieve high-resolution timer frequency
	 * and precompute its reciprocal.
	 */
	if (QueryPerformanceFrequency(&perf_frequency))
	{
		hrtime_frequency_ = perf_frequency.QuadPart;
	}
	else
	{
		ABORT();
	}
}

static uint64_t _ev_hrtime_win(unsigned int scale)
{
	LARGE_INTEGER counter;
	double scaled_freq;
	double result;

	assert(hrtime_frequency_ != 0);
	assert(scale != 0);
	if (!QueryPerformanceCounter(&counter))
	{
		ABORT();
	}
	assert(counter.QuadPart != 0);

	/* Because we have no guarantee about the order of magnitude of the
	 * performance counter interval, integer math could cause this computation
	 * to overflow. Therefore we resort to floating point math.
	 */
	scaled_freq = (double)hrtime_frequency_ / scale;
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
			ev_iocp_t* req = container_of(overlappeds[i].lpOverlapped, ev_iocp_t, overlapped);
			req->cb(req);
		}
	}
}

static void _ev_init_once_win(void)
{
	ev__tcp_init();
}

void ev__loop_update_time(ev_loop_t* loop)
{
	static ev_once_t s_guard = EV_ONCE_INIT;
	ev_once_execute(&s_guard, _ev_time_init_win);

	loop->hwtime = _ev_hrtime_win(1000);
}

int ev__translate_sys_error(int err)
{
	static int s_err_table[][2] = {
		{ WSAEINVAL, EINVAL },
	};

	size_t i;
	for (i = 0; i < ARRAY_SIZE(s_err_table); i++)
	{
		if (err == s_err_table[i][0])
		{
			return s_err_table[i][1];
		}
	}

	ABORT();
	return err;
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
			ABORT();
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

void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb)
{
	req->cb = cb;
	memset(&req->overlapped, 0, sizeof(req->overlapped));
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
	CloseHandle(loop->backend.iocp);
	loop->backend.iocp = NULL;
}

int ev__loop_init_backend(ev_loop_t* loop)
{
	static ev_once_t once = EV_ONCE_INIT;
	ev_once_execute(&once, _ev_init_once_win);

	loop->backend.iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (loop->backend.iocp == NULL)
	{
		return ev__translate_sys_error(GetLastError());
	}
	return EV_SUCCESS;
}

int ev__ntstatus_to_winsock_error(NTSTATUS status)
{
	switch (status)
	{
	case STATUS_SUCCESS:
		return ERROR_SUCCESS;

	case STATUS_PENDING:
		return ERROR_IO_PENDING;

	case STATUS_INVALID_HANDLE:
	case STATUS_OBJECT_TYPE_MISMATCH:
		return WSAENOTSOCK;

	case STATUS_INSUFFICIENT_RESOURCES:
	case STATUS_PAGEFILE_QUOTA:
	case STATUS_COMMITMENT_LIMIT:
	case STATUS_WORKING_SET_QUOTA:
	case STATUS_NO_MEMORY:
	case STATUS_QUOTA_EXCEEDED:
	case STATUS_TOO_MANY_PAGING_FILES:
	case STATUS_REMOTE_RESOURCES:
		return WSAENOBUFS;

	case STATUS_TOO_MANY_ADDRESSES:
	case STATUS_SHARING_VIOLATION:
	case STATUS_ADDRESS_ALREADY_EXISTS:
		return WSAEADDRINUSE;

	case STATUS_LINK_TIMEOUT:
	case STATUS_IO_TIMEOUT:
	case STATUS_TIMEOUT:
		return WSAETIMEDOUT;

	case STATUS_GRACEFUL_DISCONNECT:
		return WSAEDISCON;

	case STATUS_REMOTE_DISCONNECT:
	case STATUS_CONNECTION_RESET:
	case STATUS_LINK_FAILED:
	case STATUS_CONNECTION_DISCONNECTED:
	case STATUS_PORT_UNREACHABLE:
	case STATUS_HOPLIMIT_EXCEEDED:
		return WSAECONNRESET;

	case STATUS_LOCAL_DISCONNECT:
	case STATUS_TRANSACTION_ABORTED:
	case STATUS_CONNECTION_ABORTED:
		return WSAECONNABORTED;

	case STATUS_BAD_NETWORK_PATH:
	case STATUS_NETWORK_UNREACHABLE:
	case STATUS_PROTOCOL_UNREACHABLE:
		return WSAENETUNREACH;

	case STATUS_HOST_UNREACHABLE:
		return WSAEHOSTUNREACH;

	case STATUS_CANCELLED:
	case STATUS_REQUEST_ABORTED:
		return WSAEINTR;

	case STATUS_BUFFER_OVERFLOW:
	case STATUS_INVALID_BUFFER_SIZE:
		return WSAEMSGSIZE;

	case STATUS_BUFFER_TOO_SMALL:
	case STATUS_ACCESS_VIOLATION:
		return WSAEFAULT;

	case STATUS_DEVICE_NOT_READY:
	case STATUS_REQUEST_NOT_ACCEPTED:
		return WSAEWOULDBLOCK;

	case STATUS_INVALID_NETWORK_RESPONSE:
	case STATUS_NETWORK_BUSY:
	case STATUS_NO_SUCH_DEVICE:
	case STATUS_NO_SUCH_FILE:
	case STATUS_OBJECT_PATH_NOT_FOUND:
	case STATUS_OBJECT_NAME_NOT_FOUND:
	case STATUS_UNEXPECTED_NETWORK_ERROR:
		return WSAENETDOWN;

	case STATUS_INVALID_CONNECTION:
		return WSAENOTCONN;

	case STATUS_REMOTE_NOT_LISTENING:
	case STATUS_CONNECTION_REFUSED:
		return WSAECONNREFUSED;

	case STATUS_PIPE_DISCONNECTED:
		return WSAESHUTDOWN;

	case STATUS_CONFLICTING_ADDRESSES:
	case STATUS_INVALID_ADDRESS:
	case STATUS_INVALID_ADDRESS_COMPONENT:
		return WSAEADDRNOTAVAIL;

	case STATUS_NOT_SUPPORTED:
	case STATUS_NOT_IMPLEMENTED:
		return WSAEOPNOTSUPP;

	case STATUS_ACCESS_DENIED:
		return WSAEACCES;

	default:
		if ((status & (FACILITY_NTWIN32 << 16)) == (FACILITY_NTWIN32 << 16) &&
			(status & (ERROR_SEVERITY_ERROR | ERROR_SEVERITY_WARNING)))
		{
			/* It's a windows error that has been previously mapped to an ntstatus
			 * code. */
			return (DWORD)(status & 0xffff);
		}
		else
		{
			/* The default fallback for unmappable ntstatus codes. */
			return WSAEINVAL;
		}
	}
}
