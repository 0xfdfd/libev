#ifndef __EV_TCP_H__
#define __EV_TCP_H__

#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TCP TCP
 * @{
 */

/**
 * @brief TCP socket.
 */
struct ev_tcp
{
    ev_handle_t             base;               /**< Base object */
    ev_tcp_close_cb         close_cb;           /**< User close callback */

    ev_os_socket_t          sock;               /**< Socket handle */
    ev_tcp_backend_t        backend;            /**< Platform related implementation */
};
/**
 * @brief Initialize #ev_tcp_t to an invalid value.
 * @see ev_tcp_init()
 */
#define EV_TCP_INVALID      \
    {\
        EV_HANDLE_INVALID,\
        NULL,\
        EV_OS_SOCKET_INVALID,\
        EV_TCP_BACKEND_INIT,\
    }

/**
 * @brief Initialize a tcp socket
 * @param[in] loop      Event loop
 * @param[out] tcp      TCP handle
 */
int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp);

/**
 * @brief Destroy socket
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 */
void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb);

/**
 * @brief Bind the handle to an address and port.
 * addr should point to an initialized struct sockaddr_in or struct sockaddr_in6.
 * @param[in] tcp       Socket handler
 * @param[in] addr      Bind address
 * @param[in] addrlen   Address length
 * @return              #ev_errno_t
 */
int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen);

/**
 * @brief Start listening for incoming connections.
 * @param[in] sock      Listen socket
 * @param[in] backlog   The number of connections the kernel might queue
 * @return              #ev_errno_t
 */
int ev_tcp_listen(ev_tcp_t* sock, int backlog);

/**
 * @brief Accept a connection from listen socket
 * @param[in] acpt  Listen socket
 * @param[in] conn  The socket to store new connection
 * @param[in] cb    Accept callback
 * @return          #ev_errno_t
 */
int ev_tcp_accept(ev_tcp_t* acpt, ev_tcp_t* conn, ev_accept_cb cb);

/**
 * @brief Connect to address
 * @param[in] sock  Socket handle
 * @param[in] addr  Address
 * @param[in] size  Address size
 * @param[in] cb    Connect callback
 * @return          #ev_errno_t
 */
int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb);

/**
 * @brief Write data
 * 
 * Once #ev_tcp_write() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 * 
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If write success or failure. The callback will be called with write status.
 *   + If \p pipe is exiting but there are pending write request. The callback
 *     will be called with status #EV_ECANCELED.
 * 
 * @param[in] sock  Socket handle
 * @param[in] req   Write request
 * @return          #ev_errno_t
 */
int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req);

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
 * @param[in] req   Read request
 * @return          #ev_errno_t
 */
int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req);

/**
 * @brief Get the current address to which the socket is bound.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @brief Get the address of the peer connected to the socket.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @} EV_TCP
 */

#ifdef __cplusplus
}
#endif

#endif
