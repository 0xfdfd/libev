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
#include "ev/map.h"

typedef HANDLE ev_os_handle_t;
#define EV_OS_HANDLE_INVALID    INVALID_HANDLE_VALUE

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
 * @param[in] req   IOCP request
 */
typedef void(*ev_iocp_cb)(ev_iocp_t* req, size_t transferred, void* arg);

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
    size_t                      size;               /**< Write size */
    int                         stat;               /**< Write result */
    ev_iocp_t                   io[EV_IOV_MAX];     /**< IOCP backend */
}ev_write_backend_t;
#define EV_WRITE_BACKEND_INIT   \
    {\
        NULL, 0, 0,\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_IOCP_INIT), }\
    }

typedef struct ev_read_backend
{
    void*                       owner;              /**< Owner */
    size_t                      size;               /**< Read size */
    int                         stat;               /**< Write result */
    ev_iocp_t                   io[EV_IOV_MAX];     /**< IOCP backend */
}ev_read_backend_t;
#define EV_READ_BACKEND_INIT    \
    {\
        NULL, 0, 0,\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_IOCP_INIT), }\
    }

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

typedef struct ev_pipe_backend
{
    struct
    {
        ev_list_t               w_queue;
        ev_list_t               r_queue;
    }stream;
}ev_pipe_backend_t;

#ifdef __cplusplus
}
#endif
#endif
