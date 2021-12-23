/**
 * @file
 */
#ifndef __EV_H__
#define __EV_H__

#include <stddef.h>
#include <stdarg.h>
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate minimum size of iov storage.
 * @param[in] nbuf  The number of iov buffers
 * @return          The size of iov storage in bytes
 */
#define EV_IOV_BUF_SIZE(nbuf)   EV_IOV_BUF_SIZE_INTERNAL(nbuf)

enum ev_errno
{
    EV_SUCCESS          =  0,                   /**< Operation success */

    /* POSIX compatible error code */
    EV_EPERM            = -1,                   /**< Operation not permitted (POSIX.1-2001) */
    EV_ENOENT           = -2,                   /**< No such file or directory (POSIX.1-2001) */
    EV_EIO              = -5,                   /**< Host is unreachable (POSIX.1-2001) */
    EV_E2BIG            = -7,                   /**< Argument list too long (POSIX.1-2001) */
    EV_EBADF            = -9,                   /**< Bad file descriptor (POSIX.1-2001) */
    EV_EAGAIN           = -11,                  /**< Resource temporarily unavailable (POSIX.1-2001) */
    EV_ENOMEM           = -12,                  /**< Not enough space/cannot allocate memory (POSIX.1-2001) */
    EV_EACCES           = -13,                  /**< Permission denied (POSIX.1-2001) */
    EV_EFAULT           = -14,                  /**< Bad address (POSIX.1-2001) */
    EV_EBUSY            = -16,                  /**< Device or resource busy (POSIX.1-2001) */
    EV_EEXIST           = -17,                  /**< File exists (POSIX.1-2001) */
    EV_EXDEV            = -18,                  /**< Improper link (POSIX.1-2001) */
    EV_EISDIR           = -21,                  /**< Is a directory (POSIX.1-2001) */
    EV_EINVAL           = -22,                  /**< Invalid argument (POSIX.1-2001) */
    EV_EMFILE           = -24,                  /**< Too many open files (POSIX.1-2001) */
    EV_ENOSPC           = -28,                  /**< No space left on device (POSIX.1-2001) */
    EV_EROFS            = -30,                  /**< Read-only filesystem (POSIX.1-2001) */
    EV_EPIPE            = -32,                  /**< Broken pipe (POSIX.1-2001) */
    EV_ENAMETOOLONG     = -38,                  /**< Filename too long (POSIX.1-2001) */
    EV_ENOTEMPTY        = -41,                  /**< Directory not empty (POSIX.1-2001) */
    EV_EADDRINUSE       = -100,                 /**< Address already in use (POSIX.1-2001) */
    EV_EADDRNOTAVAIL    = -101,                 /**< Address not available (POSIX.1-2001) */
    EV_EAFNOSUPPORT     = -102,                 /**< Address family not supported (POSIX.1-2001) */
    EV_EALREADY         = -103,                 /**< Connection already in progress (POSIX.1-2001) */
    EV_ECANCELED        = -105,                 /**< Operation canceled (POSIX.1-2001) */
    EV_ECONNABORTED     = -106,                 /**< Connection aborted (POSIX.1-2001) */
    EV_ECONNREFUSED     = -107,                 /**< Connection refused (POSIX.1-2001) */
    EV_ECONNRESET       = -108,                 /**< Connection reset (POSIX.1-2001) */
    EV_EHOSTUNREACH     = -110,                 /**< Host is unreachable (POSIX.1-2001) */
    EV_EINPROGRESS      = -112,                 /**< Operation in progress (POSIX.1-2001) */
    EV_EISCONN          = -113,                 /**< Socket is connected (POSIX.1-2001) */
    EV_ELOOP            = -114,                 /**< Too many levels of symbolic links (POSIX.1-2001) */
    EV_EMSGSIZE         = -115,                 /**< Message too long (POSIX.1-2001) */
    EV_ENETUNREACH      = -118,                 /**< Network unreachable (POSIX.1-2001) */
    EV_ENOBUFS          = -119,                 /**< No buffer space available (POSIX.1 (XSI STREAMS option)) */
    EV_ENOTCONN         = -126,                 /**< The socket is not connected (POSIX.1-2001) */
    EV_ENOTSOCK         = -128,                 /**< Not a socket (POSIX.1-2001) */
    EV_ENOTSUP          = -129,                 /**< Operation not supported (POSIX.1-2001) */
    EV_EPROTONOSUPPORT  = -135,                 /**< Protocol not supported (POSIX.1-2001) */
    EV_ETIMEDOUT        = -138,                 /**< Operation timed out (POSIX.1-2001) */

