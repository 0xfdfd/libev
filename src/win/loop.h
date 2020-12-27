#ifndef __EV_LOOP_WIN_H__
#define __EV_LOOP_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"

/**
 * @brief Initialize IOCP request
 * @param[out] req		A pointer to the IOCP request
 * @param[in] cb		A callback when the request is finish
 */
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr	System error
 * @return				#ev_errno_t
 */
int ev__translate_sys_error_win(int syserr);

#ifdef __cplusplus
}
#endif
#endif
