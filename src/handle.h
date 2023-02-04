#ifndef __EV_HANDLE_INTERNAL_H__
#define __EV_HANDLE_INTERNAL_H__

#include "ev.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ev_handle_flag
{
    /* Used by all handles. Bit 0-7. */
    EV_HANDLE_CLOSING           = 0x01 << 0x00,     /**< 1. Handle is going to close */
    EV_HANDLE_CLOSED            = 0x01 << 0x01,     /**< 2. Handle is closed */
    EV_HANDLE_ACTIVE            = 0x01 << 0x02,     /**< 4. Handle is busy */

    /* #EV_ROLE_EV_TCP */
    EV_HANDLE_TCP_LISTING       = 0x01 << 0x08,     /**< 256. This is a listen socket and is listening */
    EV_HANDLE_TCP_ACCEPTING     = 0x01 << 0x09,     /**< 512. This is a socket waiting for accept */
    EV_HANDLE_TCP_STREAMING     = 0x01 << 0x0A,     /**< 1024. This is a socket waiting for read or write */
    EV_HANDLE_TCP_CONNECTING    = 0x01 << 0x0B,     /**< 2048. This is a connect and waiting for connect complete */
    EV_HABDLE_TCP_BOUND         = 0x01 << 0x0C,     /**< 4096. Socket is bond to address */

    /* #EV_ROLE_EV_UDP */
    EV_HANDLE_UDP_IPV6          = 0x01 << 0x08,     /**< 256. This socket have IPv6 ability */
    EV_HANDLE_UDP_CONNECTED     = 0x01 << 0x09,     /**< 512. This socket is connected */
    EV_HANDLE_UDP_BOUND         = 0x01 << 0x0A,     /**< 1024. Socket is bond to address */
    EV_HANDLE_UDP_BYPASS_IOCP   = 0x01 << 0x0B,     /**< 2048. FILE_SKIP_SET_EVENT_ON_HANDLE | FILE_SKIP_COMPLETION_PORT_ON_SUCCESS */

    /* #EV_ROLE_EV_PIPE */
    EV_HANDLE_PIPE_IPC          = 0x01 << 0x08,     /**< 256. This pipe is support IPC */
    EV_HANDLE_PIPE_STREAMING    = 0x01 << 0x09,     /**< 512. This pipe is initialized by #ev_stream_t */
} ev_handle_flag_t;

/**
 * @brief Initialize a handle.
 *
 * A initialized handle will be linked with \p loop. By default the \p handle
 * is in #ev_loop_t::handles::idle_list. If the \p handle is active (The event
 * counter is non-zero), the handle is moved into #ev_loop_t::handles::active_list.
 *
 * @note Once a handle is initialized, it must call #ev__handle_exit() when no
 *   longer needed.
 * @param[in] loop      The loop own the handle
 * @param[out] handle   A pointer to the structure
 * @param[in] role      Who we are
 */
API_LOCAL void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role);

/**
 * @brief Close the handle
 * @note The handle will not closed until close_cb was called, which was given
 *   by #ev__handle_init()
 * @note #ev__handle_exit() never reset active_events counter for you. You always
 *   need to balance active_events counter yourself.
 * @param[in] handle    handler
 * @param[in] close_cb  Close callback. If non-null, the \p close_cb will be
 *   called in next event loop. If null, the handle will be closed synchronously.
 */
API_LOCAL void ev__handle_exit(ev_handle_t* handle, ev_handle_cb close_cb);

/**
 * @brief Add active event counter. If active event counter is non-zero,
 *   #EV_HANDLE_ACTIVE is appended.
 * @param[in] handle    Handler.
 */
API_LOCAL void ev__handle_event_add(ev_handle_t* handle);

/**
 * @brief Decrease active event counter. If active event counter is zero,
 *   #EV_HANDLE_ACTIVE is removed.
 * @param[in] handle    Handler.
 */
API_LOCAL void ev__handle_event_dec(ev_handle_t* handle);

/**
 * @brief Check if the handle is in active state
 * @param[in] handle    handler
 * @return              bool
 */
API_LOCAL int ev__handle_is_active(ev_handle_t* handle);

/**
 * @brief Check if the handle is in closing or closed state
 * @param[in] handle    handler
 * @return              bool
 */
API_LOCAL int ev__handle_is_closing(ev_handle_t* handle);

/**
 * @brief Queue a task.
 * This task will be execute in next loop.
 * @param[in] handle    handler.
 * @param[in] callback  Task callback
 * @return              #ev_errno_t
 */
API_LOCAL int ev__backlog_submit(ev_handle_t* handle, ev_handle_cb callback);

/**
 * @brief Process backlog events.
 * @param[in] loop Event loop.
 */
API_LOCAL void ev__process_backlog(ev_loop_t* loop);

/**
 * @brief Process endgame events.
 * @param[in] loop Event loop.
 */
API_LOCAL void ev__process_endgame(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
