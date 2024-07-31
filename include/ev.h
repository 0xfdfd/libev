/**
 * @mainpage libev
 *
 * \section EV_OVERVIEW Overview
 *
 * [libev](https://github.com/qgymib/libev) is a rework of
 * [libuv](https://github.com/libuv/libuv), with following advantages:
 * 1. Strong types without static casts any more.
 * 2. Enhanced IPC features.
 * 3. Easy to use file system operations.
 *
 *
 * \section EV_DOCUMENTATION Documentation
 *
 * Located in the docs/ subdirectory. It use [Doxygen](http://www.doxygen.nl/)
 * to build documents, which means the source code is well documented and can
 * be read directly without build it.
 *
 * To build html-based documents, in the project root, run:
 * ```bash
 * doxygen docs/Doxyfile
 * ```
 *
 * Also documents can be browsed online [here](https://qgymib.github.io/libev/).
 *
 *
 * \section EV_BUILD_INSTRUCTIONS Build Instructions
 *
 * [CMake](https://cmake.org/) is currently the prefer way to build:
 *
 * ```bash
 * # Build:
 * $ mkdir -p build
 * $ (cd build && cmake .. -DBUILD_TESTING=ON) # generate project with tests
 * $ cmake --build build                       # add `-j <n>` with cmake >= 3.12
 * 
 * # Run tests:
 * $ (cd build && ctest -C Debug --output-on-failure)
 * ```
 *
 * \section EV_AMALGAMATE Amalgamate
 * 
 * [libev](https://github.com/qgymib/libev) support amalgamate build, which
 * allow to distribute libev's source code using only two files(`ev.h` and `ev.c`).
 *
 * > Note: Amalgamate requires python3.
 *
 * To use amalgamation, add `-DEV_AMALGAMATE_BUILD=on` when configurate cmake:
 *
 * ```bash
 * $ cmake -DEV_AMALGAMATE_BUILD=on /path/to/libev
 * $ cmake --build .
 * ```
 *
 * In `${CMAKE_CURRENT_BINARY_DIR}/amalgamate` directory, you will find the
 * generated files.
 *
 */
#ifndef __EV_H__
#define __EV_H__

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

#include "ev/expose.h"
#include "ev/list.h"
#include "ev/map.h"
#include "ev/version.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_DEFINE Defines
 * @{
 */

/**
 * @brief The maximum number of iov buffers can be support.
 */
#define EV_IOV_MAX              16

/**
 * @brief Expand macro.
 * @param[in] ...   Code to expand.
 */
#define EV_EXPAND(...)          __VA_ARGS__

/**
 * @brief Repeat code for \p n times.
 * @param[in] n     How many times to repeat.
 * @param[in] ...   Code fragment to repeat.
 */
#define EV_INIT_REPEAT(n, ...)   EV_INIT_REPEAT2(n, __VA_ARGS__)
/** @cond */
#define EV_INIT_REPEAT2(n, ...)  EV_INIT_REPEAT_##n(__VA_ARGS__)
#define EV_INIT_REPEAT_1(...)    EV_EXPAND(__VA_ARGS__)
#define EV_INIT_REPEAT_2(...)    EV_INIT_REPEAT_1(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_3(...)    EV_INIT_REPEAT_2(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_4(...)    EV_INIT_REPEAT_3(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_5(...)    EV_INIT_REPEAT_4(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_6(...)    EV_INIT_REPEAT_5(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_7(...)    EV_INIT_REPEAT_6(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_8(...)    EV_INIT_REPEAT_7(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_9(...)    EV_INIT_REPEAT_8(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_10(...)   EV_INIT_REPEAT_9(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_11(...)   EV_INIT_REPEAT_10(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_12(...)   EV_INIT_REPEAT_11(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_13(...)   EV_INIT_REPEAT_12(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_14(...)   EV_INIT_REPEAT_13(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_15(...)   EV_INIT_REPEAT_14(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_16(...)   EV_INIT_REPEAT_15(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
/** @endcond */

/**
 * @def EV_CONTAINER_OF
 * @brief cast a member of a structure out to the containing structure.
 */
#if defined(container_of)
#   define EV_CONTAINER_OF(ptr, type, member)   \
        container_of(ptr, type, member)
#elif defined(__GNUC__) || defined(__clang__)
#   define EV_CONTAINER_OF(ptr, type, member)   \
        ({ \
            const typeof(((type *)0)->member)*__mptr = (ptr); \
            (type *)((char *)__mptr - offsetof(type, member)); \
        })
#else
#   define EV_CONTAINER_OF(ptr, type, member)   \
        ((type *) ((char *) (ptr) - offsetof(type, member)))
#endif

/**
 * @} EV_DEFINE
 */

/**
 * @defgroup EV_OS OS
 * @{
 */

/**
 * @brief Infinite timeout.
 */
#define EV_INFINITE_TIMEOUT         ((uint32_t)-1)

/**
 * @} EV_OS
 */

/**
 * @defgroup EV_ERRNO Error number
 * @{
 */

/**
 * @brief Error number
 */
typedef enum ev_errno
{
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
    EV_ENOTDIR          = -20,                  /**< Not a directory (POSIX.1-2001) */
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
    EV_EPROTO           = -134,                 /**< Protocol error (POSIX.1-2001) */
    EV_EPROTONOSUPPORT  = -135,                 /**< Protocol not supported (POSIX.1-2001) */
    EV_ETIMEDOUT        = -138,                 /**< Operation timed out (POSIX.1-2001) */

    /* Extend error code */
    EV_EOF              = -4095,
}ev_errno_t;

/**
 * @brief Describe the error code
 * @param[in] err   Error code
 * @return          Describe string
 */
const char* ev_strerror(int err);

/**
 * @} EV_ERRNO
 */

/**
 * @defgroup EV_ALLOCATOR Allocator
 * @{
 */

/**
 * @brief Replacement function for malloc.
 * @see https://man7.org/linux/man-pages/man3/malloc.3.html
 */
typedef void* (*ev_malloc_fn)(size_t size);

/**
 * @brief Replacement function for calloc.
 * @see https://man7.org/linux/man-pages/man3/calloc.3.html
 */
typedef void* (*ev_calloc_fn)(size_t nmemb, size_t size);

/**
 * @brief Replacement function for realloc.
 * @see https://man7.org/linux/man-pages/man3/realloc.3.html
 */
typedef void* (*ev_realloc_fn)(void* ptr, size_t size);

/**
 * @brief Replacement function for free.
 * @see https://man7.org/linux/man-pages/man3/free.3.html
 */
typedef void (*ev_free_fn)(void* ptr);

/**
 * @brief Override the use of the standard library's malloc(3), calloc(3),
 *   realloc(3), free(3), memory allocation functions.
 *
 * This function must be called before any other function is called or after
 * all resources have been freed and thus doesn't reference any allocated
 * memory chunk.
 *
 * @warning There is no protection against changing the allocator multiple
 *   times. If the user changes it they are responsible for making sure the
 *   allocator is changed while no memory was allocated with the previous
 *   allocator, or that they are compatible.
 * @warngin Allocator must be thread-safe.
 * 
 * @param[in] malloc_func   Replacement function for malloc.
 * @param[in] calloc_func   Replacement function for calloc.
 * @param[in] realloc_func  Replacement function for realloc.
 * @param[in] free_func     Replacement function for free.
 * @return On success, it returns 0. if any of the function pointers is NULL it returns #EV_EINVAL.
 */
EV_API int ev_replace_allocator(ev_malloc_fn malloc_func, ev_calloc_fn calloc_func,
    ev_realloc_fn realloc_func, ev_free_fn free_func);

/**
 * @brief Same as [malloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void* ev_malloc(size_t size);

/**
 * @brief Same as [calloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void* ev_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as [realloc(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void* ev_realloc(void* ptr, size_t size);

/**
 * @brief Same as [free(3)](https://man7.org/linux/man-pages/man3/free.3.html)
 */
EV_API void ev_free(void* ptr);

/**
 * @} EV_ALLOCATOR
 */

/**
 * @defgroup EV_UTILS_QUEUE Queue
 * @ingroup EV_UTILS
 * @{
 */

/**
 * @brief Static initializer for #ev_queue_node_t.
 * @note A static initialized queue node is not a valid node.
 */

/**
 * @brief Queue node type.
 */
typedef struct ev_queue_node
{
    struct ev_queue_node* p_prev;
    struct ev_queue_node* p_next;
} ev_queue_node_t;
#define EV_QUEUE_NODE_INVALID  \
    {\
        NULL,\
        NULL,\
    }

/**
 * @brief Initialize circular linked list
 * @param[out] head     List handle
 */
EV_API void ev_queue_init(ev_queue_node_t* head);

/**
 * @brief Insert a node to the tail of the list.
 * @warning the node must not exist in any list.
 * @param[in,out] head  Pointer to list
 * @param[in,out] node  Pointer to a new node
 */
EV_API void ev_queue_push_back(ev_queue_node_t* head, ev_queue_node_t* node);

/**
 * @brief Insert a node to the head of the list.
 * @warning the node must not exist in any list.
 * @param[in,out] head      Pointer to list
 * @param[in,out] node      Pointer to a new node
 */
EV_API void ev_queue_push_front(ev_queue_node_t* head, ev_queue_node_t* node);

/**
 * @brief Delete a exist node
 * @param[in,out] node      The node you want to delete
 */
EV_API void ev_queue_erase(ev_queue_node_t* node);

/**
 * @brief Check whether list is empty
 * @param[in] node          Any node in list
 * @return                  bool
 */
EV_API int ev_queue_empty(const ev_queue_node_t* node);

/**
 * @brief Get the first node and remove it from the list.
 * @param[in,out] head      Pointer to list
 * @return                  The first node
 */
EV_API ev_queue_node_t* ev_queue_pop_front(ev_queue_node_t* head);

/**
 * @brief Get the last node and remove it from the list.
 * @param[in,out] head      Pointer to list
 * @return                  The last node
 */
EV_API ev_queue_node_t* ev_queue_pop_back(ev_queue_node_t* head);

/**
 * @brief Get the first node.
 * @param[in] head      Pointer to list
 * @return              The first node
 */
EV_API ev_queue_node_t* ev_queue_head(ev_queue_node_t* head);

/**
* @brief Get next node.
* @param[in] head   Queue head
* @param[in] node   Current node
* @return           The next node
*/
EV_API ev_queue_node_t* ev_queue_next(ev_queue_node_t* head, ev_queue_node_t* node);

/**
 * @} EV_UTILS_QUEUE
 */

#if defined(_WIN32)                                                     /* OS */
#   include "ev/win.h"
#else                                                                   /* OS */
#   include "ev/unix.h"
#endif                                                                  /* OS */

/**
 * @defgroup EV_Thread Thread
 * @{
 */

/**
 * @brief Thread callback
 * @param[in] arg       User data
 */
typedef void (*ev_thread_cb)(void* arg);

/**
 * @brief Thread attribute.
 */
typedef struct ev_thread_opt
{
    struct
    {
        unsigned    have_stack_size : 1;    /**< Enable stack size */
    }flags;
    size_t          stack_size;             /**< Stack size. */
} ev_thread_opt_t;

/**
 * @brief Thread local storage.
 */
typedef struct ev_tls
{
    ev_os_tls_t     tls;                    /**< Thread local storage */
} ev_tls_t;

/**
 * @brief Create thread
 * @param[out] thr  Thread handle
 * @param[in] opt   Option
 * @param[in] cb    Thread body
 * @param[in] arg   User data
 * @return          #ev_errno_t
 */
EV_API int ev_thread_init(ev_os_thread_t* thr, const ev_thread_opt_t* opt,
    ev_thread_cb cb, void* arg);

/**
 * @brief Exit thread
 * @warning Cannot be called in thread body.
 * @param[in] thr       Thread handle
 * @param[in] timeout   Timeout in milliseconds. #EV_INFINITE_TIMEOUT to wait infinite.
 * @return              #EV_ETIMEDOUT if timed out before thread terminated,
 *                      #EV_SUCCESS if thread terminated.
 */
EV_API int ev_thread_exit(ev_os_thread_t* thr, unsigned long timeout);

/**
 * @brief Get self handle
 * @return          Thread handle
 */
EV_API ev_os_thread_t ev_thread_self(void);

/**
 * @brief Get current thread id,
 * @return          Thread ID
 */
EV_API ev_os_tid_t ev_thread_id(void);

/**
 * @brief Check whether two thread handle points to same thread
 * @param[in] t1    1st thread
 * @param[in] t2    2st thread
 * @return          bool
 */
EV_API int ev_thread_equal(const ev_os_thread_t* t1, const ev_os_thread_t* t2);

/**
 * @brief Suspends the execution of the calling thread.
 * @param[in] timeout   Timeout in milliseconds.
 */
EV_API void ev_thread_sleep(uint32_t timeout);

/**
 * @brief Initialize thread local storage.
 * @param[out] tls  A pointer to thread local storage.
 * @return          #ev_errno_t
 */
EV_API int ev_tls_init(ev_tls_t* tls);

/**
 * @brief Destroy thread local storage.
 * @param[in] tls   A initialized thread local storage handler.
 */
EV_API void ev_tls_exit(ev_tls_t* tls);

/**
 * @brief Set thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @param[in] val   A thread specific value.
 */
EV_API void ev_tls_set(ev_tls_t* tls, void* val);

/**
 * @brief Get thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @return          A thread specific value.
 */
EV_API void* ev_tls_get(ev_tls_t* tls);

/**
 * @} EV_Thread
 */

struct ev_loop;

/**
 * @brief Read request
 */
typedef struct ev_read
{
    ev_list_node_t          node;               /**< Intrusive node */
    struct
    {
        ev_buf_t*           bufs;               /**< Buffer list */
        size_t              nbuf;               /**< Buffer list count */
        size_t              capacity;           /**< Total bytes of buffer */
        size_t              size;               /**< Data size */
        ev_buf_t            bufsml[EV_IOV_MAX]; /**< Bound buffer list */
    }data;                                      /**< Data field */
} ev_read_t;
#define EV_READ_INVALID     \
    {\
        EV_LIST_NODE_INIT,/* .node */\
        {/* .data */\
            NULL,                                                   /* .data.bufs */\
            0,                                                      /* .data.nbuf */\
            0,                                                      /* .data.capacity */\
            0,                                                      /* .data.size */\
            { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), },  /* .data.bufsml */\
        },\
    }

