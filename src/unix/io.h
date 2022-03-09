#ifndef __EV_IO_H__
#define __EV_IO_H__

#include "unix/loop.h"
#include <sys/epoll.h>
#define EV_IO_IN            EPOLLIN     /**< The associated file is available for read(2) operations. */
#define EV_IO_OUT           EPOLLOUT    /**< The associated file is available for write(2) operations. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize io structure
 * @param[out] io   A pointer to the structure
 * @param[in] fd    File descriptor
 * @param[in] cb    IO active callback
 * @param[in] arg   User data
 */
API_LOCAL void ev__nonblock_io_init(ev_nonblock_io_t* io, int fd, ev_nonblock_io_cb cb, void* arg);

/**
 * @brief Add events to IO structure
 * @param[in] loop  Event loop
 * @param[in] io    IO structure
 * @param[in] evts  #EV_IO_IN or #EV_IO_OUT
 */
API_LOCAL void ev__nonblock_io_add(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts);

/**
 * @brief Delete events from IO structure
 * @param[in] loop  Event loop
 * @param[in] io    IO structure
 * @param[in] evts  #EV_IO_IN or #EV_IO_OUT
 */
API_LOCAL void ev__nonblock_io_del(ev_loop_t* loop, ev_nonblock_io_t* io, unsigned evts);

/**
 * @brief Add or remove FD_CLOEXEC
 * @param[in] fd    File descriptor
 * @param[in] set   bool
 * @return          #ev_errno_t
 */
API_LOCAL int ev__cloexec(int fd, int set);

/**
 * @brief Add or remove O_NONBLOCK
 * @param[in] fd    File descriptor
 * @param[in] set   bool
 * @return          #ev_errno_t
 */
API_LOCAL int ev__nonblock(int fd, int set);

/**
 * @brief Set reuse
 * @return          #ev_errno_t
 */
API_LOCAL int ev__reuse_unix(int fd);

/**
 * @brief Return the file access mode and the file status flags
 */
API_LOCAL int ev__fcntl_getfl_unix(int fd);

/**
 * @brief Return the file descriptor flags.
 */
API_LOCAL int ev__fcntl_getfd_unix(int fd);

/**
 * @brief readv wrap
 * @return 0: try again; >0: read size; <0 errno
 */
API_LOCAL ssize_t ev__readv_unix(int fd, ev_buf_t* iov, int iovcnt);

/**
 * @brief readv wrap
 * @return 0: try again; >0: write size; <0 errno
 */
API_LOCAL ssize_t ev__writev_unix(int fd, ev_buf_t* iov, int iovcnt);

/**
 * @brief write
 * @return 0: try again; >0: write size; <0 errno
 */
API_LOCAL ssize_t ev__write_unix(int fd, void* buffer, size_t size);

/**
 * @brief Write \p req to \p fd
 * @param[in] fd    File to write
 * @param[in] req   Write request
 * @param[in] do_write  Write function
 * @param[in] arg       User defined data
 * @return              + #EV_SUCCESS: \p req send finish
 *                      + #EV_EAGAIN: \p req not send finish, need to try again
 *                      + other value: error
 */
API_LOCAL int ev__send_unix(int fd, ev_write_t* req,
    ssize_t(*do_write)(int fd, struct iovec* iov, int iovcnt, void* arg), void* arg);

#ifdef __cplusplus
}
#endif
#endif
