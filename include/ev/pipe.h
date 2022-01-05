#ifndef __EV_PIPE_H__
#define __EV_PIPE_H__

#include "ev/request.h"
#include "ev/pipe_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PIPE Pipe
 * @{
 */

/**
 * @brief PIPE
 */
struct ev_pipe
{
    ev_handle_t             base;               /**< Base object */
    ev_pipe_cb              close_cb;           /**< User close callback */

    ev_os_pipe_t            pipfd;              /**< Pipe handle */
    ev_pipe_backend_t       backend;            /**< Platform related implementation */
};
/**
 * @brief Initialize #ev_pipe_t to an invalid value.
 * @see ev_pipe_init()
 */
#define EV_PIPE_INVALID     \
    {\
        EV_HANDLE_INVALID,\
        NULL,\
        EV_OS_PIPE_INVALID,\
        EV_PIPE_BACKEND_INIT,\
    }

struct ev_pipe_write_req
{
    ev_write_t              base;               /**< Base object */
    struct
    {
        ev_role_t           role;               /**< The type of handle to send */
        union
        {
            ev_os_socket_t  os_socket;          /**< A EV handle instance */
        }u;
    }handle;
};

/**
 * @brief Initialize a pipe handle.
 * @param[in] loop      Event loop
 * @param[out] handle   Pipe handle
 * @return              #ev_errno_t
 */
int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc);

/**
 * @brief Destroy pipe
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 */
void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb);

/**
 * @brief Open an existing file descriptor or HANDLE as a pipe.
 * @note The pipe must have established connection.
 * @param[in] handle    Pipe handle
 * @param[in] file      File descriptor or HANDLE
 * @return              #ev_errno_t
 */
int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle);

/**
 * @brief Write data
 *
 * Once #ev_pipe_write() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 *
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If write success or failure. The callback will be called with write status.
 *   + If \p pipe is exiting but there are pending write request. The callback
 *     will be called with status #EV_ECANCELED.
 *
 * @param[in] pipe  Pipe handle
 * @param[in] req   Write request
 * @return          #ev_errno_t
 */
int ev_pipe_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req);

/**
 * @brief Read data
 * 
 * Once #ev_pipe_read() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 * 
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If read success or failure. The callback will be called with read status.
 *   + If \p pipe is exiting but there are pending read request. The callback
 *     will be called with status #EV_ECANCELED.
 * 
 * @param[in] pipe  Pipe handle
 * @param[in] req   Read request
 * @return          #ev_errno_t
 */
int ev_pipe_read(ev_pipe_t* pipe, ev_read_t* req);

/**
 * @return
 *   + #EV_SUCCESS: Operation success.
 *   + #EV_EINVAL: \p pipe is not initialized with IPC, or \p handle_role is not
 *     support, or \p handle_addr is NULL.
 *   + #EV_ENOENT: \p req does not receive a handle.
 *   + #EV_ENOMEM: \p handle_size is too small.
 */
int ev_pipe_accept(ev_pipe_t* pipe, ev_read_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

/**
 * @brief Make a pair of pipe
 * @param[out] fds  fds[0] for read, fds[1] for write
 * @return          #ev_errno_t
 */
int ev_pipe_make(ev_os_pipe_t fds[2]);

/**
 * @brief Initialize #ev_pipe_write_req_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Write complete callback
 * @return          #ev_errno_t
 */
int ev_pipe_write_init(ev_pipe_write_req_t* req, ev_buf_t* bufs, size_t nbuf, ev_write_cb cb);

/**
 * @brief Initialize #ev_pipe_write_req_t
 *
 * The extend version of #ev_pipe_write_init(). You should use this function if
 * any of following condition is meet:
 *
 *   + The value of \p nbuf is larger than #EV_IOV_MAX.<br>
 *     In this case you should pass \p iov_bufs as storage, the minimum value of
 *     \p iov_size can be calculated by #EV_IOV_BUF_SIZE(). \p take the ownership
 *     of \p iov_bufs, so you cannot modify or free \p iov_bufs until \p callback
 *     is called.
 *
 *   + Need to transfer a handle to peer.<br>
 *     In this case you should set the type of handle via \p handle_role and pass
 *     the address of the handle via \p handle_addr. \p req does not take the ownership
 *     of the handle, but the handle should not be closed or destroy until \p callback
 *     is called.
 *
 * @param[out] req          A write request to be initialized
 * @param[in] callback      Write complete callback
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer list size
 * @param[in] iov_bufs      The buffer to store IOV request
 * @param[in] iov_size      The size of \p iov_bufs in bytes
 * @param[in] handle_role   The type of handle to send
 * @param[in] handle_addr   The address of handle to send
 * @param[in] handle_size   The size of handle to send
 * @return                  #ev_errno_t
 */
int ev_pipe_write_init_ext(ev_pipe_write_req_t* req, ev_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

/**
 * @} EV_PIPE
 */

#ifdef __cplusplus
}
#endif
#endif
