#ifndef __EV_TCP_WIN_INTERNAL_H__
#define __EV_TCP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tcp_ctx
{
    struct sockaddr_in      addr_any_ip4;   /**< 0.0.0.0:0 */
    struct sockaddr_in6     addr_any_ip6;   /**< :::0 */
}tcp_ctx_t;

extern tcp_ctx_t            g_tcp_ctx;      /**< Global TCP context */

/**
 * @brief Initialize TCP
 */
void ev__tcp_init(void);

#ifdef __cplusplus
}
#endif
#endif
