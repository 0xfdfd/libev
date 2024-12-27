#ifndef __EV_TCP_WIN_INTERNAL_H__
#define __EV_TCP_WIN_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Windows backend for #ev_tcp_t.
 */
typedef struct ev_tcp_backend
{
    int       af; /**< AF_INET / AF_INET6 */
    ev_iocp_t io; /**< IOCP */
    struct
    {
        unsigned todo_pending : 1; /**< Already submit todo request */
    } mask;
    union {
        struct
        {
            ev_list_t
                a_queue; /**< (#ev_tcp_backend::u::accept::node) Accept queue */
            ev_list_t a_queue_done; /**< (#ev_tcp_backend::u::accept::node)
                                       Accept done queue */
        } listen;
        struct
        {
            ev_tcp_accept_cb cb;  /**< Accept callback */
            void            *arg; /**< Accept callback argument */
            ev_list_node_t
                node; /**< (#ev_tcp_backend::u::listen) Accept queue node */
            ev_tcp_t *listen; /**< Listen socket */
            int       stat;   /**< Accept result */
            /**\
             * lpOutputBuffer for AcceptEx.\
             * dwLocalAddressLength and dwRemoteAddressLength require 16 bytes\
             * more than the maximum address length for the transport protocol.\
             */
            char buffer[(sizeof(struct sockaddr_storage) + 16) * 2];
        } accept;
        struct
        {
            ev_tcp_connect_cb cb;           /**< Callback */
            void             *arg;          /**< User defined argument. */
            LPFN_CONNECTEX    fn_connectex; /**< ConnectEx */
            int               stat;         /**< Connect result */
        } client;
        struct
        {
            ev_list_t w_queue;      /**< (#ev_write_t::node) Write queue */
            ev_list_t w_queue_done; /**< (#ev_write_t::node) Write done queue */
            ev_list_t r_queue;      /**< (#ev_read_t::node) Read queue */
            ev_list_t r_queue_done; /**< (#ev_read_t::node) Read done queue */
        } stream;
    } u;
} ev_tcp_backend_t;

struct ev_tcp
{
    ev_handle_t      base;      /**< Base object */
    ev_tcp_close_cb  close_cb;  /**< User close callback */
    void            *close_arg; /**< User defined argument. */
    ev_os_socket_t   sock;      /**< Socket handle */
    ev_tcp_backend_t backend;   /**< Platform related implementation */
};

/**
 * @brief  Windows backend for #ev_tcp_write_req_t.
 */
typedef struct ev_tcp_write_backend
{
    void     *owner; /**< Owner */
    int       stat;  /**< Write result */
    ev_iocp_t io;    /**< IOCP backend */
} ev_tcp_write_backend_t;

typedef struct ev_tcp_write_req
{
    ev_write_t             base;      /**< Base object */
    ev_tcp_write_cb        write_cb;  /**< User callback */
    void                  *write_arg; /**< User defined argument. */
    ev_tcp_write_backend_t backend;   /**< Backend */
} ev_tcp_write_req_t;

/**
 * @brief Read request token for TCP socket.
 */
typedef struct ev_tcp_read_req
{
    ev_read_t           base;     /**< Base object */
    ev_tcp_read_cb      read_cb;  /**< User callback */
    void               *read_arg; /**< User defined argument. */
    EV_TCP_READ_BACKEND backend;  /**< Backend */
} ev_tcp_read_req_t;

/**
 * @brief Open fd for read/write.
 * @param[in] tcp   TCP handle
 * @param[in] fd    fd
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__tcp_open_win(ev_tcp_t *tcp, SOCKET fd);

#ifdef __cplusplus
}
#endif
#endif
