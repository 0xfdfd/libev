#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct ev_once
{
	INIT_ONCE	guard;
}ev_once_t;
#define EV_ONCE_INIT	{ INIT_ONCE_STATIC_INIT }

#ifdef __cplusplus
}
#endif
#endif
