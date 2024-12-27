#ifndef __EV_UDP_INTERNAL_H__
#define __EV_UDP_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read request token for UDP socket.
 */
typedef struct ev_udp_read
{
    ev_handle_t             handle;     /**< Base object */
    ev_read_t               base;       /**< Base request */
    ev_udp_recv_cb          usr_cb;     /**< User callback */
    void                   *usr_cb_arg; /**< User defined argument */
    struct sockaddr_storage addr;       /**< Peer address */
    EV_UDP_READ_BACKEND     backend;    /**< Backend */
} ev_udp_read_t;

/**
 * @brief Write request token for UDP socket.
 */
typedef struct ev_udp_write
{
    ev_handle_t          handle;     /**< Base object */
    ev_write_t           base;       /**< Base request */
    ev_udp_write_cb      usr_cb;     /**< User callback */
    void                *usr_cb_arg; /**< User defined argument */
    EV_UDP_WRITE_BACKEND backend;    /**< Backend */
} ev_udp_write_t;

struct ev_udp
{
    ev_handle_t    base;      /**< Base object */
    ev_udp_cb      close_cb;  /**< Close callback */
    void          *close_arg; /**< User defined argument */
    ev_os_socket_t sock;      /**< OS socket */

    ev_list_t send_list; /**< Send queue */
    ev_list_t recv_list; /**< Recv queue */

    EV_UDP_BACKEND backend; /**< Platform related implementation */
};

/**
 * @brief Convert \p interface_addr to \p dst.
 * @param[out] dst              A buffer to store address.
 * @param[in] interface_addr    Interface address
 * @param[in] is_ipv6           Whether a IPv6 address. Only valid if \p
 * interface_addr is NULL.
 * @return                      #ev_errno_t
 */
EV_LOCAL int ev__udp_interface_addr_to_sockaddr(struct sockaddr_storage *dst,
                                                const char *interface_addr,
                                                int         is_ipv6);

/**
 * @brief Queue a UDP receive request
 * @param[in] udp   UDP handle
 * @param[in] req   Receive request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_recv(ev_udp_t *udp, ev_udp_read_t *req);

/**
 * @brief Queue a UDP Send request
 * @param[in] udp   UDP handle
 * @param[in] req   Send request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_send(ev_udp_t *udp, ev_udp_write_t *req,
                          const struct sockaddr *addr, socklen_t addrlen);

#ifdef __cplusplus
}
#endif
#endif
