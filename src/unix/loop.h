#ifndef __EV_WIN_UNIX_H__
#define __EV_WIN_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "ev-common.h"

#include <sys/epoll.h>
#define EV_IO_IN	EPOLLIN
#define EV_IO_OUT	EPOLLOUT

void ev__io_init(ev_io_t* io, int fd, ev_io_cb cb);
void ev__io_add(ev_loop_t* loop, ev_io_t* io, unsigned evts);
void ev__io_del(ev_loop_t* loop, ev_io_t* io, unsigned evts);

int ev__cloexec(int fd, int set);
int ev__nonblock(int fd, int set);

#ifdef __cplusplus
}
#endif
#endif
