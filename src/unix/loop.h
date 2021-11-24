#ifndef __EV_LOOP_UNIX_H__
#define __EV_LOOP_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"

#include <sys/epoll.h>
#define EV_IO_IN            EPOLLIN     /**< The associated file is available for read(2) operations. */
#define EV_IO_OUT           EPOLLOUT    /**< The associated file is available for write(2) operations. */

typedef struct ev_loop_unix_ctx
{
    clockid_t   hwtime_clock_id;    /**< Clock id */
    int         iovmax;             /**< The limits instead of readv/writev */
}ev_loop_unix_ctx_t;

/**
 * @brief Global libev runtime
 */
extern ev_loop_unix_ctx_t g_ev_loop_unix_ctx;

/**
 * @brief Initialize io structure
 * @param[out] io   A pointer to the structure
 * @param[in] fd    File descriptor
 * @param[in] cb    IO active callback
 */
API_LOCAL void ev__nonblock_io_init(ev_nonblock_io_t* io, int fd, ev_nonblock_io_cb cb);

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
 * @brief Initialize stream.
 * @param[in] loop      Event loop
 * @param[out] stream   Stream handler
 * @param[in] fd        File descriptor
 * @param[in] wcb       Write callback
 * @param[in] rcb       Read callback
 */
API_LOCAL void ev__nonblock_stream_init(ev_loop_t* loop, ev_nonblock_stream_t* stream,
    int fd, ev_stream_write_cb wcb, ev_stream_read_cb rcb);

/**
 * @brief Cleanup and exit stream
 * @param[in] stream    Stream handler
 */
API_LOCAL void ev__nonblock_stream_exit(ev_nonblock_stream_t* stream);

/**
 * @brief Do stream write
 * @param[in] stream    Stream handle
 * @param[in] req       Write request
 * @return              #ev_errno_t
 */
API_LOCAL int ev__nonblock_stream_write(ev_nonblock_stream_t* stream, ev_write_t* req);

/**
 * @brief Do stream read
 * @param[in] stream    Stream handle
 * @param[in] req       Read request
 * @return              #ev_errno_t
 */
API_LOCAL int ev__nonblock_stream_read(ev_nonblock_stream_t* stream, ev_read_t* req);

/**
 * @brief Get pending action count.
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 * @return              Action count
 */
API_LOCAL size_t ev__nonblock_stream_size(ev_nonblock_stream_t* stream, unsigned evts);

/**
 * @brief Abort pending requests
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 */
API_LOCAL void ev__nonblock_stream_abort(ev_nonblock_stream_t* stream);

/**
 * @brief Cleanup pending requests
 * @param[in] stream    Stream handle
 * @param[in] evts      #EV_IO_IN or #EV_IO_OUT
 */
API_LOCAL void ev__nonblock_stream_cleanup(ev_nonblock_stream_t* stream, unsigned evts);

API_LOCAL void ev__write_init_unix(ev_write_t* req);

API_LOCAL void ev__read_init_unix(ev_read_t* req);

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
 * @brief Return the file access mode and the file status flags
 */
API_LOCAL int ev__getfl(int fd);

/**
 * @brief Return the file descriptor flags.
 */
API_LOCAL int ev__getfd(int fd);

#ifdef __cplusplus
}
#endif
#endif
