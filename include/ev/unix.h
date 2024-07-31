/**
 * @file
 */
#ifndef __EV_BACKEND_UNIX_H__
#define __EV_BACKEND_UNIX_H__

#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(O_APPEND)
#   define EV_FS_O_APPEND       O_APPEND
#else
#   define EV_FS_O_APPEND       0
#endif

#if defined(O_CREAT)
#   define EV_FS_O_CREAT        O_CREAT
#else
#   define EV_FS_O_CREAT        0
#endif

#if defined(O_DSYNC)
#   define EV_FS_O_DSYNC        O_DSYNC
#else
#   define EV_FS_O_DSYNC        0
#endif

#if defined(O_EXCL)
#   define EV_FS_O_EXCL         O_EXCL
#else
#   define EV_FS_O_EXCL         0
#endif

#if defined(O_SYNC)
#   define EV_FS_O_SYNC         O_SYNC
#else
#   define EV_FS_O_SYNC         0
#endif

#if defined(O_TRUNC)
#   define EV_FS_O_TRUNC        O_TRUNC
#else
#   define EV_FS_O_TRUNC        0
#endif

#if defined(O_RDONLY)
#   define EV_FS_O_RDONLY       O_RDONLY
#else
#   define EV_FS_O_RDONLY       0
#endif

#if defined(O_WRONLY)
#   define EV_FS_O_WRONLY       O_WRONLY
#else
#   define EV_FS_O_WRONLY       0
#endif

#if defined(O_RDWR)
#   define EV_FS_O_RDWR         O_RDWR
#else
#   define EV_FS_O_RDWR         0
#endif

#define EV_FS_S_IRUSR           S_IRUSR
#define EV_FS_S_IWUSR           S_IWUSR
#define EV_FS_S_IXUSR           S_IXUSR
#define EV_FS_S_IRWXU           S_IRWXU

#define EV_FS_SEEK_BEG          SEEK_SET
#define EV_FS_SEEK_CUR          SEEK_CUR
#define EV_FS_SEEK_END          SEEK_END

typedef pid_t                   ev_os_pid_t;
#define EV_OS_PID_INVALID       ((pid_t)-1)

typedef int                     ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      (-1)

typedef int                     ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    (-1)

typedef pid_t                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((pid_t)(-1))

typedef int                     ev_os_file_t;
#define EV_OS_FILE_INVALID      (-1)

typedef pthread_t               ev_os_thread_t;
#define EV_OS_THREAD_INVALID    ((pthread_t)(-1))

typedef pthread_key_t           ev_os_tls_t;
typedef pthread_mutex_t         ev_os_mutex_t;
typedef sem_t                   ev_os_sem_t;

typedef void*                   ev_os_shdlib_t;
#define EV_OS_SHDLIB_INVALID    (NULL)

struct ev_write;
struct ev_read;

struct ev_nonblock_stream;

/**
 * @brief Typedef of #ev_nonblock_stream.
 */
typedef struct ev_nonblock_stream ev_nonblock_stream_t;

/**
 * @brief Write callback
 * @param[in] req       Write request
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void(*ev_stream_write_cb)(ev_nonblock_stream_t* stream, struct ev_write* req, ssize_t size);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void(*ev_stream_read_cb)(ev_nonblock_stream_t* stream, struct ev_read* req, ssize_t size);

/**
 * @brief Buffer
 * @internal Must share the same layout with `struct iovec`.
 */
typedef struct ev_buf
{
    void*                       data;               /**< Data address */
    size_t                      size;               /**< Data size */
} ev_buf_t;

/**
 * @brief Initialize #ev_buf_t.
 * @param[in] buf   Data address.
 * @param[in] len   Data length.
 */
#define EV_BUF_INIT(buf, len)   { (void*)buf, (size_t)len }

/**
 * @brief Unix implementation of once token.
 */
struct ev_once
{
    pthread_once_t              guard;              /**< Once token */
};

/**
 * @brief Initialize #ev_once_t to Unix specific structure.
 */
#define EV_ONCE_INIT            { PTHREAD_ONCE_INIT }

struct ev_nonblock_io;

/**
 * @brief Typedef of #ev_nonblock_io.
 */
typedef struct ev_nonblock_io ev_nonblock_io_t;

