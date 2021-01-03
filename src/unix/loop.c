#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include "ev-platform.h"
#include "loop.h"

static clockid_t g_hwtime_clock_id = CLOCK_MONOTONIC;

static void _ev_init_hwtime(void)
{
	struct timespec t;
	if (clock_getres(CLOCK_MONOTONIC_COARSE, &t) != 0)
	{
		return;
	}
	if (t.tv_nsec > 1 * 1000 * 1000)
	{
		return;
	}
	g_hwtime_clock_id = CLOCK_MONOTONIC_COARSE;
}

void ev__loop_update_time(ev_loop_t* loop)
{
	static ev_once_t token = EV_ONCE_INIT;
	ev_once_execute(&token, _ev_init_hwtime);

	struct timespec t;
	if (clock_gettime(g_hwtime_clock_id, &t) != 0)
	{
		return;
	}

	loop->hwtime = t.tv_sec * (uint64_t)1e9 + t.tv_nsec;
}

int ev__cloexec(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
	int r;

	do
	{
		r = ioctl(fd, set ? FIOCLEX : FIONCLEX);
	} while (r == -1 && errno == EINTR);

	if (r)
	{
		return errno;
	}

	return EV_SUCCESS;
#else
	int flags;
	int r;

	do
	{
		r = fcntl(fd, F_GETFD);
	} while (r == -1 && errno == EINTR);

	if (r == -1)
	{
		return errno;
	}

	/* Bail out now if already set/clear. */
	if (!!(r & FD_CLOEXEC) == !!set)
	{
		return EV_SUCCESS;
	}

	if (set)
	{
		flags = r | FD_CLOEXEC;
	}
	else
	{
		flags = r & ~FD_CLOEXEC;
	}

	do
	{
		r = fcntl(fd, F_SETFD, flags);
	} while (r == -1 && errno == EINTR);

	if (r)
	{
		return errno;
	}

	return EV_SUCCESS;
#endif
}

