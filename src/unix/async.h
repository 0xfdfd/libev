#ifndef __EV_ASYNC_UNIX_H__
#define __EV_ASYNC_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

int ev__async_init_loop(ev_loop_t* loop);

void ev__async_exit_loop(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif
#endif