/**
 * @brief IO active callback
 * @param[in] io    IO object
 * @param[in] evts  IO events
 */
typedef void(*ev_nonblock_io_cb)(ev_nonblock_io_t* io, unsigned evts, void* arg);

/**
 * @brief Nonblock IO.
 */
struct ev_nonblock_io
{
    ev_map_node_t               node;               /**< #EV_LOOP_BACKEND::io */
    struct
    {
        int                     fd;                 /**< File descriptor */
        unsigned                c_events;           /**< Current events */
        unsigned                n_events;           /**< Next events */
        ev_nonblock_io_cb       cb;                 /**< IO active callback */
        void*                   arg;                /**< User data */
    }data;
};

/**
 * @brief Initialize #ev_nonblock_io_t to an invalid value.
 */
#define EV_NONBLOCK_IO_INVALID  \
    {\
        EV_MAP_NODE_INIT,\
        {\
            0,\
            0,\
            0,\
            NULL,\
            NULL,\
        }\
    }

/**
 * @brief Unix backend for #ev_async_t.
 */
#define EV_ASYNC_BACKEND    \
    struct ev_async_plt {\
        /**\
         * @brief pipefd for wakeup.\
         * To wakeup, write data to pipfd[1].\
         */\
        int                         pipfd[2];\
        ev_nonblock_io_t            io;\
    }

/**
 * @brief Initialize #EV_ASYNC_BACKEND to an Unix specific invalid value.
 */
#define EV_ASYNC_PLT_INVALID    { { -1, -1 }, EV_NONBLOCK_IO_INVALID }

/**
 * @brief Unix backend for #ev_loop_t.
 */
#define EV_LOOP_BACKEND \
    struct ev_loop_plt {\
        int                         pollfd;             /**< Multiplexing */\
        ev_map_t                    io;                 /**< table for #ev_nonblock_io_t */\
        struct {\
            int                     evtfd[2];           /**< [0] for read, [1] for write. */\
            ev_nonblock_io_t        io;\
        } threadpool;\
    }

/**
 * @brief Initialize #EV_LOOP_BACKEND to Unix specific invalid value.
 */
#define EV_LOOP_PLT_INIT        \
    {\
        -1,\
        EV_MAP_INIT(NULL, NULL),\
        {\
            { EV_OS_PIPE_INVALID, EV_OS_PIPE_INVALID },\
            EV_NONBLOCK_IO_INVALID,\
        }\
    }

/**
 * @brief Unix backend for #ev_tcp_read_req_t.
 */
#define EV_TCP_READ_BACKEND \
    struct ev_tcp_read_backend {\
        int                         _useless[0];        /**< Useless field */\
    }

/**
 * @brief Unix backend for #ev_tcp_write_req_t.
 */
#define EV_TCP_WRITE_BACKEND    \
    struct ev_tcp_write_backend {\
        int                         _useless[0];        /**< Useless field */\
    }

/**
 * @brief Unix backend for #ev_udp_read_t.
 */
#define EV_UDP_READ_BACKEND \
    struct ev_udp_read_backend {\
        int                         _useless[0];        /**< Useless field */\
    }

/**
 * @brief Unix backend for #ev_udp_write_t.
 */
#define EV_UDP_WRITE_BACKEND    \
    struct ev_udp_write_backend {\
        struct sockaddr_storage     peer_addr;          /**< Peer address */\
    }

/**
 * @brief Nonblock stream.
 */
