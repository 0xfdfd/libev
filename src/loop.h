#ifndef __EV_LOOP_INTERNAL_H__
#define __EV_LOOP_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev/loop.h"
#include "ev/handle.h"
#include "ev/buf.h"
#include "ev/request.h"
#include "defs.h"

typedef enum ev_ipc_frame_flag
{
    EV_IPC_FRAME_FLAG_INFORMATION = 1,
}ev_ipc_frame_flag_t;

/**
 * @brief Get event loop for the handle.
 * @param[in] handle    handler
 * @return              Event loop
 */
API_LOCAL ev_loop_t* ev__handle_loop(ev_handle_t* handle);

/**
 * @brief Check IPC frame header
 * @param[in] buffer    Buffer to check
 * @param[in] size      Buffer size
 * @return              bool
 */
API_LOCAL int ev__ipc_check_frame_hdr(const void* buffer, size_t size);

/**
 * @brief Initialize IPC frame header
 * @param[out] hdr      Frame header to initialize
 * @param[in] flags     Control flags
 * @param[in] exsz      Extra information size
 * @param[in] dtsz      Data size
 */
API_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t* hdr,
    uint8_t flags, uint16_t exsz, uint32_t dtsz);

/**
 * @brief Update loop time
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_update_time(ev_loop_t* loop);

/**
 * @brief Get minimal length of specific \p addr type.
 * @param[in] addr  A valid sockaddr buffer
 * @return          A valid minimal length, or (socklen_t)-1 if error.
 */
API_LOCAL socklen_t ev__get_addr_len(const struct sockaddr* addr);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
API_LOCAL int ev__write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup write request
 * @param[in] req   Write request
 */
API_LOCAL void ev__write_exit(ev_write_t* req);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
API_LOCAL int ev__read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf);

/**
 * @brief Cleanup read request
 * @param[in] req   read request
 */
API_LOCAL void ev__read_exit(ev_read_t* req);

/**
 * @brief Initialize backend
 * @param[in] loop      loop handler
 * @return              #ev_errno_t
 */
API_LOCAL int ev__loop_init_backend(ev_loop_t* loop);

/**
 * @brief Destroy backend
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_exit_backend(ev_loop_t* loop);

/**
 * @brief Wait for IO event and process
 * @param[in] loop  loop handler
 * @param[in] timeout   timeout in milliseconds
 */
API_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout);

/**
 * @brief Get clocktime
 * @return      Clock time
 */
API_LOCAL uint64_t ev__clocktime(void);

/**
 * @brief Same as abort(3)
 */
API_LOCAL EV_NORETURN void ev__abort(const char* file, int line);

#ifdef __cplusplus
}
#endif
#endif
