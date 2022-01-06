#ifndef __EV_UDP_H__
#define __EV_UDP_H__

#include "ev/request.h"
#include "ev/udp_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_UDP UDP
 * @{
 */

typedef enum ev_udp_membership 
{
    EV_UDP_LEAVE_GROUP  = 0,    /**< Leave multicast group */
    EV_UDP_ENTER_GROUP  = 1,    /**< Join multicast group */
} ev_udp_membership_t;

typedef enum ev_udp_flags
{
    EV_UDP_IPV6_ONLY    = 1,    /**< Do not bound to IPv4 address */
    EV_UDP_REUSEADDR    = 2,    /**< Reuse address. Only the last one can receive message. */
}ev_udp_flags_t;

struct ev_udp
{
    ev_handle_t             base;               /**< Base object */
    ev_udp_cb               close_cb;           /**< Close callback */
    ev_os_socket_t          sock;               /**< OS socket */

    ev_list_t               send_list;          /**< Send queue */
    ev_list_t               recv_list;          /**< Recv queue */

    ev_udp_backend_t        backend;            /**< Platform related implementation */
};

struct ev_udp_write
{
    ev_write_t              base;               /**< Base request */
    ev_udp_write_cb         usr_cb;             /**< User callback */
    ev_udp_write_backend_t  backend;            /**< Backend */
};

struct ev_udp_read
{
    ev_read_t               base;               /**< Base request */
    ev_udp_recv_cb          usr_cb;             /**< User callback */
    struct sockaddr_storage addr;               /**< Peer address */
    EV_UDP_READ_BACKEND                         /**< Backend */
};

/**
 * @brief Initialize a UDP handle.
 * @param[in] loop      Event loop
 * @param[out] udp      A UDP handle to initialize
 * @param[in] domain    AF_INET / AF_INET6 / AF_UNSPEC
 * @return              #ev_errno_t
 */
int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain);

/**
 * @brief Close UDP handle
 * @param[in] udp       A UDP handle
 * @param[in] close_cb  Close callback
 */
void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb);

/**
 * @brief Open a existing UDP socket
 * @note \p udp must be a initialized handle
 * @param[in] udp       A initialized UDP handle
 * @param[in] sock      A system UDP socket
 * @return              #ev_errno_t
 */
int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock);

/**
 * @brief Bind the UDP handle to an IP address and port.
 * @param[in] udp       A UDP handle
 * @param[in] addr      struct sockaddr_in or struct sockaddr_in6 with the
 *   address and port to bind to.
 * @param[in] flags     #ev_udp_flags_t
 * @return              #ev_errno_t
 */
int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags);

/**
 * @brief Associate the UDP handle to a remote address and port, so every message
 *   sent by this handle is automatically sent to that destination.
 * @param[in] udp       A UDP handle
 * @param[in] addr      Remote address
 * @return              #ev_errno_t
 */
int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr);

/**
 * @brief Get the local IP and port of the UDP handle.
 * @param[in] udp       A UDP handle
 * @param[out] name     Pointer to the structure to be filled with the address data.
 *   In order to support IPv4 and IPv6 struct sockaddr_storage should be used.
 * @param[in,out] len   On input it indicates the data of the name field.
 *   On output it indicates how much of it was filled.
 * @return              #ev_errno_t
 */
int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len);

/**
 * @brief Get the remote IP and port of the UDP handle on connected UDP handles.
 * @param[in] udp       A UDP handle
 * @param[out] name     Pointer to the structure to be filled with the address data.
 *   In order to support IPv4 and IPv6 struct sockaddr_storage should be used.
 * @param[in,out] len   On input it indicates the data of the name field.
 *   On output it indicates how much of it was filled.
 * @return              #ev_errno_t
 */
int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len);

/**
 * @brief Set membership for a multicast address.
 * @param[in] udp               A UDP handle
 * @param[in] multicast_addr    Multicast address to set membership for.
 * @param[in] interface_addr    Interface address.
 * @param[in] membership        #ev_udp_membership_t
 * @return                      #ev_errno_t
 */
int ev_udp_set_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership);

/**
 * @brief Set membership for a source-specific multicast group.
 * @param[in] udp               A UDP handle
 * @param[in] multicast_addr    Multicast address to set membership for.
 * @param[in] interface_addr    Interface address.
 * @param[in] source_addr       Source address.
 * @param[in] membership        #ev_udp_membership_t
 * @return                      #ev_errno_t
 */
int ev_udp_set_source_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, const char* source_addr, ev_udp_membership_t membership);

/**
 * @brief Set IP multicast loop flag. Makes multicast packets loop back to local sockets.
 * @param[in] udp   A UDP handle
 * @param[in] on    bool
 * @return          #ev_errno_t
 */
int ev_udp_set_multicast_loop(ev_udp_t* udp, int on);

/**
 * @brief Set the multicast ttl.
 * @param[in] udp   A UDP handle
 * @param[in] ttl   1 through 255
 * @return          #ev_errno_t
 */
int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl);

/**
 * @brief Set the multicast interface to send or receive data on.
 * @param[in] udp               A UDP handle
 * @param[in] interface_addr    interface address.
 * @return                      #ev_errno_t
 */
int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr);

/**
 * @brief Set broadcast on or off.
 * @param[in] udp   A UDP handle
 * @param[in] on    1 for on, 0 for off
 * @return          #ev_errno_t
 */
int ev_udp_set_broadcast(ev_udp_t* udp, int on);

/**
 * @brief Set the time to live.
 * @param[in] udp   A UDP handle
 * @param[in] ttl   1 through 255.
 * @return          #ev_errno_t
 */
int ev_udp_set_ttl(ev_udp_t* udp, int ttl);

/**
 * @brief Send data over the UDP socket.
 *
 * If the socket has not previously been bound with #ev_udp_bind() it will be bound
 * to 0.0.0.0 (the "all interfaces" IPv4 address) and a random port number.
 *
 * @param[in] udp   A UDP handle
 * @param[in] req   Write request token
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] addr  Peer address
 * @param[in] cb    Send result callback
 * @return          #ev_errno_t
 */
int ev_udp_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs, size_t nbuf,
    const struct sockaddr* addr, ev_udp_write_cb cb);

/**
 * @brief Same as #ev_udp_send(), but won't queue a send request if it can't be
 *   completed immediately.
 * @param[in] udp   A UDP handle
 * @param[in] req   Write request token
 * @param[in] addr  Peer address
 * @return          #ev_errno_t
 */
int ev_udp_try_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs, size_t nbuf,
    const struct sockaddr* addr, ev_udp_write_cb cb);

/**
 * @brief Queue a read request.
 * @param[in] udp   A UDP handle
 * @param[in] req   Read request token
 * @param[in] bufs  Receive buffer
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Receive callback
 * @return          #ev_errno_t
 */
int ev_udp_recv(ev_udp_t* udp, ev_udp_read_t* req, ev_buf_t* bufs, size_t nbuf,
    ev_udp_recv_cb cb);

/**
 * @} EV_UDP
 */

#ifdef __cplusplus
}
#endif
#endif
