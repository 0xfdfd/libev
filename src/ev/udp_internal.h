#ifndef __EV_UDP_INTERNAL_H__
#define __EV_UDP_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert \p interface_addr to \p dst.
 * @param[out] dst              A buffer to store address.
 * @param[in] interface_addr    Interface address
 * @param[in] is_ipv6           Whether a IPv6 address. Only valid if \p interface_addr is NULL.
 * @return                      #ev_errno_t
 */
EV_LOCAL int ev__udp_interface_addr_to_sockaddr(struct sockaddr_storage* dst,
        const char* interface_addr, int is_ipv6);

/**
 * @brief Queue a UDP receive request
 * @param[in] udp   UDP handle
 * @param[in] req   Receive request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req);

/**
 * @brief Queue a UDP Send request
 * @param[in] udp   UDP handle
 * @param[in] req   Send request
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req,
    const struct sockaddr* addr, socklen_t addrlen);

#ifdef __cplusplus
}
#endif
#endif