int ev__nonblock(int fd, int set)
{
#if defined(_AIX) || \
    defined(__APPLE__) || \
    defined(__DragonFly__) || \
    defined(__FreeBSD__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__linux__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
	int r;

	do
	{
		r = ioctl(fd, FIONBIO, &set);
	} while (r == -1 && errno == EINTR);

	if (r)
	{
		return errno;
	}

	return EV_SUCCESS;
#else
	int flags;
	int r;

	do
	{
		r = fcntl(fd, F_GETFL);
	} while (r == -1 && errno == EINTR);

	if (r == -1)
	{
		return errno;
	}

	/* Bail out now if already set/clear. */
	if (!!(r & O_NONBLOCK) == !!set)
	{
		return EV_SUCCESS;
	}

	if (set)
	{
		flags = r | O_NONBLOCK;
	}
	else
	{
		flags = r & ~O_NONBLOCK;
	}

	do
	{
		r = fcntl(fd, F_SETFL, flags);
	} while (r == -1 && errno == EINTR);

	if (r)
	{
		return errno;
	}

	return EV_SUCCESS;
#endif
}

static int _ev_cmp_io_unix(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
	(void)arg;
	ev_io_t* io1 = container_of(key1, ev_io_t, node);
	ev_io_t* io2 = container_of(key2, ev_io_t, node);
	return io1->data.fd - io2->data.fd;
}

int ev__loop_init_backend(ev_loop_t* loop)
{
	ev_map_init(&loop->backend.io, _ev_cmp_io_unix, NULL);

	if ((loop->backend.pollfd = epoll_create1(EPOLL_CLOEXEC)) != -1)
	{
		return EV_SUCCESS;
	}

	if (errno != ENOSYS && errno != EINVAL)
	{
		return errno;
	}

	if ((loop->backend.pollfd = epoll_create(256)) == -1)
	{
		return errno;
	}

	int ret;
	if ((ret = ev__cloexec(loop->backend.pollfd, 1)) != 0)
	{
		close(loop->backend.pollfd);
		loop->backend.pollfd = -1;
		return ret;
	}

	return EV_SUCCESS;
}

void ev__loop_exit_backend(ev_loop_t* loop)
{
	close(loop->backend.pollfd);
	loop->backend.pollfd = -1;
}

void ev__io_init(ev_io_t* io, int fd, ev_io_cb cb)
{
	io->data.fd = fd;
	io->data.c_events = 0;
	io->data.n_events = 0;
	io->data.cb = cb;
}

void ev__io_add(ev_loop_t* loop, ev_io_t* io, unsigned evts)
{
	struct epoll_event poll_event;

	io->data.n_events |= evts;
	if (io->data.n_events == io->data.c_events)
	{
		return;
	}

	memset(&poll_event, 0, sizeof(poll_event));
	poll_event.events = io->data.n_events;
	poll_event.data.fd = io->data.fd;

	int op = io->data.c_events == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

	if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
	{
		ABORT();
	}

	io->data.c_events = io->data.n_events;
	if (op == EPOLL_CTL_ADD)
	{
		ev_map_insert(&loop->backend.io, &io->node);
	}
}

void ev__io_del(ev_loop_t* loop, ev_io_t* io, unsigned evts)
{
	struct epoll_event poll_event;
	io->data.n_events &= ~evts;
	if (io->data.n_events == io->data.c_events)
	{
		return;
	}

	memset(&poll_event, 0, sizeof(poll_event));
	poll_event.events = io->data.n_events;
	poll_event.data.fd = io->data.fd;

	int op = io->data.n_events == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

	if (epoll_ctl(loop->backend.pollfd, op, io->data.fd, &poll_event) != 0)
	{
		ABORT();
	}

	io->data.c_events = io->data.n_events;
	if (op == EPOLL_CTL_DEL)
	{
		ev_map_erase(&loop->backend.io, &io->node);
	}
}

static ev_io_t* _ev_find_io(ev_loop_t* loop, int fd)
{
	ev_io_t tmp;
	tmp.data.fd = fd;

	ev_map_node_t* it = ev_map_find(&loop->backend.io, &tmp.node);
	return it != NULL ? container_of(it, ev_io_t, node) : NULL;
}

static int _ev_poll_once(ev_loop_t* loop, struct epoll_event* events, int maxevents, int timeout)
{
	int nfds = epoll_wait(loop->backend.pollfd, events, maxevents, timeout);
	if (nfds < 0)
	{
		return nfds;
	}

	int i;
	for (i = 0; i < nfds; i++)
	{
		ev_io_t* io = _ev_find_io(loop, events[i].data.fd);
		io->data.cb(io, events[i].events);
	}

	return nfds;
}

void ev__poll(ev_loop_t* loop, uint32_t timeout)
{
	int nevts;
	struct epoll_event events[64];

	/**
	 * A bug in kernels < 2.6.37 makes timeouts larger than ~30 minutes
	 * effectively infinite on 32 bits architectures.  To avoid blocking
	 * indefinitely, we cap the timeout and poll again if necessary.
	 *
	 * Note that "30 minutes" is a simplification because it depends on
	 * the value of CONFIG_HZ.  The magic constant assumes CONFIG_HZ=1200,
	 * that being the largest value I have seen in the wild (and only once.)
	 */
	const uint32_t max_safe_timeout = 1789569;

	/**
	 * from libuv, this value gives the best throughput.
	 */
	int max_performance_events = 49152;

	const uint64_t base_time = loop->hwtime;
	const uint32_t user_timeout = timeout;
	for (; max_performance_events != 0; max_performance_events--)
	{
		if (timeout > max_safe_timeout)
		{
			timeout = max_safe_timeout;
		}

		nevts = _ev_poll_once(loop, events, ARRAY_SIZE(events), timeout);

		if (nevts == ARRAY_SIZE(events))
		{/* Poll for more events but don't block this time. */
			timeout = 0;
			continue;
		}

		if (nevts >= 0)
		{
			break;
		}

		/* If errno is not EINTR, something must wrong in the program */
		if (errno != EINTR)
		{
			ABORT();
		}

		ev__loop_update_time(loop);
		uint64_t pass_time = loop->hwtime - base_time;
		if (pass_time >= user_timeout)
		{
			break;
		}

		timeout = user_timeout - pass_time;
	}
}

int ev__translate_sys_error(int syserr)
{
	return syserr;
}