    /* Extend error code */
    EV_UNKNOWN          = -1001,                /**< Unknown error */
    EV_EOF              = -1002,                /**< End of file */
};

struct ev_tcp
{
    ev_handle_t             base;               /**< Base object */
    ev_tcp_close_cb         close_cb;           /**< User close callback */

    ev_os_socket_t          sock;               /**< Socket handle */
    ev_tcp_backend_t        backend;            /**< Platform related implementation */
};
#define EV_TCP_INIT         { EV_HANDLE_INIT, NULL, EV_OS_SOCKET_INVALID, EV_TCP_BACKEND_INIT }

struct ev_pipe
{
    ev_handle_t             base;               /**< Base object */
    ev_pipe_cb              close_cb;           /**< User close callback */

    ev_os_pipe_t            pipfd;              /**< Pipe handle */
    ev_pipe_backend_t       backend;            /**< Platform related implementation */
};
#define EV_PIPE_INIT        { EV_HANDLE_INIT, NULL, EV_OS_PIPE_INVALID, EV_PIPE_BACKEND_INIT }

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
#define EV_WRITE_INIT       \
    {\
        EV_LIST_NODE_INIT,                                          /* .node */\
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
#define EV_READ_INIT        \
    {\
        EV_LIST_NODE_INIT,/* .node */\
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
 * @defgroup EV_TCP TCP
 * @{
 */

/**
 * @brief Initialize a tcp socket
 * @param[in] loop      Event loop
 * @param[out] tcp      TCP handle
 */
int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp);

/**
 * @brief Destroy socket
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 */
void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb);

/**
 * @brief Bind the handle to an address and port.
 * addr should point to an initialized struct sockaddr_in or struct sockaddr_in6.
 * @param[in] tcp       Socket handler
 * @param[in] addr      Bind address
 * @param[in] addrlen   Address length
 * @return              #ev_errno_t
 */
int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen);

/**
 * @brief Start listening for incoming connections.
 * @param[in] sock      Listen socket
 * @param[in] backlog   The number of connections the kernel might queue
 * @return              #ev_errno_t
 */
int ev_tcp_listen(ev_tcp_t* sock, int backlog);

/**
 * @brief Accept a connection from listen socket
 * @param[in] acpt  Listen socket
 * @param[in] conn  The socket to store new connection
 * @param[in] cb    Accept callback
 * @return          #ev_errno_t
 */
int ev_tcp_accept(ev_tcp_t* acpt, ev_tcp_t* conn, ev_accept_cb cb);

/**
 * @brief Connect to address
 * @param[in] sock  Socket handle
 * @param[in] addr  Address
 * @param[in] size  Address size
 * @param[in] cb    Connect callback
 * @return          #ev_errno_t
 */
int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size, ev_connect_cb cb);

/**
 * @brief Write data
 * 
 * Once #ev_tcp_write() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 * 
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If write success or failure. The callback will be called with write status.
 *   + If \p pipe is exiting but there are pending write request. The callback
 *     will be called with status #EV_ECANCELED.
 * 
 * @param[in] sock  Socket handle
 * @param[in] req   Write request
 * @return          #ev_errno_t
 */
int ev_tcp_write(ev_tcp_t* sock, ev_write_t* req);

/**
 * @brief Read data
 * 
 * Once #ev_tcp_read() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 * 
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If read success or failure. The callback will be called with read status.
 *   + If \p pipe is exiting but there are pending read request. The callback
 *     will be called with status #EV_ECANCELED.
 * 
 * @param[in] sock  Socket handle
 * @param[in] req   Read request
 * @return          #ev_errno_t
 */
int ev_tcp_read(ev_tcp_t* sock, ev_read_t* req);

/**
 * @brief Get the current address to which the socket is bound.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @brief Get the address of the peer connected to the socket.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @} EV_TCP
 */

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
 * @brief Describe the error code
 * @param[in] err   Error code
 * @return          Describe string
 */
const char* ev_strerror(int err);

/**
 * @} EV_UTILS
 */

#ifdef __cplusplus
}
#endif
#endif
