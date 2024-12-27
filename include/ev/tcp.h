#ifndef __EV_TCP_H__
#define __EV_TCP_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TCP TCP
 *
 * TCP layer.
 *
 * @{
 */

/**
 * @example tcp_echo_server.c
 * This is an example for how to use #ev_tcp_t as tcp server.
 */

/**
 * @brief TCP socket.
 */
typedef struct ev_tcp ev_tcp_t;

/**
 * @brief Close callback for #ev_tcp_t
 * @param[in] sock      A closed socket
 * @param[in] arg       User defined argument.
 */
typedef void (*ev_tcp_close_cb)(ev_tcp_t *sock, void *arg);

/**
 * @brief Accept callback
 * @param[in] lisn      Listen socket
 * @param[in] conn      Accepted socket
 * @param[in] stat      #ev_errno_t
 * @param[in] arg       User defined argument.
 */
typedef void (*ev_tcp_accept_cb)(ev_tcp_t *lisn, ev_tcp_t *conn, int stat,
                                 void *arg);

/**
 * @brief Connect callback
 * @param[in] sock      Connect socket
 * @param[in] stat      #ev_errno_t
 */
typedef void (*ev_tcp_connect_cb)(ev_tcp_t *sock, int stat, void *arg);

/**
 * @brief Write callback
 * @param[in] sock      Socket.
 * @param[in] size      Write result
 * @param[in] arg       User defined argument.
 */
typedef void (*ev_tcp_write_cb)(ev_tcp_t *sock, ssize_t size, void *arg);

/**
 * @brief Read callback
 * @param[in] sock      Socket.
 * @param[in] size      Read result.
 * @param[in] arg       User defined argument.
 */
typedef void (*ev_tcp_read_cb)(ev_tcp_t *sock, ssize_t size, void *arg);

/**
 * @brief Initialize a tcp socket
 * @param[in] loop      Event loop
 * @param[out] tcp      TCP handle
 */
EV_API int ev_tcp_init(ev_loop_t *loop, ev_tcp_t **tcp);

/**
 * @brief Destroy socket
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 * @param[in] arg       User defined argument.
 */
EV_API void ev_tcp_exit(ev_tcp_t *sock, ev_tcp_close_cb cb, void *arg);

/**
 * @brief Bind the handle to an address and port.
 * addr should point to an initialized struct sockaddr_in or struct
 * sockaddr_in6.
 * @param[in] tcp       Socket handler
 * @param[in] addr      Bind address
 * @param[in] addrlen   Address length
 * @return              #ev_errno_t
 */
EV_API int ev_tcp_bind(ev_tcp_t *tcp, const struct sockaddr *addr,
                       size_t addrlen);

/**
 * @brief Start listening for incoming connections.
 * @param[in] sock      Listen socket
 * @param[in] backlog   The number of connections the kernel might queue
 * @return              #ev_errno_t
 */
EV_API int ev_tcp_listen(ev_tcp_t *sock, int backlog);

/**
 * @brief Accept a connection from listen socket
 * @param[in] acpt          Listen socket
 * @param[in] conn          The socket to store new connection
 * @param[in] cb            Accept callback
 * @param[in] arg           User defined argument pass to \p cb.
 * @return                  #ev_errno_t
 */
EV_API int ev_tcp_accept(ev_tcp_t *acpt, ev_tcp_t *conn, ev_tcp_accept_cb cb,
                         void *arg);

/**
 * @brief Connect to address
 * @param[in] sock          Socket handle
 * @param[in] addr          Address
 * @param[in] size          Address size
 * @param[in] cb            Connect callback
 * @param[in] arg           Connect argument.
 * @return                  #ev_errno_t
 */
EV_API int ev_tcp_connect(ev_tcp_t *sock, struct sockaddr *addr, size_t size,
                          ev_tcp_connect_cb cb, void *arg);

/**
 * @brief Write data
 *
 * Once #ev_tcp_write() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 *
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If write success or failure. The callback will be called with write
 * status.
 *   + If \p pipe is exiting but there are pending write request. The callback
 *     will be called with status #EV_ECANCELED.
 *
 * @param[in] sock      Socket handle
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer number
 * @param[in] cb        Send result callback
 * @param[in] arg       User defined argument.
 * @return              #ev_errno_t
 */
EV_API int ev_tcp_write(ev_tcp_t *sock, ev_buf_t *bufs, size_t nbuf,
                        ev_tcp_write_cb cb, void *arg);

/**
 * @brief Read data
 *
 * Once #ev_tcp_read() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 *
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If read success or failure. The callback will be called with read status.
 *   + If \p pipe is exiting but there are pending read request. The callback
 *     will be called with status #EV_ECANCELED.
 *
 * @param[in] sock  Socket handle
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Read result callback
 * @param[in] arg   User defined argument.
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_read(ev_tcp_t *sock, ev_buf_t *bufs, size_t nbuf,
                       ev_tcp_read_cb cb, void *arg);

/**
 * @brief Get the current address to which the socket is bound.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_getsockname(ev_tcp_t *sock, struct sockaddr *name,
                              size_t *len);

/**
 * @brief Get the address of the peer connected to the socket.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_getpeername(ev_tcp_t *sock, struct sockaddr *name,
                              size_t *len);

/**
 * @} EV_TCP
 */

#ifdef __cplusplus
}
#endif
#endif
