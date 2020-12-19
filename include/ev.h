#ifndef __EV_H__
#define __EV_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#	include "ev/win.h"
#else
#	include "ev/unix.h"
#endif

#include "ev/map.h"
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

enum ev_errno
{
	EV_SUCCESS			= 0,			/**< success */

	/* Posix compatible error code */
	EV_EBUSY			= EBUSY,		/**< resource busy or locked */
};

enum ev_loop_mode
{
	ev_loop_mode_default,
	ev_loop_mode_once,
	ev_loop_mode_nowait,
};

struct ev_todo
{
	ev_list_node_t		node;			/**< List node */
	ev_todo_cb			cb;				/**< Callback */
};
#define EV_TODO_INIT	{ EV_LIST_NODE_INIT, NULL }

struct ev_loop
{
	uint64_t			hwtime;			/**< A fast clock time in milliseconds */
	size_t				active_handles;	/**< Active handle counter */

	struct
	{
		ev_list_t		queue;			/**< (#ev_todo_t) Pending task */
	}todo;

	struct
	{
		ev_map_t		heap;			/**< (#ev_timer_t) Timer heap */
	}timer;

	struct
	{
		unsigned		b_stop : 1;		/**< Flag: need to stop */
	}mask;

	ev_loop_plt_t		backend;		/**< Platform related implementation */
};
#define EV_LOOP_INIT	{ 0, 0, { EV_LIST_INIT }, { EV_MAP_INIT(NULL, NULL) }, { 0 }, EV_LOOP_PLT_INIT }

struct ev_handle
{
	ev_loop_t*			loop;			/**< The event loop belong to */
	ev_close_cb			close_cb;		/**< Close callback */
	ev_todo_t			close_queue;	/**< Close queue token */
	unsigned			flags;			/**< Handle flags */
};

struct ev_timer
{
	ev_handle_t			base;			/**< Base object */
	ev_map_node_t		node;			/**< (#ev_loop_t::timer::heap) */

	ev_timer_cb			close_cb;		/**< Close callback */

	struct
	{
		uint64_t		active;			/**< Active time */
	}data;

	struct
	{
		ev_timer_cb		cb;				/**< User callback */
		uint64_t		timeout;		/**< Timeout */
		uint64_t		repeat;			/**< Repeat */
	}attr;
};

struct ev_async
{
	ev_handle_t			base;			/**< Base object */

	ev_async_cb			active_cb;		/**< Active callback */
	ev_async_cb			close_cb;		/**< Close callback */

	ev_async_backend_t	backend;		/**< Platform related implementation */
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

#ifdef __cplusplus
}
#endif
#endif
