#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT   0x0600
#endif

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "ev/map.h"

typedef SOCKET ev_os_socket_t;

/**
 * @brief Buffer
 * @internal Must share the same layout with WSABUF
 */
typedef struct ev_buf
{
	ULONG						size;				/**< Data size */
	CHAR*						data;				/**< Data address */
}ev_buf_t;

struct ev_once
{
	INIT_ONCE					guard;				/**< Once token */
};
#define EV_ONCE_INIT			{ INIT_ONCE_STATIC_INIT }

typedef struct ev_loop_plt
{
	HANDLE						iocp;				/**< IOCP handle */
}ev_loop_plt_t;
#define EV_LOOP_PLT_INIT		{ NULL }

struct ev_iocp;
typedef struct ev_iocp ev_iocp_t;

/**
 * @brief IOCP complete callback
 * @param[in] req	IOCP request
 */
typedef void(*ev_iocp_cb)(ev_iocp_t* req, size_t transferred);

struct ev_iocp
{
	ev_iocp_cb					cb;					/**< Callback */
	OVERLAPPED					overlapped;			/**< IOCP field */
};
#define EV_IOCP_INIT			{ NULL, { NULL, NULL, { { 0, 0 } }, NULL } }

typedef struct ev_async_backend
{
	ev_iocp_t					iocp;				/**< IOCP request */
}ev_async_backend_t;
#define EV_ASYNC_BACKEND_INIT	{ EV_IOCP_INIT }

typedef struct ev_write_backend
{
	void*						owner;				/**< Owner */
	ev_iocp_t					io;					/**< IOCP backend */
	size_t						size;				/**< Written size */
	int							stat;				/**< Write result */
}ev_write_backend_t;

typedef struct ev_read_backend
{
	void*						owner;				/**< Owner */
	ev_iocp_t					io;					/**< IOCP backend */
	size_t						size;				/**< Written size */
	int							stat;				/**< Write result */
}ev_read_backend_t;

typedef struct ev_tcp_backend
{
	int							af;					/**< AF_INET / AF_INET6 */
	ev_iocp_t					io;					/**< IOCP */
	ev_todo_t					token;				/**< Todo token */

	struct
	{
		unsigned				todo_pending : 1;	/**< Already submit todo request */
	}mask;

	union
	{
		struct
		{
			ev_list_t			a_queue;			/**< Accept queue */
			ev_list_t			a_queue_done;		/**< Accept done queue */
		}listen;
		struct
		{
			ev_accept_cb		cb;					/**< Accept callback */
			ev_list_node_t		node;				/**< Accept queue node */
			ev_tcp_t*			listen;				/**< Listen socket */
			int					stat;				/**< Accept result */
			char				buffer[sizeof(struct sockaddr_storage) * 2 + 32];
		}accept;
		struct
		{
			ev_connect_cb		cb;					/**< Callback */
			LPFN_CONNECTEX		fn_connectex;		/**< ConnectEx */
			int					stat;				/**< Connect result */
		}conn;
		struct
		{
			ev_list_t			w_queue;			/**< Write queue */
			ev_list_t			w_queue_done;		/**< Write done queue */
			ev_list_t			r_queue;			/**< Read queue */
			ev_list_t			r_queue_done;		/**< Read done queue */
		}stream;
	}u;
}ev_tcp_backend_t;

#ifdef __cplusplus
}
#endif
#endif
