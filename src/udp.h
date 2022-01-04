#ifndef __EV_UDP_INTERNAL_H__
#define __EV_UDP_INTERNAL_H__

#include "ev-common.h"

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
API_LOCAL int ev__udp_interface_addr_to_sockaddr(struct sockaddr_storage* dst,
        const char* interface_addr, int is_ipv6);

#ifdef __cplusplus
}
#endif
#endif
