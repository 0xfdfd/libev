#ifndef __EV_PIPE_H__
#define __EV_PIPE_H__

#include "ev/request.h"
#include "ev/pipe_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_PIPE
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
        EV_PIPE_BACKEND_INVALID,\
    }

/**
 * @brief Write request token for pipe.
 */
struct ev_pipe_write_req
{
    ev_write_t              base;               /**< Base object */
    ev_pipe_write_cb        ucb;                /**< User callback */
    struct
    {
        ev_role_t           role;               /**< The type of handle to send */
        union
        {
            ev_os_socket_t  os_socket;          /**< A EV handle instance */
        }u;
    }handle;
    ev_pipe_write_backend_t backend;            /**< Backend */
};

/**
 * @brief Read request token for pipe.
 */
struct ev_pipe_read_req
{
    ev_read_t               base;               /**< Base object */
    ev_pipe_read_cb         ucb;                /**< User callback */
    struct
    {
        ev_os_socket_t      os_socket;          /**< Socket handler */
    }handle;
    ev_pipe_read_backend_t  backend;            /**< Backend */
};

/**
 * @brief Initialize a pipe handle.
 *
 * A pipe can be initialized as `IPC` mode, which is a special mode for
 * inter-process communication. The `IPC` mode have following features:
 * 1. You can transfer system resource (like a tcp socket or pipe handle) in
 *   pipe.
 * 2. The data in pipe is datagrams (like a UDP socket). Each block of data
 *   transfer in pipe will be package as a special designed data frame, so you
 *   don't need to manually split data.
 *
 * @warning Due to special features of `IPC` mode, it is significantly slower
 *   than normal mode, so don't use `IPC` mode to transmit large data.
 *
 * @param[in] loop      Event loop
 * @param[out] pipe     Pipe handle
 * @param[in] ipc       Initialize as IPC mode.
 * @return              #ev_errno_t
 */
int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc);

/**
 * @brief Destroy pipe
 * @param[in] pipe      Pipe handle.
 * @param[in] cb        Destroy callback
 */
void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb);

/**
 * @brief Open an existing file descriptor or HANDLE as a pipe.
 * @note The pipe must have established connection.
 * @param[in] pipe      Pipe handle
 * @param[in] handle    File descriptor or HANDLE
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
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Write result callback
 * @return          #ev_errno_t
 */
int ev_pipe_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_write_cb cb);

/**
 * @brief Like #ev_pipe_write(), with following difference:
 * 
 * + It has the ability to send OS resource to peer side.
 * + It is able to handle large amount of \p nbuf event it is larger than
 *   #EV_IOV_MAX.
 * 
 * @param[in] pipe          Pipe handle
 * @param[in] req           Write request
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer number
 * @param[in] handle_role   The type of handle to send
 * @param[in] handle_addr   The address of handle to send
 * @param[in] handle_size   The size of handle to send
 * @param[in] cb            Write result callback
 * @return                  #ev_errno_t
 */
int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf,
    ev_role_t handle_role, void* handle_addr, size_t handle_size,
    ev_pipe_write_cb cb);

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
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Receive callback
 * @return          #ev_errno_t
 */
int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_pipe_read_cb cb);

/**
 * @brief Accept handle from peer.
 * @param[in] pipe          Pipe handle.
 * @param[in] req           Read request.
 * @param[in] handle_role   Handle type.
 * @param[in] handle_addr   Handle address.
 * @param[in] handle_size   Handle size.
 * @return  #EV_SUCCESS: Operation success.
 * @return  #EV_EINVAL: \p pipe is not initialized with IPC, or \p handle_role is
 *              not support, or \p handle_addr is NULL.
 * @return  #EV_ENOENT: \p req does not receive a handle.
 * @return  #EV_ENOMEM: \p handle_size is too small.
 */
int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

/**
 * @brief Make a pair of pipe.
 *
 * Close pipe by #ev_pipe_close() when no longer need it.
 *
 * @param[out] fds  fds[0] for read, fds[1] for write
 * @return          #ev_errno_t
 */
int ev_pipe_make(ev_os_pipe_t fds[2]);

/**
 * @brief Close OS pipe.
 * @param[in] fd    pipe create by #ev_pipe_make().
 */
void ev_pipe_close(ev_os_pipe_t fd);

/**
 * @} EV_PIPE
 */

#ifdef __cplusplus
}
#endif
#endif
