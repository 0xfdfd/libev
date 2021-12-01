#ifndef __EV_WIN_H__
#define __EV_WIN_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT   0x0600
#endif

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "ev/ipc-protocol.h"
#include "ev/map.h"

typedef HANDLE ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      INVALID_HANDLE_VALUE

typedef SOCKET ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    INVALID_SOCKET

/**
 * @brief Buffer
 * @internal Must share the same layout with WSABUF
 */
struct ev_buf
{
    ULONG                       size;               /**< Data size */
    CHAR*                       data;               /**< Data address */
};
#define EV_BUF_INIT(buf, len)   { (ULONG)len, (CHAR*)buf }

struct ev_once
{
    INIT_ONCE                   guard;              /**< Once token */
};
#define EV_ONCE_INIT            { INIT_ONCE_STATIC_INIT }

typedef struct ev_loop_plt
{
    HANDLE                      iocp;               /**< IOCP handle */
}ev_loop_plt_t;
#define EV_LOOP_PLT_INIT        { NULL }

struct ev_iocp;
typedef struct ev_iocp ev_iocp_t;

/**
 * @brief IOCP complete callback
 * @param[in] iocp  IOCP request
 */
typedef void(*ev_iocp_cb)(ev_iocp_t* iocp, size_t transferred, void* arg);

struct ev_iocp
{
    void*                       arg;                /**< Index */
    ev_iocp_cb                  cb;                 /**< Callback */
    OVERLAPPED                  overlapped;         /**< IOCP field */
};
#define EV_IOCP_INIT            { NULL, NULL, { 0, 0, { { 0, 0 } }, NULL } }

typedef struct ev_async_backend
{
    ev_iocp_t                   iocp;               /**< IOCP request */
}ev_async_backend_t;
#define EV_ASYNC_BACKEND_INIT   { EV_IOCP_INIT }

typedef struct ev_write_backend
{
    void*                       owner;              /**< Owner */
    ev_iocp_t*                  io;                 /**< IOCP list */
    int                         stat;               /**< Write result */
    ev_iocp_t                   iosml[EV_IOV_MAX];  /**< IOCP backend */
}ev_write_backend_t;
#define EV_WRITE_BACKEND_INIT   \
    {\
        NULL, NULL, 0,\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_IOCP_INIT), }\
    }

typedef struct ev_read_backend
{
    void*                       owner;              /**< Owner */
    ev_iocp_t*                  io;                 /**< IOCP list */
    int                         stat;               /**< Write result */
    ev_iocp_t                   iosml[EV_IOV_MAX];  /**< IOCP backend */
}ev_read_backend_t;
#define EV_READ_BACKEND_INIT    \
    {\
        NULL, NULL, 0,\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_IOCP_INIT), }\
    }

/**
 * @brief Can be used by #ev_write_backend_t and #ev_read_backend_t
 */
#define EV_IOV_BUF_SIZE_INTERNAL(nbuf)   \
    ((sizeof(ev_iocp_t) + sizeof(ev_buf_t)) * (nbuf))

typedef struct ev_tcp_backend
{
    int                         af;                 /**< AF_INET / AF_INET6 */
    ev_iocp_t                   io;                 /**< IOCP */
    ev_todo_t                   token;              /**< Todo token */

    struct
    {
        unsigned                todo_pending : 1;   /**< Already submit todo request */
    }mask;

    union
    {
        struct
        {
            ev_list_t           a_queue;            /**< (#ev_tcp_t::backend::u::accept::node) Accept queue */
            ev_list_t           a_queue_done;       /**< (#ev_tcp_t::backend::u::accept::node) Accept done queue */
        }listen;
        struct
        {
            ev_accept_cb        cb;                 /**< Accept callback */
            ev_list_node_t      node;               /**< (#ev_tcp_t::backend::u::listen) Accept queue node */
            ev_tcp_t*           listen;             /**< Listen socket */
            int                 stat;               /**< Accept result */
            /**
             * lpOutputBuffer for AcceptEx.
             * dwLocalAddressLength and dwRemoteAddressLength require 16 bytes
             * more than the maximum address length for the transport protocol.
             */
            char                buffer[(sizeof(struct sockaddr_storage) + 16) * 2];
        }accept;
        struct
        {
            ev_connect_cb       cb;                 /**< Callback */
            LPFN_CONNECTEX      fn_connectex;       /**< ConnectEx */
            int                 stat;               /**< Connect result */
        }client;
        struct
        {
            ev_list_t           w_queue;            /**< (#ev_write_t::node) Write queue */
            ev_list_t           w_queue_done;       /**< (#ev_write_t::node) Write done queue */
            ev_list_t           r_queue;            /**< (#ev_read_t::node) Read queue */
            ev_list_t           r_queue_done;       /**< (#ev_read_t::node) Read done queue */
        }stream;
    }u;
}ev_tcp_backend_t;
#define EV_TCP_BACKEND_INIT     { 0, EV_IOCP_INIT, EV_TODO_INIT, { 0 }, { { EV_LIST_INIT, EV_LIST_INIT } } }

