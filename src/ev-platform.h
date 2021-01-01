#ifndef __EV_PLATFORM_INTERNAL_H__
#define __EV_PLATFORM_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#	include <WS2tcpip.h>
#else
#	include <arpa/inet.h>
#endif

#include "ev.h"

int ev__loop_init_backend(ev_loop_t* loop);
void ev__loop_exit_backend(ev_loop_t* loop);

void ev__loop_update_time(ev_loop_t* loop);

void ev__poll(ev_loop_t* loop, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif
