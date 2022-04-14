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

/**
 * @brief Typedef of #ev_tcp.
 */
typedef struct ev_tcp ev_tcp_t;

struct ev_tcp_read_req;

/**
 * @brief Typedef of #ev_tcp_read_req.
 */
typedef struct ev_tcp_read_req ev_tcp_read_req_t;

struct ev_tcp_write_req;

/**
 * @brief Typedef of #ev_tcp_write_req.
 */
typedef struct ev_tcp_write_req ev_tcp_write_req_t;

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
typedef void(*ev_tcp_accept_cb)(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);

/**
 * @brief Connect callback
 * @param[in] sock      Connect socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_tcp_connect_cb)(ev_tcp_t* sock, int stat);

/**
 * @brief Write callback
 * @param[in] req       Write request token
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void (*ev_tcp_write_cb)(ev_tcp_write_req_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void (*ev_tcp_read_cb)(ev_tcp_read_req_t* req, size_t size, int stat);

/**
 * @} EV_TCP
 */

#ifdef __cplusplus
}
#endif
#endif
