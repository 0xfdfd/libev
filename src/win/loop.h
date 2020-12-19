#ifndef __EV_WIN_LOOP_H__
#define __EV_WIN_LOOP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "ev-common.h"

/**
 * @brief Initialize a handle
 * @param[in] loop		The loop own the handle
 * @param[out] handle	A pointer to the structure
 * @param[in] close_cb	A callback when handle is closed
 */
void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_close_cb close_cb);

/**
 * @brief Close the handle
 * @note The handle will not closed until close_cb was called, which was given
 *   by #ev__handle_init()
 * @param[in] handle	handler
 */
void ev__handle_exit(ev_handle_t* handle);

/**
 * @brief Set handle as active
 * @param[in] handle	handler
 */
void ev__handle_active(ev_handle_t* handle);

/**
 * @brief Set handle as inactive
 * @param[in] handle	handler
 */
void ev__handle_deactive(ev_handle_t* handle);

/**
 * @brief Check if the handle is in active state
 * @param[in] handle	handler
 * @return				bool
 */
int ev__handle_is_active(ev_handle_t* handle);

/**
 * @brief Check if the handle is in closing or closed state
 * @param[in] handle	handler
 * @return				bool
 */
int ev__handle_is_closing(ev_handle_t* handle);

/**
 * @brief Add a pending task
 * @param[in] loop		Event loop
 * @param[in] token		A pointer to the pending token
 * @param[in] cb		A callback when the pending task is active
 */
void ev__todo(ev_loop_t* loop, ev_todo_t* token, ev_todo_cb cb);

/**
 * @brief Initialize IOCP request
 * @param[out] req		A pointer to the IOCP request
 * @param[in] cb		A callback when the request is finish
 */
void ev__iocp_init(ev_iocp_t* req, ev_iocp_cb cb);

#ifdef __cplusplus
}
#endif
#endif
