#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <windows.h>

struct ev_once
{
	INIT_ONCE	guard;
};
#define EV_ONCE_INIT	{ INIT_ONCE_STATIC_INIT }

typedef struct ev_loop_plt
{
	HANDLE		iocp;
}ev_loop_plt_t;

#ifdef __cplusplus
}
#endif
#endif