struct ev_nonblock_stream
{
    struct ev_loop*             loop;               /**< Event loop */

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

/**
 * @brief Initialize #ev_nonblock_stream_t to Unix specific invalid value.
 */
#define EV_NONBLOCK_STREAM_INIT \
    {\
        NULL,                           /* .loop */\
        { 0, 0, 0, 0, 0 },              /* .flags */\
        EV_NONBLOCK_IO_INVALID,         /* .io */\
        { EV_LIST_INIT, EV_LIST_INIT }, /* .pending */\
        { NULL, NULL }                  /* .callbacks */\
    }

/**
 * @brief Unix backend for #ev_tcp_t.
 */
#define EV_TCP_BACKEND    \
    struct ev_tcp_backend {\
        union {\
            struct {\
                ev_nonblock_io_t            io;                 /**< IO object */\
                ev_list_t                   accept_queue;       /**< Accept queue */\
            }listen;\
            struct {\
                ev_tcp_accept_cb            cb;                 /**< Accept callback */\
                ev_list_node_t              accept_node;        /**< Accept queue node */\
            }accept;\
            ev_nonblock_stream_t            stream;             /**< IO component */\
            struct {\
                ev_nonblock_io_t            io;                 /**< IO object */\
                ev_tcp_connect_cb           cb;                 /**< Connect callback */\
                int                         stat;               /**< Connect result */\
            }client;\
        }u;\
    }

/**
 * @brief Initialize #EV_TCP_BACKEND to Unix specific invalid value.
 */
#define EV_TCP_BACKEND_INIT         { { { EV_NONBLOCK_IO_INVALID, EV_LIST_INIT } } }

/**
 * @brief Unix backend for #ev_udp_t.
 */
#define EV_UDP_BACKEND  \
    struct ev_udp_backend {\
        ev_nonblock_io_t                    io;                 /**< Backend IO */\
    }

/**
 * @brief Unix backend for #ev_pipe_read_req_t.
 */
#define EV_PIPE_READ_BACKEND    \
    struct ev_pipe_read_backend {\
        int                                 _useless[0];        /**< Useless field */\
    }

/**
 * @brief Unix backend for #ev_pipe_write_req_t.
 */
#define EV_PIPE_WRITE_BACKEND   \
    struct ev_pipe_write_backend {\
        int                                 _useless[0];        /**< Useless field */\
    }

/**
 * @brief Unix backend for #ev_pipe_t.
 */
#define EV_PIPE_BACKEND \
    union ev_pipe_backend {\
        int                                 _useless;           /**< For static initializer */\
        struct {\
            ev_nonblock_stream_t            stream;             /**< Stream */\
        }data_mode;\
        struct {\
            ev_nonblock_io_t                io;                 /**< IO object */\
            struct {\
                unsigned                    wio_pending : 1;    /**< Write pending */\
                unsigned                    rio_pending : 1;    /**< Read pending */\
                unsigned                    no_cmsg_cloexec : 1;/**< No MSG_CMSG_CLOEXEC */\
            }mask;\
            struct {\
                struct {\
                    size_t                  head_read_size;     /**< Head read size */\
                    size_t                  data_remain_size;   /**< Data remain to read */\
                    size_t                  buf_idx;            /**< Buffer index to fill */\
                    size_t                  buf_pos;            /**< Buffer position to fill */\
                    ev_pipe_read_req_t*     reading;            /**< Current handling request */\
                }curr;\
                ev_list_t                   rqueue;             /**< #ev_pipe_read_req_t */\
                uint8_t                     buffer[sizeof(ev_ipc_frame_hdr_t)];\
            }rio;\
            struct {\
                struct {\
                    size_t                  head_send_capacity; /**< Head send capacity */\
                    size_t                  head_send_size;     /**< Head send size */\
                    size_t                  buf_idx;            /**< Buffer index to send */\
                    size_t                  buf_pos;            /**< Buffer position to send */\
                    ev_pipe_write_req_t*    writing;            /**< Currernt handling request */\
                }curr;\
                ev_list_t                   wqueue;             /**< #ev_pipe_write_req_t */\
                uint8_t                     buffer[sizeof(ev_ipc_frame_hdr_t)];\
            }wio;\
        }ipc_mode;\
    }

/**
 * @brief Initialize #EV_PIPE_BACKEND to Unix specific invalid value.
 */
#define EV_PIPE_BACKEND_INVALID         { 0 }

/**
 * @brief Unix backend for #ev_shm_t.
 */
#define EV_SHM_BACKEND  \
    struct ev_shm_backend {\
        char                                name[256];\
        int                                 map_file;\
        struct {\
            unsigned                        is_open : 1;\
        }mask;\
    }

/**
 * @brief Initialize #EV_SHM_BACKEND to Unix specific invalid value.
 */
#define EV_SHM_BACKEND_INVALID          { 0 }

/**
 * @brief Unix backend for #ev_process_t.
 */
#define EV_PROCESS_BACKEND  \
    struct ev_process_backend_s {\
        struct {\
            int                             waitpid : 1;     /**< Already call waitpid() */\
        } flags;\
    }

#ifdef __cplusplus
}
#endif

#endif
