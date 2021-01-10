#ifndef __EV_H__
#define __EV_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "ev/list.h"

enum ev_errno;
typedef enum ev_errno ev_errno_t;

enum ev_loop_mode;
typedef enum ev_loop_mode ev_loop_mode_t;

struct ev_handle;
typedef struct ev_handle ev_handle_t;

struct ev_loop;
typedef struct ev_loop ev_loop_t;

struct ev_timer;
typedef struct ev_timer ev_timer_t;

struct ev_once;
typedef struct ev_once ev_once_t;

struct ev_todo;
typedef struct ev_todo ev_todo_t;

struct ev_async;
typedef struct ev_async ev_async_t;

struct ev_tcp;
typedef struct ev_tcp ev_tcp_t;

struct ev_write;
typedef struct ev_write ev_write_t;

struct ev_read;
typedef struct ev_read ev_read_t;

/**
 * @brief Called when a object is closed
 * @param[in] handle	A base handle
 */
typedef void (*ev_close_cb)(ev_handle_t* handle);

/**
 * @brief An application-defined callback function.
 *
 * Specify a pointer to this function when calling the #ev_once_execute function.
 */
typedef void(*ev_once_cb)(void);

/**
 * @brief Type definition for callback passed to #ev_timer_start().
 * @param[in] handle	A pointer to #ev_timer_t structure
 */
typedef void(*ev_timer_cb)(ev_timer_t* timer);

/**
 * @brief Type definition for callback to run in next loop
 * @param[in] handle	A pointer to #ev_todo_t structure
 */
typedef void(*ev_todo_cb)(ev_todo_t* todo);

/**
 * @brief Type definition for callback passed to #ev_async_init().
 * @param[in] handle	A pointer to #ev_async_t structure
 */
typedef void (*ev_async_cb)(ev_async_t* async);

/**
 * @brief Close callback for #ev_tcp_t
 * @param[in] sock		A closed socket
 */
typedef void (*ev_tcp_close_cb)(ev_tcp_t* sock);

/**
 * @brief Accept callback
 * @param[in] lisn		Listen socket
 * @param[in] conn		Accepted socket
 * @param[in] stat		#ev_errno_t
 */
typedef void (*ev_accept_cb)(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);

/**
 * @brief Connect callback
 * @param[in] sock		Connect socket
 * @param[in] stat		#ev_errno_t
 */
typedef void (*ev_connect_cb)(ev_tcp_t* sock, int stat);

/**
 * @brief Write callback
 * @param[in] req		Write request
 * @param[in] size		Write size
 * @param[in] stat		Write result
 */
