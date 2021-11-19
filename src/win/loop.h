#ifndef __EV_LOOP_WIN_INTERNAL_H__
#define __EV_LOOP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"
#include "ev-platform.h"
#include "winapi.h"

/**
 * @brief Initialize IOCP request
 * @param[out] req      A pointer to the IOCP request
 * @param[in] cb        A callback when the request is finish
 */
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req      A write request to be initialized
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer list size
 * @param[in] owner     Who own this object
 * @param[in] stat      Initial status
 * @param[in] iocp_cb   IOCP completion callback
 * @param[in] w_cb      Write complete callback
 */
void ev__write_init_win(ev_write_t* req, ev_buf_t bufs[], size_t nbuf, void* owner, int stat, ev_iocp_cb iocp_cb, ev_write_cb w_cb);

int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif
#endif
