#ifndef __EV_UNIX_H__
#define __EV_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

struct ev_once
{
	pthread_once_t	guard;
};
#define EV_ONCE_INIT	{ PTHREAD_ONCE_INIT }

typedef struct ev_loop_plt
{
	int				pollfd;
}ev_loop_plt_t;

#ifdef __cplusplus
}
#endif
#endif
