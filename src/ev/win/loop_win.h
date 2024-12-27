#ifndef __EV_LOOP_WIN_INTERNAL_H__
#define __EV_LOOP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#define EV_INVALID_PID_WIN  0

typedef struct ev_loop_win_ctx
{
    struct
    {
        char                    zero_[1];                   /**< A zero length buffer */
    } net;
} ev_loop_win_ctx_t;

extern ev_loop_win_ctx_t        g_ev_loop_win_ctx;          /**< Global runtime for Windows */

/**
 * @brief Initialize windows context.
 */
EV_LOCAL void ev__init_once_win(void);

/**
 * @brief Initialize IOCP request
 * @param[out] req      A pointer to the IOCP request
 * @param[in] callback  A callback when the request is finish
 * @param[in] arg       User defined argument passed to callback
 */
EV_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg);

/**
 * @brief Post to specific IOCP request.
 * @param[in] loop      Event loop
 * @param[in] req       IOCP request
 */
EV_LOCAL void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req);

/**
 * @brief Set \p sock as reusable address.
 * @param[in] sock  Socket to set reusable.
 * @param[in] opt   0 if not reusable, otherwise reusable.
 * @return          #ev_errnot_t
 */
EV_LOCAL int ev__reuse_win(SOCKET sock, int opt);

/**
 * @brief Set \p sock as IPv6 only.
 * @param[in] sock  Socket to set IPv6 only.
 * @param[in] opt   0 if IPv4 available, otherwise IPv6 only.
 * @return          #ev_errnot_t
 */
EV_LOCAL int ev__ipv6only_win(SOCKET sock, int opt);

#ifdef __cplusplus
}
#endif
#endif
