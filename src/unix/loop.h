#ifndef __EV_LOOP_UNIX_H__
#define __EV_LOOP_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"

#include <sys/epoll.h>
#define EV_IO_IN		EPOLLIN		/**< The associated file is available for read(2) operations. */
#define EV_IO_OUT		EPOLLOUT	/**< The associated file is available for write(2) operations. */

#define EV_INVALID_FD	-1			/**< Invalid file descriptor */

/**
 * @brief Initialize io structure
 * @param[out] io	A pointer to the structure
 * @param[in] fd	File descriptor
 * @param[in] cb	IO active callback
 */
void ev__io_init(ev_io_t* io, int fd, ev_io_cb cb);

/**
 * @brief Add events to IO structure
 * @param[in] loop	Event loop
 * @param[in] io	IO structure
 * @param[in] evts	#EV_IO_IN or #EV_IO_OUT
 */
void ev__io_add(ev_loop_t* loop, ev_io_t* io, unsigned evts);

/**
 * @brief Delete events from IO structure
 * @param[in] loop	Event loop
 * @param[in] io	IO structure
 * @param[in] evts	#EV_IO_IN or #EV_IO_OUT
 */
void ev__io_del(ev_loop_t* loop, ev_io_t* io, unsigned evts);

/**
 * @brief Add or remove FD_CLOEXEC
 * @param[in] fd	File descriptor
 * @param[in] set	bool
 * @return			#ev_errno_t
 */
int ev__cloexec(int fd, int set);

/**
 * @brief Add or remove O_NONBLOCK
 * @param[in] fd	File descriptor
 * @param[in] set	bool
 * @return			#ev_errno_t
 */
int ev__nonblock(int fd, int set);

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr	System error
 * @return				#ev_errno_t
 */
int ev__translate_sys_error_unix(int syserr);

#ifdef __cplusplus
}
#endif
#endif
