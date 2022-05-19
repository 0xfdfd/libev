#ifndef __EV_TODO_INTERNAL_H__
#define __EV_TODO_INTERNAL_H__

#include "ev/todo.h"
#include "ev-platform.h"

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL void ev__process_todo(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif
