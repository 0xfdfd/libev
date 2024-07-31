/**
 * @file
 */
#ifndef __EV_BACKEND_WIN_H__
#define __EV_BACKEND_WIN_H__

#ifndef _WIN32_WINNT
#    define _WIN32_WINNT   0x0600
#endif
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#   if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#       define SSIZE_MAX INTPTR_MAX
#       define _SSIZE_T_
#       define _SSIZE_T_DEFINED
#   endif

/**
 * @addtogroup EV_FILESYSTEM
 * @{
 */

/**
 * @brief The file is opened in append mode. Before each write, the file offset
 *   is positioned at the end of the file.
 */
#define EV_FS_O_APPEND          _O_APPEND

/**
 * @brief The file is created if it does not already exist.
 */
#define EV_FS_O_CREAT           _O_CREAT

/**
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and a minimum of metadata are flushed to disk.
 */
#define EV_FS_O_DSYNC           FILE_FLAG_WRITE_THROUGH

/**
 * @brief If the `O_CREAT` flag is set and the file already exists, fail the open.
 */
#define EV_FS_O_EXCL            _O_EXCL

/**
 * @brief The file is opened for synchronous I/O. Write operations will complete
 *   once all data and all metadata are flushed to disk.
 */
#define EV_FS_O_SYNC            FILE_FLAG_WRITE_THROUGH

/**
 * @brief If the file exists and is a regular file, and the file is opened
 *   successfully for write access, its length shall be truncated to zero.
 */
#define EV_FS_O_TRUNC           _O_TRUNC

/**
 * @brief Open the file for read-only access.
 */
#define EV_FS_O_RDONLY          _O_RDONLY

/**
 * @brief Open the file for write-only access.
 */
#define EV_FS_O_WRONLY          _O_WRONLY

/**
 * @def EV_FS_O_RDWR
 * @brief Open the file for read-write access.
 */
#define EV_FS_O_RDWR            _O_RDWR

/**
 * @brief User has read permission.
 */
#define EV_FS_S_IRUSR           _S_IREAD

/**
 * @brief User has write permission.
 */
#define EV_FS_S_IWUSR           _S_IWRITE

/**
 * @brief User has execute permission.
 */
#define EV_FS_S_IXUSR           _S_IEXEC

/**
 * @brief user (file owner) has read, write, and execute permission.
 */
#define EV_FS_S_IRWXU           (EV_FS_S_IRUSR | EV_FS_S_IWUSR | EV_FS_S_IXUSR)

/**
 * @brief The starting point is zero or the beginning of the file.
 */
#define EV_FS_SEEK_BEG           FILE_BEGIN

/**
 * @brief The starting point is the current value of the file pointer.
 */
#define EV_FS_SEEK_CUR           FILE_CURRENT

/**
 * @brief The starting point is the current end-of-file position.
 */
#define EV_FS_SEEK_END           FILE_END

/**
 * @brief Windows system define of file.
 */
typedef HANDLE                   ev_os_file_t;

/**
 * @brief Invalid valid of #ev_os_file_t.
 */
#define EV_OS_FILE_INVALID      INVALID_HANDLE_VALUE

/**
 * @} EV_FILESYSTEM
 */

/**
 * @addtogroup EV_PROCESS
 * @{
 */

typedef HANDLE                  ev_os_pid_t;
#define EV_OS_PID_INVALID       INVALID_HANDLE_VALUE

/**
 * @} EV_PROCESS
 */

typedef HANDLE                  ev_os_pipe_t;
#define EV_OS_PIPE_INVALID      INVALID_HANDLE_VALUE

typedef SOCKET                  ev_os_socket_t;
#define EV_OS_SOCKET_INVALID    INVALID_SOCKET

typedef DWORD                   ev_os_tid_t;
#define EV_OS_TID_INVALID       ((DWORD)(-1))

typedef HANDLE                  ev_os_thread_t;
#define EV_OS_THREAD_INVALID    INVALID_HANDLE_VALUE

typedef DWORD                   ev_os_tls_t;
typedef CRITICAL_SECTION        ev_os_mutex_t;
typedef HANDLE                  ev_os_sem_t;

typedef HANDLE                  ev_os_shdlib_t;
#define EV_OS_SHDLIB_INVALID    (NULL)

/**
 * @brief Buffer
 * @internal Must share the same layout with WSABUF
 */
typedef struct ev_buf
{
    ULONG                       size;               /**< Data size */
    CHAR*                       data;               /**< Data address */
} ev_buf_t;

/**
 * @brief Initialize #ev_buf_t.
 * @param[in] buf   Data address.
 * @param[in] len   Data length.
 */
#define EV_BUF_INIT(buf, len)   { (ULONG)len, (CHAR*)buf }

struct ev_once
{
    INIT_ONCE                   guard;              /**< Once token */
};

/**
 * @brief Initialize #ev_once_t to Windows specific structure.
 */
#define EV_ONCE_INIT            { INIT_ONCE_STATIC_INIT }

struct ev_iocp;

/**
 * @brief Typedef of #ev_iocp.
 */
typedef struct ev_iocp ev_iocp_t;

/**
 * @brief IOCP complete callback
 * @param[in] iocp  IOCP request
 */
typedef void(*ev_iocp_cb)(ev_iocp_t* iocp, size_t transferred, void* arg);

/**
 * @brief IOCP structure.
 */
struct ev_iocp
{
    void*                       arg;                /**< Index */
    ev_iocp_cb                  cb;                 /**< Callback */
    OVERLAPPED                  overlapped;         /**< IOCP field */
};

/**
 * @brief Initialize #ev_iocp_t to invalid value.
 */
#define EV_IOCP_INIT            { NULL, NULL, { 0, 0, { { 0, 0 } }, NULL } }

/**
 * @brief Windows backend for #ev_async_t.
 */
#define EV_ASYNC_BACKEND    \
    struct ev_async_plt {\
        LONG volatile               async_sent;\
        ev_iocp_t                   io;\
    }

/**
 * @brief Initialize #EV_ASYNC_BACKEND to an Windows specific invalid value.
 */
#define EV_ASYNC_PLT_INVALID    { 0, EV_IOCP_INIT }

#define EV_LOOP_BACKEND \
    struct ev_loop_plt {\
        HANDLE                      iocp;               /**< IOCP handle */\
        struct {\
            ev_iocp_t               io;                 /**< Wakeup token */\
        } threadpool;\
    }

/**
 * @brief Initialize #EV_LOOP_BACKEND to Windows specific invalid value.
 */
#define EV_LOOP_PLT_INIT        { NULL, { EV_IOCP_INIT } }

/**
 * @brief Windows backend for #ev_tcp_read_req_t.
 */
#define EV_TCP_READ_BACKEND \
    struct ev_tcp_read_backend {\
        ev_tcp_t*                   owner;              /**< Owner */\
        ev_iocp_t                   io;                 /**< IOCP */\
        int                         stat;               /**< Read result */\
    }

/**
 * @brief  Windows backend for #ev_tcp_write_req_t.
 */
#define EV_TCP_WRITE_BACKEND    \
    struct ev_tcp_write_backend {\
        void*                       owner;              /**< Owner */\
        int                         stat;               /**< Write result */\
        ev_iocp_t                   io;                 /**< IOCP backend */\
    }

/**
 * @brief Windows backend for #ev_tcp_t.
 */