typedef enum ev_pipe_win_ipc_info_type
{
    EV_PIPE_WIN_IPC_INFO_TYPE_STATUS,               /**< #ev_pipe_win_ipc_info_t::data::as_status */
    EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO,        /**< #ev_pipe_win_ipc_info_t::data::as_protocol_info */
}ev_pipe_win_ipc_info_type_t;

typedef struct ev_pipe_win_ipc_info
{
    ev_pipe_win_ipc_info_type_t type;               /**< Type */
    union
    {
        struct
        {
            DWORD               pid;                /**< PID */
        }as_status;

        WSAPROTOCOL_INFOW       as_protocol_info;   /**< Protocol info */
    }data;
}ev_pipe_win_ipc_info_t;

typedef union ev_pipe_backend
{
    int                         _useless;           /**< For static initializer */

    struct
    {
        struct
        {
            ev_iocp_t           io;                 /**< IOCP backend */
            ev_list_t           r_pending;          /**< Request queue to be read */
            ev_read_t*          r_doing;            /**< Request queue in reading */
        }rio;

        struct
        {
            struct ev_pipe_backend_data_mode_wio
            {
                size_t          idx;                /**< Index. Must not change. */
                ev_iocp_t       io;                 /**< IOCP backend */
                ev_write_t*     w_req;              /**< The write request mapping for IOCP */
                size_t          w_buf_idx;          /**< The write buffer mapping for IOCP */
            }iocp[EV_IOV_MAX];

            unsigned            w_io_idx;           /**< Usable index */
            unsigned            w_io_cnt;           /**< Busy count */

            ev_list_t           w_pending;          /**< Request queue to be write */
            ev_list_t           w_doing;            /**< Request queue in writing */
            ev_write_t*         w_half;
            size_t              w_half_idx;
        }wio;
    }data_mode;

    struct
    {
#define EV_PIPE_BACKEND_BUFFER_SIZE    \
    (sizeof(ev_ipc_frame_hdr_t) + sizeof(ev_pipe_win_ipc_info_t))

        DWORD                   peer_pid;           /**< Peer process ID */

        struct
        {
            struct
            {
                unsigned        r_pending : 1;      /**< There is a IOCP request pending */
            }mask;
            int                 r_err;              /**< Error code if read failure */
            DWORD               remain_size;        /**< How many data need to read (bytes) */
            ev_list_t           pending;            /**< Buffer list to be filled */
            ev_read_t*          reading;            /**< Request for read */
            DWORD               buf_idx;            /**< Buffer for read */
            DWORD               buf_pos;            /**< Available position */
            ev_iocp_t           io;                 /**< IOCP read backend */
            uint8_t             buffer[EV_PIPE_BACKEND_BUFFER_SIZE];
        }rio;

        struct
        {
            struct
            {
                unsigned        w_pending : 1;      /**< There is a IOCP request pending */
            }mask;
            int                 w_err;              /**< Error code if write failure */
            ev_iocp_t           io;                 /**< IOCP write backend */
            ev_write_t*         sending;            /**< The write request sending */
            size_t              buf_idx;            /**< The buffer index need to send */
            ev_list_t           pending;            /**< FIFO queue of write request */
            uint8_t             buffer[EV_PIPE_BACKEND_BUFFER_SIZE];
        }wio;

#undef EV_PIPE_BACKEND_BUFFER_SIZE
    }ipc_mode;
}ev_pipe_backend_t;
#define EV_PIPE_BACKEND_INIT    { 0 }

#ifdef __cplusplus
}
#endif
#endif
