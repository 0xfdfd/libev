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
 * @param[out] req		A pointer to the IOCP request
 * @param[in] cb		A callback when the request is finish
 */
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif
#endif
