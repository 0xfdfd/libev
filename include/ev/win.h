#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <windows.h>

typedef SOCKET ev_socket_t;

/**
 * @brief Buffer
 * @internal Must share the same layout with WSABUF
 */
typedef struct ev_buf
{
	ULONG			size;		/**< Data size */
	CHAR*			data;		/**< Data address */
}ev_buf_t;

struct ev_once
{
	INIT_ONCE		guard;		/**< Once token */
};
#define EV_ONCE_INIT			{ INIT_ONCE_STATIC_INIT }

typedef struct ev_loop_plt
{
	HANDLE			iocp;		/**< IOCP handle */
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

typedef struct ev_tcp_backend
{
	ev_socket_t				sock;			/**< Socket handle */
	int						af;				/**< AF_INET / AF_INET6 */

	union
	{
		struct
		{
			ev_iocp_t		io;				/**< IOCP handle */
			ev_list_t		accept_queue;	/**< Accept queue */
		}listen;
		struct
		{
			ev_iocp_t		io;				/**< IOCP handle */
			ev_accept_cb	cb;				/**< Accept callback */
			ev_list_node_t	node;			/**< Accept queue node */
			ev_tcp_t*		listen;			/**< Listen socket */
		}accept;
	}u;
}ev_tcp_backend_t;

#ifdef __cplusplus
}
#endif
#endif
