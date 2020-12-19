#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <windows.h>

struct ev_once
{
	INIT_ONCE	guard;
};
#define EV_ONCE_INIT			{ INIT_ONCE_STATIC_INIT }

typedef struct ev_loop_plt
{
	HANDLE		iocp;
}ev_loop_plt_t;
#define EV_LOOP_PLT_INIT		{ NULL }

struct ev_iocp;
typedef struct ev_iocp ev_iocp_t;

/**
 * @brief IOCP complete callback
 * @param[in] req	IOCP request
 */
typedef void(*ev_iocp_cb)(ev_iocp_t* req);

struct ev_iocp
{
	ev_iocp_cb		cb;			/**< Callback */
	OVERLAPPED		overlapped;	/**< IOCP field */
};
#define EV_IOCP_INIT			{ NULL, { NULL, NULL, { { 0, 0 } } } }

typedef struct ev_async_backend
{
	ev_iocp_t		iocp;		/**< IOCP request */
}ev_async_backend_t;
#define EV_ASYNC_BACKEND_INIT	{ EV_IOCP_INIT }

#ifdef __cplusplus
}
#endif
#endif
