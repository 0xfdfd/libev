#ifndef __EV_PIPE_H__
#define __EV_PIPE_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PIPE Pipe
 * @{
 */

enum ev_pipe_flags_e
{
    EV_PIPE_READABLE    = 0x01, /**< Pipe is readable */
    EV_PIPE_WRITABLE    = 0x02, /**< Pipe is writable */
    EV_PIPE_NONBLOCK    = 0x04, /**< Pipe is nonblock */
    EV_PIPE_IPC         = 0x08, /**< Enable IPC */
};

/**
 * @brief Typedef of #ev_pipe_flags_e.
 */
typedef enum ev_pipe_flags_e ev_pipe_flags_t;

struct ev_pipe;

/**
 * @brief Typedef of #ev_pipe.
 */
typedef struct ev_pipe ev_pipe_t;

struct ev_pipe_write_req;

/**
 * @brief Typedef of #ev_pipe_write_req.
 */
typedef struct ev_pipe_write_req ev_pipe_write_req_t;

struct ev_pipe_read_req;

/**
 * @brief Typedef of #ev_pipe_read_req.
 */
typedef struct ev_pipe_read_req ev_pipe_read_req_t;

/**
 * @brief Callback for #ev_pipe_t
 * @param[in] handle      A pipe
 */
typedef void(*ev_pipe_cb)(ev_pipe_t* handle);

/**
 * @brief Write callback
 * @param[in] req       Write request
 * @param[in] result    Write result
 */
typedef void(*ev_pipe_write_cb)(ev_pipe_write_req_t* req, ssize_t result);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] result    Read result
 */
typedef void(*ev_pipe_read_cb)(ev_pipe_read_req_t* req, ssize_t result);

/**
 * @brief IPC frame header.
 *
 * Frame layout:
 *  [LOW ADDR] | ------------------------ |
 *             | Frame header             | -> 16 bytes
 *             | ------------------------ |
 *             | Information              | -> #ev_ipc_frame_hdr_t::hdr_exsz
 *             | ------------------------ |
 *             | Data                     | -> #ev_ipc_frame_hdr_t::hdr_dtsz
 * [HIGH ADDR] | ------------------------ |
 *
 * Frame header layout:
 *  -------------------------------------------------------------------------
 *  | 00     | 01     | 02     | 03     | 04     | 05     | 06     | 07     |
 *  -------------------------------------------------------------------------
 *  | MAGIC                             | FLAGS  | VER.   | INFO SIZE       |
 *  -------------------------------------------------------------------------
 *  | 0x45   | 0x56   | 0x46   | 0x48   |I       | 0x00   | native endian   |
 *  -------------------------------------------------------------------------
 *  -------------------------------------------------------------------------
 *  | 08     | 09     | 10     | 11     | 12     | 13     | 14     | 15     |
 *  -------------------------------------------------------------------------
 *  | DATA SIZE                         | RESERVED                          |
 *  -------------------------------------------------------------------------
 *  | native endian                     | 0x00   | 0x00   | 0x00   | 0x00   |
 *  -------------------------------------------------------------------------
 * 
 * FLAG layout (8 bits) :
 * | bit  | 0                   | 1                 |
 * | ---- | ------------------- | ----------------- |
 * | [00] | without information | have information  |
 */
typedef struct ev_ipc_frame_hdr
{
    uint32_t    hdr_magic;      /**< Magic code */
    uint8_t     hdr_flags;      /**< Bit OR flags */
    uint8_t     hdr_version;    /**< Protocol version */
    uint16_t    hdr_exsz;       /**< Extra data size */
    uint32_t    hdr_dtsz;       /**< Data size */
    uint32_t    reserved;       /**< Zeros */
}ev_ipc_frame_hdr_t;

/**
 * @brief PIPE
 */
struct ev_pipe
{
    ev_handle_t             base;               /**< Base object */
    ev_pipe_cb              close_cb;           /**< User close callback */

    ev_os_pipe_t            pipfd;              /**< Pipe handle */
    EV_PIPE_BACKEND         backend;            /**< Platform related implementation */
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
    EV_PIPE_WRITE_BACKEND   backend;            /**< Backend */
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
    EV_PIPE_READ_BACKEND    backend;            /**< Backend */
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
EV_API int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe, int ipc);

/**
 * @brief Destroy pipe
 * @param[in] pipe      Pipe handle.
 * @param[in] cb        Destroy callback
 */
EV_API void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb);

/**
 * @brief Open an existing file descriptor or HANDLE as a pipe.
 * @note The pipe must have established connection.
 * @param[in] pipe      Pipe handle
 * @param[in] handle    File descriptor or HANDLE
 * @return              #ev_errno_t
 */
EV_API int ev_pipe_open(ev_pipe_t* pipe, ev_os_pipe_t handle);

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
EV_API int ev_pipe_write(ev_pipe_t* pipe, ev_pipe_write_req_t* req, ev_buf_t* bufs,
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
EV_API int ev_pipe_write_ex(ev_pipe_t* pipe, ev_pipe_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_role_t handle_role, void* handle_addr,
    size_t handle_size, ev_pipe_write_cb cb);

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
EV_API int ev_pipe_read(ev_pipe_t* pipe, ev_pipe_read_req_t* req, ev_buf_t* bufs,
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
EV_API int ev_pipe_accept(ev_pipe_t* pipe, ev_pipe_read_req_t* req,
    ev_role_t handle_role, void* handle_addr, size_t handle_size);

/**
 * @brief Make a pair of pipe.
 *
 * Close pipe by #ev_pipe_close() when no longer need it.
 *
 * @note #EV_PIPE_READABLE and #EV_PIPE_WRITABLE are silently ignored.
 * @note If pipe is create for IPC usage, both \p rflags and \p wflags must
 *   have #EV_PIPE_IPC set. Only set one of \p rflags or \p wflags will return
 *   #EV_EINVAL.
 * 
 * @param[out] fds      fds[0] for read, fds[1] for write
 * @param[in] rflags    Bit-OR of #ev_pipe_flags_t for read pipe.
 * @param[in] wflags    Bit-OR of #ev_pipe_flags_t for write pipe.
 * @return          #ev_errno_t
 */
EV_API int ev_pipe_make(ev_os_pipe_t fds[2], int rflags, int wflags);

/**
 * @brief Close OS pipe.
 * @param[in] fd    pipe create by #ev_pipe_make().
 */
EV_API void ev_pipe_close(ev_os_pipe_t fd);

/**
 * @} EV_PIPE
 */

#ifdef __cplusplus
}
#endif
#endif
