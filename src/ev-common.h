#ifndef __EV_COMMON_INTERNAL_H__
#define __EV_COMMON_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <assert.h>
#include "ev.h"
#include "ev-platform.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define EV_MIN(a, b)    ((a) < (b) ? (a) : (b))

/**
 * @brief Align \p size to \p align, who's value is larger or equal to \p size
 *   and can be divided with no remainder by \p align.
 * @note \p align must equal to 2^n
 */
#define ALIGN_SIZE(size, align) \
    (((uintptr_t)(size) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1))

#define ACCESS_ONCE(TYPE, var)  (*(volatile TYPE*) &(var))

#define ENSURE_LAYOUT(TYPE_A, TYPE_B, FIELD_A_1, FIELD_B_1, FIELD_A_2, FIELD_B_2)   \
    assert(sizeof(TYPE_A) == sizeof(TYPE_B));\
    assert(offsetof(TYPE_A, FIELD_A_1) == offsetof(TYPE_B, FIELD_B_1));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_1) == sizeof(((TYPE_B*)0)->FIELD_B_1));\
    assert(offsetof(TYPE_A, FIELD_A_2) == offsetof(TYPE_B, FIELD_B_2));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_2) == sizeof(((TYPE_B*)0)->FIELD_B_2))

typedef enum ev_ipc_frame_flag
{
    EV_IPC_FRAME_FLAG_INFORMATION = 1,
}ev_ipc_frame_flag_t;

typedef enum ev_handle_flag
{
    /* Used by all handles. Bit 0-7. */
    EV_HANDLE_CLOSING       = 0x01 << 0x00,     /**< 1. Handle is going to close */
    EV_HANDLE_CLOSED        = 0x01 << 0x01,     /**< 2. Handle is closed */
    EV_HANDLE_ACTIVE        = 0x01 << 0x02,     /**< 4. Handle is busy */

    /* EV_ROLE_TCP */
    EV_TCP_LISTING          = 0x01 << 0x08,     /**< 256. This is a listen socket and is listening */
    EV_TCP_ACCEPTING        = 0x01 << 0x09,     /**< 512. This is a socket waiting for accept */
    EV_TCP_STREAMING        = 0x01 << 0x0A,     /**< 1024. This is a socket waiting for read or write */
    EV_TCP_CONNECTING       = 0x01 << 0x0B,     /**< 2048. This is a connect and waiting for connect complete */
    EV_TCP_BOUND            = 0x01 << 0x0C,     /**< 4096. Socket is bond to address */

    /* EV_ROLE_PIPE */
    EV_PIPE_IPC             = 0x01 << 0x08,     /**< 256. This pipe is support IPC */
    EV_PIPE_STREAMING       = 0x01 << 0x09,     /**< 512. This pipe is initialized by #ev_stream_t */
}ev_handle_flag_t;

/**
 * @brief Initialize a handle
 * @param[in] loop      The loop own the handle
 * @param[out] handle   A pointer to the structure
 * @param[in] role      Who we are
 * @param[in] close_cb  A callback when handle is closed
 */
API_LOCAL void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role, ev_close_cb close_cb);

/**
 * @brief Close the handle
 * @note The handle will not closed until close_cb was called, which was given
 *   by #ev__handle_init()
 * @param[in] handle    handler
 */
API_LOCAL void ev__handle_exit(ev_handle_t* handle);

/**
 * @brief Set handle as active
 * @param[in] handle    handler
 */
API_LOCAL void ev__handle_active(ev_handle_t* handle);

/**
 * @brief Set handle as inactive
 * @param[in] handle    handler
 */
API_LOCAL void ev__handle_deactive(ev_handle_t* handle);

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
 * @brief Initialize todo token
 * @param[out] token    Todo token
 */
API_LOCAL void ev__todo_init(ev_todo_t* token);

/**
 * @brief Add a pending task
 * @param[in] loop      Event loop
 * @param[in] token     A pointer to the pending token
 * @param[in] cb        A callback when the pending task is active
 */
API_LOCAL void ev__todo_queue(ev_loop_t* loop, ev_todo_t* token, ev_todo_cb cb);

/**
 * @brief Cancel a pending task
 * @param[in] loop      Event loop
 * @param[in] token     A pending token
 */
API_LOCAL void ev__todo_cancel(ev_loop_t* loop, ev_todo_t* token);

/**
 * @brief Check IPC frame header
 * @param[in] buffer    Buffer to check
 * @param[in] size      Buffer size
 * @return              bool
 */
API_LOCAL int ev__ipc_check_frame_hdr(const void* buffer, size_t size);

/**
 * @brief Initialize IPC frame header
 * @param[out] hdr      Frame header to initialize
 * @param[in] flags     Control flags
 * @param[in] exsz      Extra information size
 * @param[in] dtsz      Data size
 */
API_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t* hdr,
    uint8_t flags, uint16_t exsz, uint32_t dtsz);

/**
 * @brief Update loop time
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_update_time(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif
#endif
