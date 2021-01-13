#ifndef __EV_UNIX_H__
#define __EV_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "ev/map.h"

typedef int ev_os_socket_t;

/**
 * @brief Buffer
 * @internal Must share the same layout with struct iovec
 */
struct ev_buf
{
	char*						data;		/**< Data address */
	size_t						size;		/**< Data size */
};
#define EV_BUF_INIT(buf, len)	{ (char*)buf, (size_t)len }

struct ev_once
{
	pthread_once_t				guard;		/**< Once token */
};
#define EV_ONCE_INIT			{ PTHREAD_ONCE_INIT }

typedef struct ev_loop_plt
{
	int							pollfd;			/**< Multiplexing */
	ev_map_t					io;				/**< #ev_io_t */
	struct epoll_event			events[128];	/**< Events array */
}ev_loop_plt_t;
#define EV_LOOP_PLT_INIT		{ -1, EV_MAP_INIT(NULL, NULL) }

typedef struct ev_write_backend
{
	size_t						idx;			/**< Write buffer index */
	size_t						len;			/**< Total write size */
}ev_write_backend_t;

typedef struct ev_read_backend
{
	int							useless[0];		/**< Useless field */
}ev_read_backend_t;

struct ev_io;
typedef struct ev_io ev_io_t;

/**
 * @brief IO active callback
 * @param[in] io	IO object
 * @param[in] evts	IO events
 */
typedef void (*ev_io_cb)(ev_io_t* io, unsigned evts);

struct ev_io
{
	ev_map_node_t				node;			/**< #ev_loop_plt_t::io */
	struct
	{
		int						fd;				/**< File descriptor */
		unsigned				c_events;		/**< Current events */
		unsigned				n_events;		/**< Next events */
		ev_io_cb				cb;				/**< IO active callback */
	}data;
};
#define EV_IO_INIT				{ EV_MAP_NODE_INIT, { 0, 0, 0, NULL } }

typedef struct ev_async_backend
{
	ev_io_t						io_read;		/**< Read request */
	int							fd_write;		/**< File descriptor for write */
}ev_async_backend_t;
#define EV_ASYNC_BACKEND_INIT	{ EV_IO_INIT, -1 }

typedef struct ev_tcp_backend
{
	ev_io_t						io;				/**< IO object */

	union
	{
		struct
		{
			ev_list_t			accept_queue;	/**< Accept queue */
		}listen;

		struct
		{
			ev_accept_cb		cb;				/**< Accept callback */
			ev_list_node_t		accept_node;	/**< Accept queue node */
		}accept;

		struct
		{
			ev_list_t			w_queue;		/**< #ev_read_t Write queue */
			ev_list_t			r_queue;		/**< #ev_write_t Read queue */
		}stream;

		struct
		{
			ev_connect_cb		cb;				/**< Connect callback */
			ev_todo_t			token;			/**< Todo token */
			int					stat;			/**< Connect result */
		}client;
	}u;
}ev_tcp_backend_t;

#ifdef __cplusplus
}
#endif
#endif
