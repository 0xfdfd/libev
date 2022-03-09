#ifndef __EV_STREAM_UNIX_H__
#define __EV_STREAM_UNIX_H__

#include "unix/loop.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
#endif
