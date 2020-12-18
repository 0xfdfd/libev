#ifndef __EV_WIN_LOOP_H__
#define __EV_WIN_LOOP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "ev-common.h"

void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle);
void ev__handle_exit(ev_handle_t* handle);
void ev__todo(ev_loop_t* loop, ev_todo_t* todo, ev_todo_cb cb);
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

#ifdef __cplusplus
}
#endif
#endif
