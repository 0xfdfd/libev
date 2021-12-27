/**
 * @file
 */
#ifndef __EV_H__
#define __EV_H__

#include <stddef.h>
#include <stdarg.h>
#include "ev/errno.h"
#include "ev/defs.h"
#include "ev/list.h"
#include "ev/todo.h"
#include "ev/mutex.h"
#include "ev/sem.h"
#include "ev/thread.h"
#include "ev/shmem.h"
#include "ev/loop.h"
#include "ev/timer.h"
#include "ev/async.h"
#include "ev/threadpool.h"
#include "ev/tcp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate minimum size of iov storage.
 * @param[in] nbuf  The number of iov buffers
 * @return          The size of iov storage in bytes
 */
#define EV_IOV_BUF_SIZE(nbuf)   EV_IOV_BUF_SIZE_INTERNAL(nbuf)

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

/**
 * @brief Write request
 */
struct ev_write
{
    ev_list_node_t          node;               /**< Intrusive node */
    struct
    {
        ev_write_cb         cb;                 /**< Write complete callback */
        ev_buf_t*           bufs;               /**< Buffer list */
        size_t              nbuf;               /**< Buffer list count */
        size_t              size;               /**< Write size */
        size_t              capacity;           /**< Total bytes need to send */
        ev_buf_t            bufsml[EV_IOV_MAX]; /**< Bound buffer list */
    }data;
    struct
    {
        ev_role_t           role;               /**< The type of handle to send */
        union
        {
            ev_os_socket_t  os_socket;          /**< A EV handle instance */
        }u;
    }handle;
    ev_write_backend_t      backend;            /**< Back-end */
};
#define EV_WRITE_INVALID    \
    {\
        EV_LIST_NODE_INVALID,                                          /* .node */\
        {/* .data */\
            NULL,                                                   /* .data.cb */\
            NULL,                                                   /* .data.bufs */\
            0,                                                      /* .data.nbuf */\
            0,                                                      /* .data.size */\
            0,                                                      /* .data.capacity */\
            { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), }   /* .data.bufsml */\
        },\
        {/* .handle */\
            EV_ROLE_UNKNOWN,                                        /* .handle.role */\
            { EV_OS_SOCKET_INVALID }                                /* .handle.u.os_socket */\
        },\
        EV_WRITE_BACKEND_INIT                                       /* .backend */\
    }

/**
 * @brief Read request
 */
struct ev_read
{
    ev_list_node_t          node;               /**< Intrusive node */
    struct
    {
        ev_read_cb          cb;                 /**< Read complete callback */
        ev_buf_t*           bufs;               /**< Buffer list */
        size_t              nbuf;               /**< Buffer list count */
        size_t              capacity;           /**< Total bytes of buffer */
        size_t              size;               /**< Data size */
        ev_buf_t            bufsml[EV_IOV_MAX]; /**< Bound buffer list */
    }data;
    struct
    {
        ev_os_socket_t      os_socket;
    }handle;
    ev_read_backend_t       backend;            /**< Back-end */
};
#define EV_READ_INVALID     \
    {\
        EV_LIST_NODE_INVALID,/* .node */\
        {/* .data */\
            NULL,                                                   /* .data.cb */\
            NULL,                                                   /* .data.bufs */\
            0,                                                      /* .data.nbuf */\
            0,                                                      /* .data.capacity */\
            0,                                                      /* .data.size */\
            { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), },  /* .data.bufsml */\
        },\
        {\
            EV_OS_SOCKET_INVALID,                                   /* .handle.os_socket */\
        },\
        EV_READ_BACKEND_INIT\
    }

/**
 * @defgroup EV_PIPE Pipe
 * @{
 */

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
int ev_pipe_write(ev_pipe_t* pipe, ev_write_t* req);

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
 * @} EV_PIPE
 */

/**
 * @defgroup EV_UTILS Miscellaneous utilities
 * @{
 */

/**
 * @defgroup EV_UTILS_NET Network
 * @{
 */

/**
 * @brief Convert ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr);

/**
 * @brief Convert ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len);

/**
 * @} EV_UTILS/EV_UTILS_NET
 */

/**
 * @brief Executes the specified function one time.
 *
 * No other threads that specify the same one-time initialization structure can
 * execute the specified function while it is being executed by the current thread.
 *
 * @param[in] guard     A pointer to the one-time initialization structure.
 * @param[in] cb        A pointer to an application-defined InitOnceCallback function.
 */
void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Write complete callback
 * @return          #ev_errno_t
 */
int ev_write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf, ev_write_cb cb);

/**
 * @brief Initialize #ev_write_t
 * 
 * The extend version of #ev_write_init(). You should use this function if any
 * of following condition is meet:
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
int ev_write_init_ext(ev_write_t* req, ev_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Read complete callback
 * @return          #ev_errno_t
 */
int ev_read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf, ev_read_cb cb);

/**
 * @brief Initialize #ev_write_t
 *
 * The extend version of #ev_write_init(). You should use this function if any
 * of following condition is meet:
 *
 *   + The value of \p nbuf is larger than #EV_IOV_MAX.<br>
 *     In this case you should pass \p iov_bufs as storage, the minimum value of
 *     \p iov_size can be calculated by #EV_IOV_BUF_SIZE(). \p take the ownership
 *     of \p iov_bufs, so you cannot modify or free \p iov_bufs until \p callback
 *     is called.
 *
 * @param[out] req          A write request to be initialized
 * @param[in] callback      Write complete callback
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer list size
 * @param[in] iov_bufs      The buffer to store IOV request
 * @param[in] iov_size      The size of \p iov_bufs in bytes
 * @return                  #ev_errno_t
 */
int ev_read_init_ext(ev_read_t* req, ev_read_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size);

/**
 * @brief Constructor for #ev_buf_t
 * @param[in] buf   Buffer
 * @param[in] len   Buffer length
 * @return          A buffer
 */
ev_buf_t ev_buf_make(void* buf, size_t len);

/**
 * @brief Constructor for #ev_buf_t list
 * 
 * Example:
 * @code
 * void foo_bar(void)
 * {
 *     ev_buf_t bufs[2];
 * 
 *     void* buf_1 = some_address;
 *     size_t len_1 = length_of_buf_1;
 * 
 *     void* buf_2 = some_address;
 *     size_t len_2 = length_of_buf_2;
 * 
 *     ev_buf_make_n(bufs, 2, buf_1, len_1, buf_2, len_2);
 * }
 * @endcode
 *
 * @param[out] bufs Buffer array
 * @param[in] nbuf  Buffer number
 * @param[in] ...   Buffer info, must a pair of (void*, size_t)
 */
void ev_buf_make_n(ev_buf_t bufs[], size_t nbuf, ...);

/**
 * @brief Constructor for #ev_buf_t list
 * 
 * Like #ev_buf_make_n(), but accept a va_list for argument.
 * @param[out] bufs Buffer array
 * @param[in] nbuf  Buffer number
 * @param[in] ap    va_list for Buffer array
 */
void ev_buf_make_v(ev_buf_t bufs[], size_t nbuf, va_list ap);

/**
 * @} EV_UTILS
 */

#ifdef __cplusplus
}
#endif
#endif
