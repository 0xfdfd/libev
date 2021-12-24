#ifndef __EV_BACKEND_UNIX_H__
#define __EV_BACKEND_UNIX_H__

#include "ev/os_unix.h"
#include "ev/map.h"
#include "ev/list.h"
#include "ev/ipc-protocol.h"
#include "ev/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ev_nonblock_stream;
typedef struct ev_nonblock_stream ev_nonblock_stream_t;

/**
 * @brief Write callback
 * @param[in] req       Write request
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void(*ev_stream_write_cb)(ev_nonblock_stream_t* stream, ev_write_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void(*ev_stream_read_cb)(ev_nonblock_stream_t* stream, ev_read_t* req, size_t size, int stat);

/**
 * @brief Buffer
 * @internal Must share the same layout with struct iovec
 */
struct ev_buf
{
    void*                       data;               /**< Data address */
    size_t                      size;               /**< Data size */
};
#define EV_BUF_INIT(buf, len)   { (void*)buf, (size_t)len }

struct ev_once
{
    pthread_once_t              guard;              /**< Once token */
};
#define EV_ONCE_INIT            { PTHREAD_ONCE_INIT }

struct ev_nonblock_io;
typedef struct ev_nonblock_io ev_nonblock_io_t;

/**
 * @brief IO active callback
 * @param[in] io    IO object
 * @param[in] evts  IO events
 */
typedef void(*ev_nonblock_io_cb)(ev_nonblock_io_t* io, unsigned evts, void* arg);

struct ev_nonblock_io
{
    ev_map_node_t               node;               /**< #ev_loop_plt_t::io */
    struct
    {
        int                     fd;                 /**< File descriptor */
        unsigned                c_events;           /**< Current events */
        unsigned                n_events;           /**< Next events */
        ev_nonblock_io_cb       cb;                 /**< IO active callback */
        void*                   arg;                /**< User data */
    }data;
};
#define EV_NONBLOCK_IO_INIT     { EV_MAP_NODE_INIT, { 0, 0, 0, NULL, NULL } }

typedef struct ev_loop_plt
{
    int                         pollfd;             /**< Multiplexing */
    ev_map_t                    io;                 /**< #ev_io_t */

    struct
    {
        int                     fd;                 /**< Wakeup fd */
        ev_nonblock_io_t        io;                 /**< Wakeup IO */
    }wakeup;
}ev_loop_plt_t;
#define EV_LOOP_PLT_INIT        \
    {\
        -1,\
        EV_MAP_INIT(NULL, NULL),\
        {\
            -1,\
            EV_NONBLOCK_IO_INIT,\
        }\
    }

typedef struct ev_write_backend
{
    size_t                      idx;                /**< Write buffer index */
}ev_write_backend_t;
#define EV_WRITE_BACKEND_INIT   { 0 }

typedef struct ev_read_backend
{
    int                         useless[0];         /**< Useless field */
}ev_read_backend_t;
#define EV_READ_BACKEND_INIT    { }

/**
 * @brief Can be used by #ev_write_backend_t and #ev_read_backend_t
 */
#define EV_IOV_BUF_SIZE_INTERNAL(nbuf)   \
    (sizeof(ev_buf_t) * (nbuf))

struct ev_nonblock_stream
{
    ev_loop_t*                  loop;               /**< Event loop */

    struct
    {
        unsigned                io_abort : 1;       /**< No futher IO allowed */
        unsigned                io_reg_r : 1;       /**< IO registered read event */
        unsigned                io_reg_w : 1;       /**< IO registered write event */
    }flags;

    ev_nonblock_io_t            io;                 /**< IO object */

    struct
    {
        ev_list_t               w_queue;            /**< Write queue */
        ev_list_t               r_queue;            /**< Read queue */
    }pending;

    struct
    {
        ev_stream_write_cb      w_cb;               /**< Write callback */
        ev_stream_read_cb       r_cb;               /**< Read callback */
    }callbacks;
};
#define EV_NONBLOCK_STREAM_INIT \
    {\
        NULL,                           /* .loop */\
        { 0, 0, 0, 0, 0 },              /* .flags */\
        EV_NONBLOCK_IO_INIT,            /* .io */\
        { EV_LIST_INIT, EV_LIST_INIT }, /* .pending */\
        { NULL, NULL }                  /* .callbacks */\
    }

typedef struct ev_tcp_backend
{
    union
    {
        struct
        {
            ev_nonblock_io_t    io;                 /**< IO object */
            ev_list_t           accept_queue;       /**< Accept queue */
        }listen;

        struct
        {
            ev_accept_cb        cb;                 /**< Accept callback */
            ev_list_node_t      accept_node;        /**< Accept queue node */
        }accept;

        ev_nonblock_stream_t    stream;             /**< IO component */

        struct
        {
            ev_nonblock_io_t    io;                 /**< IO object */
            ev_connect_cb       cb;                 /**< Connect callback */
            ev_todo_t           token;              /**< Todo token */
            int                 stat;               /**< Connect result */
        }client;
    }u;
}ev_tcp_backend_t;
#define EV_TCP_BACKEND_INIT     { { { EV_NONBLOCK_IO_INIT, EV_LIST_INIT } } }

typedef union ev_pipe_backend
{
    int                         _useless;           /**< For static initializer */

    struct
    {
        ev_nonblock_stream_t    stream;             /**< Stream */
    }data_mode;

    struct
    {
        ev_nonblock_io_t        io;                 /**< IO object */

        struct
        {
            unsigned            wio_pending : 1;    /**< Write pending */
            unsigned            rio_pending : 1;    /**< Read pending */
            unsigned            no_cmsg_cloexec : 1;/**< No MSG_CMSG_CLOEXEC */
        }mask;

        struct
        {
            struct
            {
                size_t          head_read_size;     /**< Head read size */
                size_t          data_remain_size;   /**< Data remain to read */

                size_t          buf_idx;            /**< Buffer index to fill */
                size_t          buf_pos;            /**< Buffer position to fill */

                ev_read_t*      reading;            /**< Currernt handling request */
            }curr;

            ev_list_t           rqueue;             /**< #ev_read_t */
            uint8_t             buffer[sizeof(ev_ipc_frame_hdr_t)];
        }rio;

        struct
        {
            struct
            {
                size_t          head_send_capacity; /**< Head send capacity */
                size_t          head_send_size;     /**< Head send size */

                size_t          buf_idx;            /**< Buffer index to send */
                size_t          buf_pos;            /**< Buffer position to send */

                ev_write_t*     writing;            /**< Currernt handling request */
            }curr;

            ev_list_t           wqueue;             /**< #ev_write_t */
            uint8_t             buffer[sizeof(ev_ipc_frame_hdr_t)];
        }wio;
    }ipc_mode;
}ev_pipe_backend_t;
#define EV_PIPE_BACKEND_INIT    { 0 }

typedef struct ev_shm_backend
{
    char                        name[256];
    int                         map_file;

    struct
    {
        unsigned                is_open : 1;
    }mask;
}ev_shm_backend_t;
#define EV_SHM_BACKEND_INIT     { 0 }

#ifdef __cplusplus
}
#endif
#endif
