#ifndef __EV_ASYNC_UNIX_INTERNAL_H__
#define __EV_ASYNC_UNIX_INTERNAL_H__

#include "async.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a pair of eventfd.
 * Index 0 for read, index 1 for write.
 */
API_LOCAL int ev__asyc_eventfd(int evtfd[2]);

/**
 * @brief Post event to eventfd.
 */
API_LOCAL void ev__async_post(int wfd);

/**
 * @brief Pend event from eventfd.
 */
API_LOCAL void ev__async_pend(int rfd);

#ifdef __cplusplus
}
#endif

#endif
