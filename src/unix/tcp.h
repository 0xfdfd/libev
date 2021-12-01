#ifndef __EV_TCP_H__
#define __EV_TCP_H__

#include "ev-common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open fd for read/write.
 * @param[in] tcp   TCP handle
 * @param[in] fd    fd
 * @return          #ev_errno_t
 */
API_LOCAL int ev__tcp_open(ev_tcp_t* tcp, int fd);

#ifdef __cplusplus
}
#endif
#endif
