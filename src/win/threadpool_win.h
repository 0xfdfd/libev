#ifndef __EV_THREADPOOL_WIN_INTERNAL_H__
#define __EV_THREADPOOL_WIN_INTERNAL_H__

#include "threadpool.h"

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL void ev__threadpool_init_win(ev_loop_t* loop);
API_LOCAL void ev__threadpool_exit_win(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
