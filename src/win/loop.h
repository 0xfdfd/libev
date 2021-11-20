#ifndef __EV_LOOP_WIN_INTERNAL_H__
#define __EV_LOOP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "winapi.h"

/**
 * @brief Initialize IOCP request
 * @param[out] req      A pointer to the IOCP request
 * @param[in] callback  A callback when the request is finish
 * @param[in] arg       User defined argument passed to callback
 */
API_LOCAL void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb callback, void* arg);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req      A write request to be initialized
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer list size
 * @param[in] owner     Who own this object
 * @param[in] stat      Initial status
 * @param[in] iocp_cb   IOCP completion callback
 * @param[in] w_cb      Write complete callback
 * @return              #ev_errno_t
 */
API_LOCAL int ev__write_init_win(ev_write_t* req, ev_buf_t bufs[], size_t nbuf,
    void* owner, int stat, ev_iocp_cb iocp_cb, void* iocp_arg, ev_write_cb w_cb);

/**
 * @brief Cleanup #ev_write_t
 */
API_LOCAL void ev__write_exit_win(ev_write_t* req);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req      A read request to be initialized
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer list size
 * @param[in] owner     Who own this object
 * @param[in] stat      Initial status
 * @param[in] iocp_cb   IOCP completion callback
 * @param[in] w_cb      Write complete callback
 * @return              #ev_errno_t
 */
API_LOCAL int ev__read_init_win(ev_read_t* req, ev_buf_t bufs[], size_t nbuf,
    void* owner, int stat, ev_iocp_cb iocp_cb, void* iocp_arg, ev_read_cb r_cb);

/**
 * @brief Cleanup #ev_read_t
 */
API_LOCAL void ev__read_exit_win(ev_read_t* req);

API_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif
#endif