#define EV_TCP_BACKEND  \
    struct ev_tcp_backend {\
        int                         af;                 /**< AF_INET / AF_INET6 */\
        ev_iocp_t                   io;                 /**< IOCP */\
        struct {\
            unsigned                todo_pending : 1;   /**< Already submit todo request */\
        }mask;\
        union {\
            struct {\
                ev_list_t           a_queue;            /**< (#ev_tcp_backend::u::accept::node) Accept queue */\
                ev_list_t           a_queue_done;       /**< (#ev_tcp_backend::u::accept::node) Accept done queue */\
            }listen;\
            struct {\
                ev_tcp_accept_cb    cb;                 /**< Accept callback */\
                ev_list_node_t      node;               /**< (#ev_tcp_backend::u::listen) Accept queue node */\
                ev_tcp_t*           listen;             /**< Listen socket */\
                int                 stat;               /**< Accept result */\
                /**\
                 * lpOutputBuffer for AcceptEx.\
                 * dwLocalAddressLength and dwRemoteAddressLength require 16 bytes\
                 * more than the maximum address length for the transport protocol.\
                 */\
                char                buffer[(sizeof(struct sockaddr_storage) + 16) * 2];\
            }accept;\
            struct {\
                ev_tcp_connect_cb   cb;                 /**< Callback */\
                LPFN_CONNECTEX      fn_connectex;       /**< ConnectEx */\
                int                 stat;               /**< Connect result */\
            }client;\
            struct {\
                ev_list_t           w_queue;            /**< (#ev_write_t::node) Write queue */\
                ev_list_t           w_queue_done;       /**< (#ev_write_t::node) Write done queue */\
                ev_list_t           r_queue;            /**< (#ev_read_t::node) Read queue */\
                ev_list_t           r_queue_done;       /**< (#ev_read_t::node) Read done queue */\
            }stream;\
        }u;\
    }

/**
 * @brief Initialize #EV_TCP_BACKEND to Windows specific invalid value.
 */
#define EV_TCP_BACKEND_INIT     \
    {\
        0,\
        EV_IOCP_INIT,\
        { 0 },\
        { { EV_LIST_INIT, EV_LIST_INIT } },\
    }

