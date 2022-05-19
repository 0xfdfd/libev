#ifndef __EV_COMMON_INTERNAL_H__
#define __EV_COMMON_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "ev-platform.h"
#include <stdlib.h>
#include <assert.h>

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

/**
 * @brief exchange value of \p v1 and \p v2.
 * @note \p v1 and \p v2 must have the same type.
 * @param[in] TYPE      Type of \p v1 and \p v2.
 * @param[in,out] v1    value1
 * @param[in,out] v2    value2
 */
#define EXCHANGE_VALUE(TYPE, v1, v2)    \
    do {\
        TYPE _tmp = v1;\
        v1 = v2;\
        v2 = _tmp;\
    } while(0)

/**
 * @def EV_COUNT_ARG
 * @brief Count the number of arguments in macro
 */
#ifdef _MSC_VER // Microsoft compilers
#   define EV_COUNT_ARG(...)  _EV_INTERNAL_EXPAND_ARGS_PRIVATE(_EV_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define _EV_INTERNAL_EXPAND_ARGS_PRIVATE(...) EAF_EXPAND(_EV_INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#else // Non-Microsoft compilers
#   define EV_COUNT_ARG(...) _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#endif

#define ENSURE_LAYOUT(TYPE_A, FIELD_A_1, FIELD_A_2, TYPE_B, FIELD_B_1, FIELD_B_2)   \
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
 * @param[in] force     Force exit, without async callback
 */
API_LOCAL void ev__handle_exit(ev_handle_t* handle, int force);

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
 * @brief Get event loop for the handle.
 * @param[in] handle    handler
 * @return              Event loop
 */
API_LOCAL ev_loop_t* ev__handle_loop(ev_handle_t* handle);

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

/**
 * @brief Submit task to event loop with multi-thread support.
 * @note MT-Safe
 * @note Use this function in threads that \p loop not running. If the thread
 *   has \p loop running, use #ev_todo().
 * @param[in] loop  Event loop
 * @param[in] token Todo token
 * @param[in] cb    Callback
 */
API_LOCAL void ev__loop_submit_task_mt(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb);

/**
 * @brief Get minimal length of specific \p addr type.
 * @param[in] addr  A valid sockaddr buffer
 * @return          A valid minimal length, or (socklen_t)-1 if error.
 */
API_LOCAL socklen_t ev__get_addr_len(const struct sockaddr* addr);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
API_LOCAL int ev__write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup write request
 * @param[in] req   Write request
 */
API_LOCAL void ev__write_exit(ev_write_t* req);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
API_LOCAL int ev__read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup read request
 * @param[in] req   read request
 */
API_LOCAL void ev__read_exit(ev_read_t* req);

#ifdef __cplusplus
}
#endif
#endif
