#ifndef __EV_LOOP_WIN_INTERNAL_H__
#define __EV_LOOP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "loop.h"
#include "ev/thread.h"

#define EV_INVALID_PID_WIN  0

typedef struct ev_loop_win_ctx
{
    /**
     * Frequency of the high-resolution clock.
     */
    uint64_t                    hrtime_frequency_;

    struct
    {
        struct sockaddr_in      addr_any_ip4;               /**< 0.0.0.0:0 */
        struct sockaddr_in6     addr_any_ip6;               /**< :::0 */
        char                    zero_[1];                   /**< A zero length buffer */
    }net;

    struct
    {
        ev_tls_t                thread_key;                 /**< Thread handle */
    }thread;
}ev_loop_win_ctx_t;

extern ev_loop_win_ctx_t        g_ev_loop_win_ctx;          /**< Global runtime for Windows */

/**
 * @brief Initialize windows context.
 */
API_LOCAL void ev__init_once_win(void);

/**
 * @brief Initialize IOCP request
 * @param[out] req      A pointer to the IOCP request
 * @param[in] callback  A callback when the request is finish
 * @param[in] arg       User defined argument passed to callback
 */
API_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg);

/**
 * @brief Post to specific IOCP request.
 * @param[in] loop      Event loop
 * @param[in] req       IOCP request
 */
API_LOCAL void ev__iocp_post(ev_loop_t* loop, ev_iocp_t* req);

/**
 * @brief Convert typeof NTSTATUS error to typeof WinSock error
 * @param[in] status  NTSTATUS error
 * @return WinSock error
 */
API_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status);

/**
 * @brief Set \p sock as reusable address.
 * @param[in] sock  Socket to set reusable.
 * @param[in] opt   0 if not reusable, otherwise reusable.
 * @return          #ev_errnot_t
 */
API_LOCAL int ev__reuse_win(SOCKET sock, int opt);

/**
 * @brief Set \p sock as IPv6 only.
 * @param[in] sock  Socket to set IPv6 only.
 * @param[in] opt   0 if IPv4 available, otherwise IPv6 only.
 * @return          #ev_errnot_t
 */
API_LOCAL int ev__ipv6only_win(SOCKET sock, int opt);

/**
 * @brief Maps a character string to a UTF-16 (wide character) string.
 * @param[out] dst  Pointer to store wide string. Use #ev__free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src);

/**
 * @brief Maps a UTF-16 (wide character) string to a character string.
 * @param[out] dst  Pointer to store wide string. Use #ev__free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src);

#ifdef __cplusplus
}
#endif
#endif