typedef int (WSAAPI* ev_wsarecvfrom_fn)(
    SOCKET socket,
    LPWSABUF buffers,
    DWORD buffer_count,
    LPDWORD bytes,
    LPDWORD flags,
    struct sockaddr* addr,
    LPINT addr_len,
    LPWSAOVERLAPPED overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

typedef int (WSAAPI* ev_wsarecv_fn)(
    SOCKET socket,
    LPWSABUF buffers,
    DWORD buffer_count,
    LPDWORD bytes,
    LPDWORD flags,
    LPWSAOVERLAPPED overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

/**
 * @brief Windows backend for #ev_udp_write_t.
 */
#define EV_UDP_WRITE_BACKEND    \
    struct ev_udp_write_backend {\
        ev_iocp_t                   io;                 /**< IOCP handle */\
        int                         stat;               /**< Write result */\
        ev_udp_t*                   owner;              /**< Owner */\
    }

/**
 * @brief Windows backend for #ev_udp_read_t.
 */
#define EV_UDP_READ_BACKEND \
    struct ev_udp_read_backend {\
        ev_udp_t*                   owner;              /**< Owner */\
        ev_iocp_t                   io;                 /**< IOCP handle */\
        int                         stat;               /**< Read result */\
    }

/**
 * @brief Windows backend for #ev_udp_t.
 */
#define EV_UDP_BACKEND  \
    struct ev_udp_backend {\
        ev_wsarecv_fn               fn_wsarecv;\
        ev_wsarecvfrom_fn           fn_wsarecvfrom;     /**< WSARecvFrom() */\
    }

typedef enum ev_pipe_win_ipc_info_type
{
    EV_PIPE_WIN_IPC_INFO_TYPE_STATUS,               /**< #ev_pipe_win_ipc_info_t::data::as_status */
    EV_PIPE_WIN_IPC_INFO_TYPE_PROTOCOL_INFO,        /**< #ev_pipe_win_ipc_info_t::data::as_protocol_info */
}ev_pipe_win_ipc_info_type_t;

/**
 * @brief Windows IPC frame information.
 */
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
} ev_pipe_win_ipc_info_t;

#define EV_PIPE_WRITE_BACKEND   \
    struct ev_pipe_write_backend {\
        ev_pipe_t*                  owner;              /**< Owner */\
        int                         stat;               /**< Write result */\
    }

/**
 * @brief Windows backend for #ev_pipe_read_req_t.
 */
#define EV_PIPE_READ_BACKEND    \
    struct ev_pipe_read_backend {\
        ev_pipe_t*                  owner;              /**< Owner */\
        int                         stat;               /**< Read result */\
    }

#define EV_PIPE_BACKEND_BUFFER_SIZE    \
        (sizeof(ev_ipc_frame_hdr_t) + sizeof(ev_pipe_win_ipc_info_t))

#define EV_PIPE_BACKEND \
    union ev_pipe_backend {\
        int                                 _useless;           /**< For static initializer */\
        struct {\
            struct {\
                ev_iocp_t                   io;                 /**< IOCP backend */\
                ev_list_t                   r_pending;          /**< Request queue to be read */\
                ev_pipe_read_req_t*         r_doing;            /**< Request queue in reading */\
            }rio;\
            struct {\
                struct ev_pipe_backend_data_mode_wio {\
                    size_t                  idx;                /**< Index. Must not change. */\
                    ev_iocp_t               io;                 /**< IOCP backend */\
                    ev_pipe_write_req_t*    w_req;              /**< The write request mapping for IOCP */\
                    size_t                  w_buf_idx;          /**< The write buffer mapping for IOCP */\
                }iocp[EV_IOV_MAX];\
                unsigned                    w_io_idx;           /**< Usable index */\
                unsigned                    w_io_cnt;           /**< Busy count */\
                ev_list_t                   w_pending;          /**< Request queue to be write */\
                ev_list_t                   w_doing;            /**< Request queue in writing */\
                ev_pipe_write_req_t*        w_half;\
                size_t                      w_half_idx;\
            }wio;\
        } data_mode;\
        struct {\
            int                             iner_err;           /**< Internal error code */\
            DWORD                           peer_pid;           /**< Peer process ID */\
            struct {\
                struct {\
                    unsigned                rio_pending : 1;    /**< There is a IOCP request pending */\
                }mask;\
                struct {\
                    ev_pipe_read_req_t*     reading;            /**< Request for read */\
                    DWORD                   buf_idx;            /**< Buffer for read */\
                    DWORD                   buf_pos;            /**< Available position */\
                }reading;\
                ev_list_t                   pending;            /**< Buffer list to be filled */\
                int                         r_err;              /**< Error code if read failure */\
                DWORD                       remain_size;        /**< How many data need to read (bytes) */\
                ev_iocp_t                   io;                 /**< IOCP read backend */\
                uint8_t                     buffer[EV_PIPE_BACKEND_BUFFER_SIZE];\
            }rio;\
            struct {\
                struct {\
                    unsigned                iocp_pending : 1;   /**< There is a IOCP request pending */\
                }mask;\
                struct {\
                    ev_pipe_write_req_t*    w_req;              /**< The write request sending */\
                    size_t                  donecnt;            /**< Success send counter */\
                }sending;\
                ev_list_t                   pending;            /**< FIFO queue of write request */\
                int                         w_err;              /**< Error code if write failure */\
                ev_iocp_t                   io;                 /**< IOCP write backend, with #ev_write_t */\
                uint8_t                     buffer[EV_PIPE_BACKEND_BUFFER_SIZE];\
            }wio;\
        } ipc_mode;\
    }

/**
 * @brief Initialize #EV_PIPE_BACKEND to Windows specific invalid value.
 */
#define EV_PIPE_BACKEND_INVALID         { 0 }

/**
 * @brief Windows backend for #ev_shm_t.
 */
#define EV_SHM_BACKEND  \
    struct ev_shm_backend {\
        HANDLE                              map_file;           /**< Shared memory file */\
    }

/**
 * @brief Initialize #EV_SHM_BACKEND to Windows specific invalid value.
 */
#define EV_SHM_BACKEND_INVALID          { NULL }

/**
 * @brief Windows backend for #ev_process_t.
 */
#define EV_PROCESS_BACKEND  \
    struct ev_process_backend_s {\
        HANDLE                              wait_handle;\
    }

#ifdef __cplusplus
}
#endif

#endif