typedef void (*ev_write_cb)(ev_write_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req		Read callback
 * @param[in] size		Read size
 * @param[in] stat		Read result
 */
typedef void (*ev_read_cb)(ev_read_t* req, size_t size, int stat);

struct ev_todo
{
	ev_list_node_t			node;			/**< List node */
	ev_todo_cb				cb;				/**< Callback */
};
#define EV_TODO_INIT		{ EV_LIST_NODE_INIT, NULL }

#if defined(_WIN32)
#	include "ev/win.h"
#else
#	include "ev/unix.h"
#endif

enum ev_errno
{
	EV_SUCCESS			= 0,			/**< success */

	/* Posix compatible error code */
	EV_EACCES			= EACCES,		/**< permission denied */
	EV_EBUSY			= EBUSY,		/**< resource busy or locked */
	EV_EINVAL			= EINVAL,		/**< invalid argument */
	EV_ENOSPC			= ENOSPC,		/**< no enough space */
	EV_EADDRINUSE		= EADDRINUSE,	/**< address is already in use */
	EV_EINPROGRESS		= EINPROGRESS,	/**< operation in progress */
	EV_EALREADY			= EALREADY,		/**< already done */

	/* Extend error code */
	EV_UNKNOWN			= -1,			/**< Unknown error */
	EV_EOF				= -2,			/**< end of file */
};

enum ev_loop_mode
{
	ev_loop_mode_default,
	ev_loop_mode_once,
	ev_loop_mode_nowait,
};

struct ev_loop
{
	uint64_t				hwtime;			/**< A fast clock time in milliseconds */
	size_t					active_handles;	/**< Active handle counter */

	struct
	{
		ev_list_t			queue;			/**< (#ev_todo_t) Pending task */
	}todo;

	struct
	{
		ev_map_t			heap;			/**< (#ev_timer_t) Timer heap */
	}timer;

	struct
	{
		unsigned			b_stop : 1;		/**< Flag: need to stop */
	}mask;

	ev_loop_plt_t			backend;		/**< Platform related implementation */
};
#define EV_LOOP_INIT		{ 0, 0, { EV_LIST_INIT }, { EV_MAP_INIT(NULL, NULL) }, { 0 }, EV_LOOP_PLT_INIT }

struct ev_handle
{
	ev_loop_t*				loop;			/**< The event loop belong to */
	ev_close_cb				close_cb;		/**< Close callback */
	ev_todo_t				close_queue;	/**< Close queue token */
	unsigned				flags;			/**< Handle flags */
};

struct ev_timer
{
	ev_handle_t				base;			/**< Base object */
	ev_map_node_t			node;			/**< (#ev_loop_t::timer::heap) */

	ev_timer_cb				close_cb;		/**< Close callback */

	struct
	{
		uint64_t			active;			/**< Active time */
	}data;

	struct
	{
		ev_timer_cb			cb;				/**< User callback */
		uint64_t			timeout;		/**< Timeout */
		uint64_t			repeat;			/**< Repeat */
	}attr;
};

struct ev_async
{
	ev_handle_t				base;			/**< Base object */

	ev_async_cb				active_cb;		/**< Active callback */
	ev_async_cb				close_cb;		/**< Close callback */

	ev_async_backend_t		backend;		/**< Platform related implementation */
};

struct ev_tcp
{
	ev_handle_t				base;			/**< Base object */
	ev_tcp_close_cb			close_cb;		/**< User close callback */

	ev_os_socket_t			sock;			/**< Socket handle */
	ev_tcp_backend_t		backend;		/**< Platform related implementation */
};

struct ev_write
{
	ev_list_node_t			node;
	struct
	{
		ev_write_cb			cb;				/**< Write complete callback */
		ev_buf_t*			bufs;			/**< Buffer list */
		size_t				nbuf;			/**< Buffer list count */
	}data;
	struct
	{
		size_t				idx;			/**< Write buffer index */
		size_t				len;			/**< Total write size */
	}info;
};

struct ev_read
{
	ev_list_node_t			node;
	struct
	{
		ev_read_cb			cb;				/**< Read complete callback */
		ev_buf_t*			bufs;			/**< Buffer list */
		size_t				nbuf;			/**< Buffer list count */
	}data;
};

/**
 * @brief Initializes the given structure.
 * @param[out] loop		Event loop handler
 * @return				#ev_errno_t
 */
int ev_loop_init(ev_loop_t* loop);

/**
 * @brief Releases all internal loop resources.
 *
 * Call this function only when the loop has finished executing and all open
 * handles and requests have been closed, or it will return #EV_EBUSY. After
 * this function returns, the user can free the memory allocated for the loop.
 */
void ev_loop_exit(ev_loop_t* loop);

/**
 * @brief Stop the event loop, causing uv_run() to end as soon as possible.
 *
 * This will happen not sooner than the next loop iteration. If this function
 * was called before blocking for i/o, the loop won't block for i/o on this
 * iteration.
 *
 * @param[in] loop		Event loop handler
 */
void ev_loop_stop(ev_loop_t* loop);

/**
 * @brief This function runs the event loop.
 *
 * It will act differently depending on the specified mode:
 * + #ev_loop_mode_default: Runs the event loop until there are no more active
 *     and referenced handles or requests. Returns non-zero if #ev_loop_stop()
 *     was called and there are still active handles or requests. Returns zero
 *     in all other cases.
 * + #ev_loop_mode_once: Poll for i/o once. Note that this function blocks if
 *     there are no pending callbacks. Returns zero when done (no active
 *     handles or requests left), or non-zero if more callbacks are expected
 *     (meaning you should run the event loop again sometime in the future).
 * + #ev_loop_mode_nowait: Poll for i/o once but don't block if there are no
 *     pending callbacks. Returns zero if done (no active handles or requests
 *     left), or non-zero if more callbacks are expected (meaning you should
 *     run the event loop again sometime in the future).
 *
 * @param[in] loop      Event loop handler
 * @param[in] mode      Running mode
 * @return              Returns zero when no active handles or requests left,
 *                      otherwise return non-zero
 */
int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode);

/**
 * @brief Initialize the handle.
 * @param[in] loop		A pointer to the event loop
 * @param[out] handle	The structure to initialize
 * @return				#ev_errno_t
 */
int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle);

/**
 * @brief Destroy the timer
 * @warning The timer structure cannot be freed until close_cb is called.
 * @param[in] handle	Timer handle
 * @param[in] close_cb	Close callback
 */
void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb);

/**
 * @brief Start the timer. timeout and repeat are in milliseconds.
 *
 * If timeout is zero, the callback fires on the next event loop iteration. If
 * repeat is non-zero, the callback fires first after timeout milliseconds and
 * then repeatedly after repeat milliseconds.
 *
 * @param[in] handle	Timer handle
 * @param[in] cb		Active callback
 * @param[in] timeout	The first callback timeout
 * @param[in] repeat	Repeat timeout
 * @return				#ev_errno_t
 */
