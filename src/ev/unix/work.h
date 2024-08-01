#ifndef __EV_WORK_INTERNAL_H__
#define __EV_WORK_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

EV_LOCAL void ev__init_work(ev_loop_t* loop);

EV_LOCAL void ev__exit_work(ev_loop_t* loop);

#ifdef __cplusplus
}
#endif
#endif
