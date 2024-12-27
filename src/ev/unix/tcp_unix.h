#ifndef __EV_TCP_UNIX_H__
#define __EV_TCP_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unix backend for #ev_tcp_t.
 */
typedef struct ev_tcp_backend
{
    union {
        struct
        {
            ev_nonblock_io_t io;           /**< IO object */
            ev_list_t        accept_queue; /**< Accept queue */
        } listen;
        struct
        {
            ev_tcp_accept_cb cb;          /**< Accept callback */
            void            *arg;         /**< User defined argument. */
            ev_list_node_t   accept_node; /**< Accept queue node */
        } accept;
        ev_nonblock_stream_t stream; /**< IO component */
        struct
        {
            ev_nonblock_io_t  io;   /**< IO object */
            ev_tcp_connect_cb cb;   /**< Connect callback */
            void             *arg;  /**< User defined argument */
            int               stat; /**< Connect result */
        } client;
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
 * @brief Open fd for read/write.
 * @param[in] tcp   TCP handle
 * @param[in] fd    fd
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__tcp_open(ev_tcp_t *tcp, int fd);

#ifdef __cplusplus
}
#endif
#endif
