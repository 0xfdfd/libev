#ifndef __EV_MISC_INTERNAL_H__
#define __EV_MISC_INTERNAL_H__

#include "ev.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr    System error
 * @return              #ev_errno_t
 */
API_LOCAL int ev__translate_sys_error(int syserr);

#ifdef __cplusplus
}
#endif

#endif
