#ifndef __EV_UNIX_H__
#define __EV_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct ev_once
{
	pthread_once_t	guard;
}ev_once_t;
#define EV_ONCE_INIT	{ PTHREAD_ONCE_INIT }

#ifdef __cplusplus
}
#endif
#endif