int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat);

/**
 * @brief Stop the timer.
 *
 * the callback will not be called anymore.
 *
 * @param[in] handle	Timer handle
 */
void ev_timer_stop(ev_timer_t* handle);

/**
 * @brief Initialize the handle.
 *
 * A NULL callback is allowed.
 * 
 * @param[in] loop		Event loop
 * @param[out] handle	A pointer to the structure
 * @param[in] cb		Active callback
 * @return				#ev_errno_t
 */
int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb);

/**
 * @brief Destroy the structure.
 * @param[in] handle	Async handle
 * @param[in] close_cb	Close callback
 */
void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb);

/**
 * @brief Wake up the event loop and call the async handle's callback.
 * @param[in] handle	Async handle
 */
void ev_async_weakup(ev_async_t* handle);

/**
 * @brief Executes the specified function one time.
 *
 * No other threads that specify the same one-time initialization structure can
 * execute the specified function while it is being executed by the current thread.
 *
 * @param[in] guard		A pointer to the one-time initialization structure.
 * @param[in] cb		A pointer to an application-defined InitOnceCallback function.
 */
void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @brief Initialize a tcp socket
 * @param[in] loop		Event loop
 * @param[out] tcp		TCP handle
 */
int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp);

/**
 * @brief Destroy socket
 * @param[in] sock		Socket
 * @param[in] cb		Destroy callback
 */
void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb);

/**
 * @brief Bind the handle to an address and port.
 * addr should point to an initialized struct sockaddr_in or struct sockaddr_in6.
 * @param[in] tcp		Socket handler
 * @param[in] addr		Bind address
 * @param[in] addrlen	Address length
 * @return				#ev_errno_t
 */
int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen);

/**
 * @brief Start listening for incoming connections.
 * @param[in] tcp		Listen socket
 * @param[in] backlog	The number of connections the kernel might queue
 * @return				#ev_errno_t
 */
int ev_tcp_listen(ev_tcp_t* tcp, int backlog);

/**
 * @brief Accept a connection from listen socket
 * @param[in] acpt	Listen socket
 * @param[in] conn	The socket to store new connection
 * @param[in] cb	Accept callback
 * @return			#ev_errno_t
 */
int ev_tcp_accept(ev_tcp_t* acpt, ev_tcp_t* conn, ev_accept_cb cb);

/**
 * @brief Connect to address
 * @param[in] addr	Address
 * @param[in] size	Address size
 * @param[in] cb	Connect callback
 * @return			#ev_errno_t
 */
int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb);

/**
 * @brief Write data
 * @param[in] sock	Socket handle
 * @param[in] req	Write request
 * @param[in] bufs	Buffer list
 * @param[in] nbufs	Buffer list count
 * @param[in] cb	Write complete callback
 * @return			#ev_errno_t
 */
int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb);

/**
 * @brief Read data
 * @param[in] sock	Socket handle
 * @param[in] req	Read request
 * @param[in] bufs	Buffer list
 * @param[in] nbufs	Buffer list count
 * @param[in] cb	Read complete callback
 * @return			#ev_errno_t
 */
int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb);

/**
 * @brief Get the current address to which the socket is bound.
 * @param[in] sock	Socket handle
 * @param[out] name	A buffer to store address
 * @param[in,out] len	buffer size
 * @return			#ev_errno_t
 */
int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @brief Get the address of the peer connected to the socket.
 * @param[in] sock	Socket handle
 * @param[out] name	A buffer to store address
 * @param[in,out] len	buffer size
 * @return			#ev_errno_t
 */
int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @brief Convert ip and port into network address
 * @param[in] ip	Character string contains IP
 * @param[in] port	Port
 * @param[out] addr	network address structure
 * @return			#ev_errno_t
 */
int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr);

/**
 * @brief Convert ip and port into network address
 * @param[in] ip	Character string contains IP
 * @param[in] port	Port
 * @param[out] addr	network address structure
 * @return			#ev_errno_t
 */
int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr	network address structure
 * @param[out] port	Port
 * @param[out] ip	A buffer to store IP string
 * @param[in] len	Buffer length
 * @return			#ev_errno_t
 */
int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr	network address structure
 * @param[out] port	Port
 * @param[out] ip	A buffer to store IP string
 * @param[in] len	Buffer length
 * @return			#ev_errno_t
 */
int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len);

/**
 * @brief Describe the error code
 * @param[in] err	Error code
 * @return			Describe string
 */
const char* ev_strerror(int err);

#ifdef __cplusplus
}
#endif
#endif
