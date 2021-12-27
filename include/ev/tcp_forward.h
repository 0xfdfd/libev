#ifndef __EV_TCP_FORWARD_H__
#define __EV_TCP_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TCP TCP
 * @{
 */

struct ev_tcp;
typedef struct ev_tcp ev_tcp_t;

/**
 * @brief Close callback for #ev_tcp_t
 * @param[in] sock      A closed socket
 */
typedef void(*ev_tcp_close_cb)(ev_tcp_t* sock);

/**
 * @brief Accept callback
 * @param[in] lisn      Listen socket
 * @param[in] conn      Accepted socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_accept_cb)(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);

/**
 * @brief Connect callback
 * @param[in] sock      Connect socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_connect_cb)(ev_tcp_t* sock, int stat);

/**
 * @} EV_TCP
 */

#ifdef __cplusplus
}
#endif
#endif
