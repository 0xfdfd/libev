#ifndef __EV_LOOP_WIN_H__
#define __EV_LOOP_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "ev-common.h"

/**
 * @brief Initialize IOCP request
 * @param[out] req		A pointer to the IOCP request
 * @param[in] cb		A callback when the request is finish
 */
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

#ifdef __cplusplus
}
#endif
#endif