/**
 * @brief Write request
 */
typedef struct ev_write
{
    ev_list_node_t          node;               /**< Intrusive node */
    ev_buf_t*               bufs;               /**< Buffer list */
    size_t                  nbuf;               /**< Buffer list count */
    size_t                  size;               /**< Write size */
    size_t                  capacity;           /**< Total bytes need to send */
    ev_buf_t                bufsml[EV_IOV_MAX]; /**< Bound buffer list */
} ev_write_t;
#define EV_WRITE_INVALID    \
    {\
        EV_LIST_NODE_INIT,                                          /* .node */\
        NULL,                                                       /* .data.bufs */\
        0,                                                          /* .data.nbuf */\
        0,                                                          /* .data.size */\
        0,                                                          /* .data.capacity */\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), }       /* .data.bufsml */\
    }

/**
 * @defgroup EV_TIME Time
 * @{
 */

/**
 * @brief Time spec
 */
struct ev_timespec_s;

/**
 * @brief Typedef of #ev_timespec_s.
 */
typedef struct ev_timespec_s ev_timespec_t;

/**
 * @brief High-accuracy time type.
 */
struct ev_timespec_s
{
    uint64_t    tv_sec;     /**< seconds */
    uint32_t    tv_nsec;    /**< nanoseconds */
};
#define EV_TIMESPEC_INVALID \
    {\
        0,\
        0,\
    }

/**
 * @} EV_TIME
 */

#ifdef __cplusplus
}
#endif

#include "ev/mutex.h"
#include "ev/sem.h"
#include "ev/once.h"
#include "ev/shm.h"
#include "ev/shdlib.h"
#include "ev/handle.h"
#include "ev/loop.h"
#include "ev/async.h"
#include "ev/timer.h"
#include "ev/tcp.h"
#include "ev/udp.h"
#include "ev/pipe.h"
#include "ev/fs.h"
#include "ev/process.h"
#include "ev/misc.h"

#endif
