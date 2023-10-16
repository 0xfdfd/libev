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
 * @brief Thread-local storage.
 */
typedef struct ev_tl_storage
{
    ev_os_tl_storage_t  tls;                    /**< Thread local storage */
} ev_tl_storage_t;

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
EV_API int ev_tl_storage_init(ev_tl_storage_t* tls);

/**
 * @brief Destroy thread local storage.
 * @param[in] tls   A initialized thread local storage handler.
 */
EV_API void ev_tl_storage_exit(ev_tl_storage_t* tls);

/**
 * @brief Set thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @param[in] val   A thread specific value.
 */
EV_API void ev_tl_storage_set(ev_tl_storage_t* tls, void* val);

/**
 * @brief Get thread local value.
 * @param[in] tls   A initialized thread local storage handler.
 * @return          A thread specific value.
 */
EV_API void* ev_tl_storage_get(ev_tl_storage_t* tls);

/**
 * @} EV_Thread
 */

/**
 * @defgroup EV_MUTEX Mutex
 * @{
 */

/**
 * @brief Mutex handle type.
 */
typedef struct ev_mutex
{
    union
    {
        int             i;  /**< For static initialize */
        ev_os_mutex_t   r;  /**< Real mutex */
    }u;
}ev_mutex_t;

/**
 * @brief Initialize #ev_mutex_t to an invalid value.
 * @see ev_mutex_init()
 */
#define EV_MUTEX_INVALID    \
    {\
        {\
            0\
        }\
    }

/**
 * @brief Initialize the mutex.
 * @param[out] handle   Mutex handle
 * @param[in] recursive Force recursive mutex. Set to non-zero to force create a
 *   recursive mutex. However, a value of zero does not means it is a non-
 *   recursive mutex, it is implementation depend.
 */
EV_API void ev_mutex_init(ev_mutex_t* handle, int recursive);

/**
 * @brief Destroy the mutex object referenced by \p handle
 * @param[in] handle    Mutex object
 */
EV_API void ev_mutex_exit(ev_mutex_t* handle);

/**
 * @brief The mutex object referenced by \p handle shall be locked.
 * @param[in] handle    Mutex object
 */
EV_API void ev_mutex_enter(ev_mutex_t* handle);

/**
 * @brief Release the mutex object referenced by \p handle.
 * @param[in] handle    Mutex object
 */
EV_API void ev_mutex_leave(ev_mutex_t* handle);

/**
 * @brief If the mutex object referenced by \p handle is currently locked, the
 *   call shall return immediately.
 * @param[in] handle    Mutex object.
 * @return              #EV_SUCCESS: a lock on the mutex object referenced by \p handle is acquired.
 * @return              #EV_EBUSY: The \p handle could not be acquired because it was already locked.
 */
EV_API int ev_mutex_try_enter(ev_mutex_t* handle);

/**
 * @} EV_MUTEX
 */

/**
 * @defgroup EV_SEMAPHORE Semaphore
 * @{
 */

/**
 * @brief Semaphore handle type.
 */
typedef struct ev_sem_s
{
    union
    {
        int         i;
        ev_os_sem_t r;
    }u;
}ev_sem_t;
#define EV_SEM_INVALID { { 0 } }

/**
 * @brief Initializes the unnamed semaphore at the address pointed to by \p sem.
 * @param[out] sem      Semaphore to be initialized.
 * @param[in] value     Initial value
 */
EV_API void ev_sem_init(ev_sem_t* sem, unsigned value);

/**
 * @brief Destroy the unnamed semaphore at the address pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
EV_API void ev_sem_exit(ev_sem_t* sem);

/**
 * @brief Increments (unlocks)  the  semaphore pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
EV_API void ev_sem_post(ev_sem_t* sem);

/**
 * @brief Decrements (locks) the semaphore pointed to by \p sem.
 * @param[in] sem       Semaphore handle
 */
EV_API void ev_sem_wait(ev_sem_t* sem);

/**
 * @brief If the decrement cannot be immediately performed, then call returns an
 *   error #EV_EAGAIN instead of blocking.
 * @param[in] sem       Semaphore handle
 * @return              #EV_SUCCESS if success, #EV_EAGAIN if failed.
 */
EV_API int ev_sem_try_wait(ev_sem_t* sem);

/**
 * @} EV_SEMAPHORE
 */

/**
 * @defgroup EV_ONCE Once
 * @{
 */

/**
 * @brief Typedef of #ev_once.
 */
typedef struct ev_once ev_once_t;

/**
 * @brief An application-defined callback function.
 *
 * Specify a pointer to this function when calling the #ev_once_execute function.
 */
typedef void(*ev_once_cb)(void);

/**
 * @brief Executes the specified function one time.
 *
 * No other threads that specify the same one-time initialization structure can
 * execute the specified function while it is being executed by the current thread.
 *
 * @see #EV_ONCE_INIT
 * @param[in] guard     A pointer to the one-time initialized structure.
 * @param[in] cb        A pointer to an application-defined #ev_once_cb function.
 */
EV_API void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @} EV_ONCE
 */

/**
 * @defgroup EV_SHARED_MEMORY Shared memory
 * @{
 */

/**
 * @brief Shared memory type.
 */
typedef struct ev_shm
{
    void*                   addr;       /**< Shared memory address */
    size_t                  size;       /**< Shared memory size */
    EV_SHM_BACKEND          backend;    /**< Backend */
} ev_shm_t;
#define EV_SHM_INIT         { NULL, 0, EV_SHM_BACKEND_INVALID }

/**
 * @brief Create a new shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @param[in] size  Shared memory size
 * @return          #ev_errno_t
 */
EV_API int ev_shm_init(ev_shm_t* shm, const char* key, size_t size);

/**
 * @brief Open a existing shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @return          #ev_errno_t
 */
EV_API int ev_shm_open(ev_shm_t* shm, const char* key);

/**
 * @brief Close shared memory
 * @param[in] shm   Shared memory token
 */
EV_API void ev_shm_exit(ev_shm_t* shm);

/**
 * @brief Get shared memory address
 * @param[in] shm   Shared memory token
 * @return          Shared memory address
 */
EV_API void* ev_shm_addr(ev_shm_t* shm);

/**
 * @brief Get shared memory size
 * @param[in] shm   Shared memory token
 * @return          Shared memory size
 */
EV_API size_t ev_shm_size(ev_shm_t* shm);

/**
 * @} EV_SHARED_MEMORY
 */

struct ev_loop;

/**
 * @defgroup EV_HANDLE Handle
 * @{
 */

typedef enum ev_role
{
    EV_ROLE_UNKNOWN         = -1,                   /**< Unknown type */

    EV_ROLE_EV_HANDLE       = 0,                    /**< Type of #ev_handle_t */
    EV_ROLE_EV_TIMER        = 1,                    /**< Type of #ev_timer_t */
    EV_ROLE_EV_ASYNC        = 2,                    /**< Type of #ev_async_t */
    EV_ROLE_EV_PIPE         = 3,                    /**< Type of #ev_pipe_t */
    EV_ROLE_EV_TCP          = 4,                    /**< Type of #ev_tcp_t */
    EV_ROLE_EV_UDP          = 5,                    /**< Type of #ev_udp_t */
    EV_ROLE_EV_WORK         = 6,                    /**< Type of #ev_work_t */
    EV_ROLE_EV_FILE         = 7,                    /**< Type of #ev_file_t */
    EV_ROLE_EV_REQ_UDP_R    = 100,                  /**< Type of #ev_udp_read_t */
    EV_ROLE_EV_REQ_UDP_W    = 101,                  /**< Type of #ev_udp_write_t */
    EV_ROLE_EV__RANGE_BEG   = EV_ROLE_EV_HANDLE,
    EV_ROLE_EV__RANGE_END   = EV_ROLE_EV_REQ_UDP_W,

    EV_ROLE_OS_SOCKET       = 1000,                 /**< OS socket */
    EV_ROLE_OS__RANGE_BEG   = EV_ROLE_OS_SOCKET,
    EV_ROLE_OS__RANGE_END   = EV_ROLE_OS_SOCKET,
} ev_role_t;

struct ev_handle;
typedef struct ev_handle ev_handle_t;

/**
 * @brief Called when a object is closed
 * @param[in] handle    A base handle
 */
typedef void(*ev_handle_cb)(ev_handle_t* handle);

/**
 * @brief Base class for all major object.
 */
struct ev_handle
{
    struct ev_loop*         loop;               /**< The event loop belong to */
    ev_list_node_t          handle_queue;       /**< Node for #ev_loop_t::handles */

    struct
    {
        ev_role_t           role;               /**< The type of this object */
        unsigned            flags;              /**< Handle flags */
    } data;                                     /**< Data field */

    struct
    {
        /**
         * @brief Backlog status.
         * | Status         | Meaning                     |
         * | -------------- | --------------------------- |
         * | EV_ENOENT      | Not in backlog queue        |
         * | EV_EEXIST      | In backlog queue            |
         */
        int                 status;
        ev_handle_cb        cb;                 /**< Callback */
        ev_list_node_t      node;               /**< Node for #ev_loop_t::backlog */
    } backlog;

    struct
    {
        ev_handle_cb        close_cb;           /**< Close callback */
        ev_list_node_t      node;               /**< Close queue token */
    } endgame;
};

/**
 * @brief Initialize #ev_handle_t to an invalid value.
 */
#define EV_HANDLE_INVALID       \
    {\
        NULL,                       /* .loop */\
        EV_LIST_NODE_INIT,          /* .handle_queue */\
        {/* .data */\
            EV_ROLE_UNKNOWN,        /* .role */\
            0,                      /* .flags */\
        },\
        {/* .backlog */\
            EV_ECANCELED,           /* .status */\
            NULL,                   /* .cb */\
            EV_LIST_NODE_INIT,      /* .node */\
        },\
        {/* .endgame */\
            NULL,                   /* .close_cb */\
            EV_LIST_NODE_INIT,      /* .node */\
        },\
    }

/**
 * @} EV_HANDLE
 */

/**
 * @defgroup EV_EVENT_LOOP Event loop
 * @{
 */

/**
 * @brief Running mode of event loop.
 */
enum ev_loop_mode
{
    /**
     * @brief Runs the event loop until there are no more active and referenced
     *   handles or requests.
     *
     * Returns non-zero if #ev_loop_stop() was called and there are
     * still active handles or requests. Returns zero in all other cases.
     */
    EV_LOOP_MODE_DEFAULT,

    /**
     * @brief Poll for I/O once.
     *
     * Note that this function blocks if there are no pending callbacks. Returns
     * zero when done (no active handles or requests left), or non-zero if more
     * callbacks are expected (meaning you should run the event loop again sometime
     * in the future).
     */
    EV_LOOP_MODE_ONCE,

    /**
     * @brief Poll for i/o once but don't block if there are no pending callbacks.
     *
     * Returns zero if done (no active handles or requests left), or non-zero if
     * more callbacks are expected (meaning you should run the event loop again
     * sometime in the future).
     */
    EV_LOOP_MODE_NOWAIT,
};

typedef struct ev_work ev_work_t;

/**
 * @brief Thread pool task
 * @param[in] work  Work token
 */
typedef void (*ev_work_cb)(ev_work_t* work);

/**
 * @brief Work done callback in event loop
 * @param[in] work      Work token
 * @param[in] status    Work status
 */
typedef void (*ev_work_done_cb)(ev_work_t* work, int status);

/**
 * @brief Thread pool work token.
 */
struct ev_work
{
    ev_handle_t                     base;           /**< Base object */
    ev_queue_node_t                 node;           /**< List node */

    struct
    {
        struct ev_threadpool*       pool;           /**< Thread pool */

        /**
         * @brief Work status.
         * + #EV_ELOOP:     In queue but not called yet.
         * + #EV_EBUSY:     Already in process
         * + #EV_ECANCELED: Canceled
         * + #EV_SUCCESS:   Done
         */
        int                         status;

        ev_work_cb                  work_cb;        /**< work callback */
        ev_work_done_cb             done_cb;        /**< done callback */
    }data;
};
#define EV_WORK_INVALID \
    {\
        EV_HANDLE_INVALID,\
        EV_QUEUE_NODE_INVALID,\
        { NULL, EV_EINPROGRESS, NULL, NULL },\
    }

/**
 * @brief Typedef of #ev_loop_mode.
 */
typedef enum ev_loop_mode ev_loop_mode_t;

struct ev_loop;

/**
 * @brief Typedef of #ev_loop.
 */
typedef struct ev_loop ev_loop_t;

/**
 * @brief Type definition for callback passed to #ev_loop_walk().
 * @param[in] handle    Object handle.
 * @param[in] arg       User defined argument.
 * @return              0 to continue, otherwise stop walk.
 */
typedef int (*ev_walk_cb)(ev_handle_t* handle, void* arg);

/**
 * @brief Event loop type.
 */
struct ev_loop
{
    uint64_t                        hwtime;             /**< A fast clock time in milliseconds */

    struct
    {
        ev_list_t                   idle_list;          /**< (#ev_handle::node) All idle handles */
        ev_list_t                   active_list;        /**< (#ev_handle::node) All active handles */
    }handles;                                           /**< table for handles */

    ev_list_t                       backlog_queue;      /**< Backlog queue */
    ev_list_t                       endgame_queue;      /**< Close queue */

    /**
     * @brief Timer context
     */
    struct
    {
        ev_map_t                    heap;               /**< #ev_timer_t::node. Timer heap */
    }timer;

    struct
    {
        struct ev_threadpool*       pool;               /**< Thread pool */
        ev_list_node_t              node;               /**< node for #ev_threadpool_t::loop_table */

        ev_mutex_t                  mutex;              /**< Work queue lock */
        ev_list_t                   work_queue;         /**< Work queue */
    } threadpool;

    struct
    {
        unsigned                    b_stop : 1;         /**< Flag: need to stop */
    }mask;

    EV_LOOP_BACKEND                 backend;            /**< Platform related implementation */
};

/**
 * @brief Static initializer for #ev_loop_t.
 * @note A static initialized #ev_loop_t is not a workable event loop, please
 *   initialize with #ev_loop_init().
 */
#define EV_LOOP_INVALID        \
    {\
        0,                                      /* .hwtime */\
        { EV_LIST_INIT, EV_LIST_INIT },         /* .handles */\
        EV_LIST_INIT,                           /* .backlog_queue */\
        EV_LIST_INIT,                           /* .endgame_queue */\
        { EV_MAP_INIT(NULL, NULL) },            /* .timer */ \
        {/* .threadpool */\
            NULL,                               /* .pool */\
            EV_LIST_NODE_INIT,                  /* .node */\
            EV_MUTEX_INVALID,                   /* .mutex */\
            EV_LIST_INIT,                       /* .work_queue */\
        },\
        { 0 },                                  /* .mask */\
        EV_LOOP_PLT_INIT,                       /* .backend */\
    }

/**
 * @brief Initializes the given structure.
 * @param[out] loop     Event loop handler
 * @return              #ev_errno_t
 */
EV_API int ev_loop_init(ev_loop_t* loop);

/**
 * @brief Releases all internal loop resources.
 *
 * Call this function only when the loop has finished executing and all open
 * handles and requests have been closed, or it will return #EV_EBUSY. After
 * this function returns, the user can free the memory allocated for the loop.
 *
 * @param[in] loop      Event loop handler.
 * @return #ev_errno_t
 */
EV_API int ev_loop_exit(ev_loop_t* loop);

/**
 * @brief Stop the event loop, causing uv_run() to end as soon as possible.
 *
 * This will happen not sooner than the next loop iteration. If this function
 * was called before blocking for i/o, the loop won't block for i/o on this
 * iteration.
 *
 * @param[in] loop      Event loop handler
 */
EV_API void ev_loop_stop(ev_loop_t* loop);

/**
 * @brief This function runs the event loop.
 *
 * Checkout #ev_loop_mode_t for mode details.
 * @param[in] loop      Event loop handler
 * @param[in] mode      Running mode
 * @return              Returns zero when no active handles or requests left,
 *                      otherwise return non-zero
 * @see ev_loop_mode_t
 */
EV_API int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode);

/**
 * @brief Submit task into thread pool.
 * @param[in] loop      Event loop.
 * @param[in] token     Work token.
 * @param[in] work_cb   Work callback in thread pool.
 * @param[in] done_cb   Work done callback in event loop.
 * @return              #ev_errno_t
 */
EV_API int ev_loop_queue_work(ev_loop_t* loop, ev_work_t* token,
    ev_work_cb work_cb, ev_work_done_cb done_cb);

/**
 * @brief Cancel task.
 * @note No matter the task is canceled or not, the task always callback in the
 *   event loop.
 * @param[in] token     Work token
 * @return              #ev_errno_t
 */
EV_API int ev_loop_cancel(ev_work_t* token);

/**
 * @brief Walk the list of handles.
 * \p cb will be executed with the given arg.
 * @param[in] loop      The event loop.
 * @param[in] cb        Walk callback.
 * @param[in] arg       User defined argument.
 */
EV_API void ev_loop_walk(ev_loop_t* loop, ev_walk_cb cb, void* arg);

/**
 * @} EV_EVENT_LOOP
 */

/**
 * @defgroup EV_ASYNC Async
 * @{
 */

struct ev_async;

/**
 * @brief Typedef of #ev_async.
 */
typedef struct ev_async ev_async_t;

/**
 * @brief Type definition for callback passed to #ev_async_init().
 * @param[in] async     A pointer to #ev_async_t structure
 */
typedef void(*ev_async_cb)(ev_async_t* async);

/**
 * @brief Async handle type.
 */
struct ev_async
{
    ev_handle_t             base;               /**< Base object */
    ev_async_cb             active_cb;          /**< Active callback */
    ev_async_cb             close_cb;           /**< Close callback */
    EV_ASYNC_BACKEND        backend;            /**< Platform related fields */
};

/**
 * @brief Static initializer for #ev_async_t.
 * @note A static initialized #ev_async_t is not a valid handle.
 */
#define EV_ASYNC_INVALID    \
    {\
        EV_HANDLE_INVALID,\
        NULL,\
        NULL,\
        EV_ASYNC_PLT_INVALID,\
    }

/**
 * @brief Initialize the handle.
 *
 * A NULL callback is allowed.
 *
 * @param[in] loop      Event loop
 * @param[out] handle   A pointer to the structure
 * @param[in] cb        Active callback
 * @return              #ev_errno_t
 */
EV_API int ev_async_init(ev_loop_t* loop, ev_async_t* handle, ev_async_cb cb);

/**
 * @brief Destroy the structure.
 * @param[in] handle    Async handle
 * @param[in] close_cb  Close callback
 */
EV_API void ev_async_exit(ev_async_t* handle, ev_async_cb close_cb);

/**
 * @brief Wake up the event loop and call the async handle's callback.
 * @note MT-Safe
 * @param[in] handle    Async handle
 */
EV_API void ev_async_wakeup(ev_async_t* handle);

/**
 * @} EV_ASYNC
 */

/**
 * @defgroup EV_TIMER Timer
 * @{
 */

/**
 * @brief Typedef of #ev_timer.
 */
typedef struct ev_timer ev_timer_t;

/**
 * @brief Type definition for callback passed to #ev_timer_start().
 * @param[in] timer     A pointer to #ev_timer_t structure
 */
typedef void(*ev_timer_cb)(ev_timer_t* timer);

/**
 * @brief Timer handle type.
 */
struct ev_timer
{
    ev_handle_t             base;               /**< Base object */
    ev_map_node_t           node;               /**< #ev_loop_t::timer::heap */

    ev_timer_cb             close_cb;           /**< Close callback */

    struct
    {
        uint64_t            active;             /**< Active time */
    }data;

    struct
    {
        ev_timer_cb         cb;                 /**< User callback */
        uint64_t            timeout;            /**< Timeout */
        uint64_t            repeat;             /**< Repeat */
    }attr;
};

/**
 * @brief Initialize #ev_timer_t to an invalid value.
 */
#define EV_TIMER_INVALID    \
    {\
        EV_HANDLE_INVALID,\
        EV_MAP_NODE_INIT,\
        NULL,\
        {\
            0\
        },\
        {\
            NULL,\
            0,\
            0,\
        }\
    }

/**
 * @brief Initialize the handle.
 * @param[in] loop      A pointer to the event loop
 * @param[out] handle   The structure to initialize
 * @return              #ev_errno_t
 */
EV_API int ev_timer_init(ev_loop_t* loop, ev_timer_t* handle);

/**
 * @brief Destroy the timer
 * @warning The timer structure cannot be freed until close_cb is called.
 * @param[in] handle    Timer handle
 * @param[in] close_cb  Close callback
 */
EV_API void ev_timer_exit(ev_timer_t* handle, ev_timer_cb close_cb);

/**
 * @brief Start the timer. timeout and repeat are in milliseconds.
 *
 * If timeout is zero, the callback fires on the next event loop iteration. If
 * repeat is non-zero, the callback fires first after timeout milliseconds and
 * then repeatedly after repeat milliseconds.
 *
 * @param[in] handle    Timer handle
 * @param[in] cb        Active callback
 * @param[in] timeout   The first callback timeout
 * @param[in] repeat    Repeat timeout
 * @return              #ev_errno_t
 */
EV_API int ev_timer_start(ev_timer_t* handle, ev_timer_cb cb, uint64_t timeout, uint64_t repeat);

/**
 * @brief Stop the timer.
 *
 * the callback will not be called anymore.
 *
 * @param[in] handle    Timer handle
 */
EV_API void ev_timer_stop(ev_timer_t* handle);

/**
 * @} EV_TIMER
 */

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
 * @defgroup EV_TCP TCP
 *
 * TCP layer.
 *
 * @{
 */

/**
 * @example tcp_echo_server.c
 * This is an example for how to use #ev_tcp_t as tcp server.
 */

/**
 * @brief Typedef of #ev_tcp.
 */
typedef struct ev_tcp ev_tcp_t;

/**
 * @brief Typedef of #ev_tcp_read_req.
 */
typedef struct ev_tcp_read_req ev_tcp_read_req_t;

/**
 * @brief Typedef of #ev_tcp_write_req.
 */
typedef struct ev_tcp_write_req ev_tcp_write_req_t;

/**
 * @brief Close callback for #ev_tcp_t
 * @param[in] sock      A closed socket
 */
typedef void(*ev_tcp_close_cb)(ev_tcp_t* sock);

/**
 * @brief Accept callback
 * @param[in] lisn      Listen socket
 * @param[in] conn      Accepted socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_tcp_accept_cb)(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);

/**
 * @brief Connect callback
 * @param[in] sock      Connect socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_tcp_connect_cb)(ev_tcp_t* sock, int stat);

/**
 * @brief Write callback
 * @param[in] req       Write request token
 * @param[in] size      Write result
 */
typedef void (*ev_tcp_write_cb)(ev_tcp_write_req_t* req, ssize_t size);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read result
 */
typedef void (*ev_tcp_read_cb)(ev_tcp_read_req_t* req, ssize_t size);

/**
 * @brief TCP socket.
 */
struct ev_tcp
{
    ev_handle_t             base;               /**< Base object */
    ev_tcp_close_cb         close_cb;           /**< User close callback */

    ev_os_socket_t          sock;               /**< Socket handle */
    EV_TCP_BACKEND          backend;            /**< Platform related implementation */
};
/**
 * @brief Initialize #ev_tcp_t to an invalid value.
 * @see ev_tcp_init()
 */
#define EV_TCP_INVALID      \
    {\
        EV_HANDLE_INVALID,\
        NULL,\
        EV_OS_SOCKET_INVALID,\
        EV_TCP_BACKEND_INIT,\
    }

/**
 * @brief Read request token for TCP socket.
 */
struct ev_tcp_read_req
{
    ev_read_t               base;               /**< Base object */
    ev_tcp_read_cb          user_callback;      /**< User callback */
    EV_TCP_READ_BACKEND     backend;            /**< Backend */
};

/**
 * @brief Write request token for TCP socket.
 */
struct ev_tcp_write_req
{
    ev_write_t              base;               /**< Base object */
    ev_tcp_write_cb         user_callback;      /**< User callback */
    EV_TCP_WRITE_BACKEND    backend;            /**< Backend */
};

/**
 * @brief Initialize a tcp socket
 * @param[in] loop      Event loop
 * @param[out] tcp      TCP handle
 */
EV_API int ev_tcp_init(ev_loop_t* loop, ev_tcp_t* tcp);

/**
 * @brief Destroy socket
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 */
EV_API void ev_tcp_exit(ev_tcp_t* sock, ev_tcp_close_cb cb);

/**
 * @brief Bind the handle to an address and port.
 *
 * \p addr should point to an initialized struct sockaddr_in or struct sockaddr_in6.
 *
 * @param[in] tcp       Socket handler
 * @param[in] addr      Bind address
 * @param[in] addrlen   Address length
 * @return              #ev_errno_t
 */
EV_API int ev_tcp_bind(ev_tcp_t* tcp, const struct sockaddr* addr, size_t addrlen);

/**
 * @brief Start listening for incoming connections.
 * @param[in] sock      Listen socket
 * @param[in] backlog   The number of connections the kernel might queue
 * @return              #ev_errno_t
 */
EV_API int ev_tcp_listen(ev_tcp_t* sock, int backlog);

/**
 * @brief Accept a connection from listen socket
 * @param[in] acpt  Listen socket
 * @param[in] conn  The socket to store new connection
 * @param[in] cb    Accept callback
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_accept(ev_tcp_t* acpt, ev_tcp_t* conn, ev_tcp_accept_cb cb);

/**
 * @brief Connect to address
 * @param[in] sock  Socket handle
 * @param[in] addr  Address
 * @param[in] size  Address size
 * @param[in] cb    Connect callback
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_connect(ev_tcp_t* sock, struct sockaddr* addr, size_t size,
    ev_tcp_connect_cb cb);

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
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Send result callback
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_write(ev_tcp_t* sock, ev_tcp_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb);

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
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Read result callback
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_read(ev_tcp_t* sock, ev_tcp_read_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_read_cb cb);

/**
 * @brief Get the current address to which the socket is bound.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_getsockname(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @brief Get the address of the peer connected to the socket.
 * @param[in] sock  Socket handle
 * @param[out] name A buffer to store address
 * @param[in,out] len   buffer size
 * @return          #ev_errno_t
 */
EV_API int ev_tcp_getpeername(ev_tcp_t* sock, struct sockaddr* name, size_t* len);

/**
 * @} EV_TCP
 */

/**
 * @defgroup EV_UDP UDP
 * @{
 */

/**
 * @brief Multicast operation.
 */
enum ev_udp_membership
{
    EV_UDP_LEAVE_GROUP  = 0,    /**< Leave multicast group */
    EV_UDP_ENTER_GROUP  = 1,    /**< Join multicast group */
};

/**
 * @brief Typedef of #ev_udp_membership.
 */
typedef enum ev_udp_membership ev_udp_membership_t;

/**
 * @brief UDP socket flags.
 */
enum ev_udp_flags
{
    EV_UDP_IPV6_ONLY    = 1,    /**< Do not bound to IPv4 address */
    EV_UDP_REUSEADDR    = 2,    /**< Reuse address. Only the last one can receive message. */
};

/**
 * @brief Typedef of #ev_udp_flags.
 */
typedef enum ev_udp_flags ev_udp_flags_t;

struct ev_udp;

/**
 * @brief Typedef of #ev_udp.
 */
typedef struct ev_udp ev_udp_t;

struct ev_udp_write;

/**
 * @brief Typedef of #ev_udp_write.
 */
typedef struct ev_udp_write ev_udp_write_t;

struct ev_udp_read;

/**
 * @brief Typedef of #ev_udp_read.
 */
typedef struct ev_udp_read ev_udp_read_t;

/**
 * @brief Callback for #ev_udp_t
 * @param[in] udp   UDP handle
 */
typedef void (*ev_udp_cb)(ev_udp_t* udp);

/**
 * @brief Write callback
 * @param[in] req       Write request.
 * @param[in] size      Write result.
 */
typedef void (*ev_udp_write_cb)(ev_udp_write_t* req, ssize_t size);

/**
 * @brief Read callback
 * @param[in] req       Read callback.
 * @param[in] addr      Peer address.
 * @param[in] size      Read result.
 */
typedef void (*ev_udp_recv_cb)(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size);

/**
 * @brief UDP socket type.
 */
struct ev_udp
{
    ev_handle_t             base;               /**< Base object */
    ev_udp_cb               close_cb;           /**< Close callback */
    ev_os_socket_t          sock;               /**< OS socket */

    ev_list_t               send_list;          /**< Send queue */
    ev_list_t               recv_list;          /**< Recv queue */

    EV_UDP_BACKEND          backend;            /**< Platform related implementation */
};

/**
 * @brief Write request token for UDP socket.
 */
struct ev_udp_write
{
    ev_handle_t             handle;             /**< Base object */
    ev_write_t              base;               /**< Base request */
    ev_udp_write_cb         usr_cb;             /**< User callback */
    EV_UDP_WRITE_BACKEND    backend;            /**< Backend */
};

/**
 * @brief Read request token for UDP socket.
 */
struct ev_udp_read
{
    ev_handle_t             handle;             /**< Base object */
    ev_read_t               base;               /**< Base request */
    ev_udp_recv_cb          usr_cb;             /**< User callback */
    struct sockaddr_storage addr;               /**< Peer address */
    EV_UDP_READ_BACKEND     backend;            /**< Backend */
};

/**
 * @brief Initialize a UDP handle.
 * @param[in] loop      Event loop
 * @param[out] udp      A UDP handle to initialize
 * @param[in] domain    AF_INET / AF_INET6 / AF_UNSPEC
 * @return              #ev_errno_t
 */
EV_API int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain);

/**
 * @brief Close UDP handle
 * @param[in] udp       A UDP handle
 * @param[in] close_cb  Close callback
 */
EV_API void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb);

/**
 * @brief Open a existing UDP socket
 * @note \p udp must be a initialized handle
 * @param[in] udp       A initialized UDP handle
 * @param[in] sock      A system UDP socket
 * @return              #ev_errno_t
 */
EV_API int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock);

/**
 * @brief Bind the UDP handle to an IP address and port.
 * @param[in] udp       A UDP handle
 * @param[in] addr      struct sockaddr_in or struct sockaddr_in6 with the
 *   address and port to bind to.
 * @param[in] flags     #ev_udp_flags_t
 * @return              #ev_errno_t
 */
EV_API int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags);

/**
 * @brief Associate the UDP handle to a remote address and port, so every message
 *   sent by this handle is automatically sent to that destination.
 * @param[in] udp       A UDP handle
 * @param[in] addr      Remote address
 * @return              #ev_errno_t
 */
EV_API int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr);

/**
 * @brief Get the local IP and port of the UDP handle.
 * @param[in] udp       A UDP handle
 * @param[out] name     Pointer to the structure to be filled with the address data.
 *   In order to support IPv4 and IPv6 struct sockaddr_storage should be used.
 * @param[in,out] len   On input it indicates the data of the name field.
 *   On output it indicates how much of it was filled.
 * @return              #ev_errno_t
 */
EV_API int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len);

/**
 * @brief Get the remote IP and port of the UDP handle on connected UDP handles.
 * @param[in] udp       A UDP handle
 * @param[out] name     Pointer to the structure to be filled with the address data.
 *   In order to support IPv4 and IPv6 struct sockaddr_storage should be used.
 * @param[in,out] len   On input it indicates the data of the name field.
 *   On output it indicates how much of it was filled.
 * @return              #ev_errno_t
 */
EV_API int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len);

/**
 * @brief Set membership for a multicast address.
 * @param[in] udp               A UDP handle
 * @param[in] multicast_addr    Multicast address to set membership for.
 * @param[in] interface_addr    Interface address.
 * @param[in] membership        #ev_udp_membership_t
 * @return                      #ev_errno_t
 */
EV_API int ev_udp_set_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership);

/**
 * @brief Set membership for a source-specific multicast group.
 * @param[in] udp               A UDP handle
 * @param[in] multicast_addr    Multicast address to set membership for.
 * @param[in] interface_addr    Interface address.
 * @param[in] source_addr       Source address.
 * @param[in] membership        #ev_udp_membership_t
 * @return                      #ev_errno_t
 */
EV_API int ev_udp_set_source_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, const char* source_addr, ev_udp_membership_t membership);

/**
 * @brief Set IP multicast loop flag. Makes multicast packets loop back to local sockets.
 * @param[in] udp   A UDP handle
 * @param[in] on    bool
 * @return          #ev_errno_t
 */
EV_API int ev_udp_set_multicast_loop(ev_udp_t* udp, int on);

/**
 * @brief Set the multicast ttl.
 * @param[in] udp   A UDP handle
 * @param[in] ttl   1 through 255
 * @return          #ev_errno_t
 */
EV_API int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl);

/**
 * @brief Set the multicast interface to send or receive data on.
 * @param[in] udp               A UDP handle
 * @param[in] interface_addr    interface address.
 * @return                      #ev_errno_t
 */
EV_API int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr);

/**
 * @brief Set broadcast on or off.
 * @param[in] udp   A UDP handle
 * @param[in] on    1 for on, 0 for off
 * @return          #ev_errno_t
 */
EV_API int ev_udp_set_broadcast(ev_udp_t* udp, int on);

/**
 * @brief Set the time to live.
 * @param[in] udp   A UDP handle
 * @param[in] ttl   1 through 255.
 * @return          #ev_errno_t
 */
EV_API int ev_udp_set_ttl(ev_udp_t* udp, int ttl);

/**
 * @brief Send data over the UDP socket.
 *
 * If the socket has not previously been bound with #ev_udp_bind() it will be bound
 * to 0.0.0.0 (the "all interfaces" IPv4 address) and a random port number.
 *
 * @param[in] udp   A UDP handle
 * @param[in] req   Write request token
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] addr  Peer address
 * @param[in] cb    Send result callback
 * @return          #ev_errno_t
 */
EV_API int ev_udp_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs,
    size_t nbuf, const struct sockaddr* addr, ev_udp_write_cb cb);

/**
 * @brief Same as #ev_udp_send(), but won't queue a send request if it can't be
 *   completed immediately.
 * @param[in] udp   A UDP handle
 * @param[in] req   Write request token
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] addr  Peer address
 * @param[in] cb    Send result callback
 * @return          #ev_errno_t
 */
EV_API int ev_udp_try_send(ev_udp_t* udp, ev_udp_write_t* req, ev_buf_t* bufs,
    size_t nbuf, const struct sockaddr* addr, ev_udp_write_cb cb);

/**
 * @brief Queue a read request.
 * @param[in] udp   A UDP handle
 * @param[in] req   Read request token
 * @param[in] bufs  Receive buffer
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Receive callback
 * @return          #ev_errno_t
 */
EV_API int ev_udp_recv(ev_udp_t* udp, ev_udp_read_t* req, ev_buf_t* bufs,
    size_t nbuf, ev_udp_recv_cb cb);

/**
 * @} EV_UDP
 */

/**
 * @defgroup EV_TLS TLS
 * @{
 */

typedef struct ev_tls ev_tls_t;

typedef void (*ev_tls_cb)(ev_tls_t* tls);

struct ev_tls
{
    ev_tcp_t    tcp;
    ev_tls_cb   on_exit;
};

/**
 * @brief Initialize a tls socket.
 * @param[in] loop      Event loop
 * @param[out] tcp      TLS handle
 * @return              #ev_errno_t
 */
EV_API int ev_tls_init(ev_loop_t* loop, ev_tls_t* tls);

/**
 * @brief Destroy socket
 * @param[in] sock      Socket
 * @param[in] cb        Destroy callback
 */
EV_API void ev_tls_exit(ev_tls_t* tls, ev_tls_cb on_exit);

/**
 * @brief Bind the handle to an address and port.
 * \p addr should point to an initialized struct sockaddr_in or struct sockaddr_in6.
 * @param[in] tcp       Socket handler
 * @param[in] addr      Bind address
 * @param[in] addrlen   Address length
 * @return              #ev_errno_t
 */
EV_API int ev_tls_bind(ev_tls_t* tls, const struct sockaddr* addr, size_t addrlen);

/**
 * @brief Write data
 *
 * Once #ev_tls_write() return #EV_SUCCESS, it take the ownership of \p req, so
 * you should not modify the content of it until bounded callback is called.
 *
 * It is a guarantee that every bounded callback of \p req will be called, with
 * following scene:
 *   + If write success or failure. The callback will be called with write status.
 *   + If \p pipe is exiting but there are pending write request. The callback
 *     will be called with status #EV_ECANCELED.
 *
 * @param[in] tls   Socket handle
 * @param[in] req   Write request
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer number
 * @param[in] cb    Send result callback
 * @return          #ev_errno_t
 */
EV_API int ev_tls_write(ev_tls_t* tls, ev_tcp_write_req_t* req,
    ev_buf_t* bufs, size_t nbuf, ev_tcp_write_cb cb);

/**
 * @} // EV_TLS
 */

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

/**
 * @defgroup EV_FILESYSTEM File System
 * @{
 */

/**
 * @brief Directory type.
 */
typedef enum ev_dirent_type_e
{
    EV_DIRENT_UNKNOWN,
    EV_DIRENT_FILE,
    EV_DIRENT_DIR,
    EV_DIRENT_LINK,
    EV_DIRENT_FIFO,
    EV_DIRENT_SOCKET,
    EV_DIRENT_CHR,
    EV_DIRENT_BLOCK
} ev_dirent_type_t;

/**
 * @brief File system request type.
 */
enum ev_fs_req_type_e
{
    EV_FS_REQ_UNKNOWN,
    EV_FS_REQ_OPEN,
    EV_FS_REQ_SEEK,
    EV_FS_REQ_READ,
    EV_FS_REQ_WRITE,
    EV_FS_REQ_FSTAT,
    EV_FS_REQ_READDIR,
    EV_FS_REQ_READFILE,
    EV_FS_REQ_MKDIR,
    EV_FS_REQ_REMOVE,
};

/**
 * @brief Typedef of #ev_fs_req_type_e.
 */
typedef enum ev_fs_req_type_e ev_fs_req_type_t;

struct ev_file_s;

/**
 * @brief Typedef of #ev_file_s.
 */
typedef struct ev_file_s ev_file_t;

struct ev_fs_req_s;

/**
 * @brief Typedef of #ev_fs_req_s.
 */
typedef struct ev_fs_req_s ev_fs_req_t;

struct ev_fs_stat_s;

/**
 * @brief Typedef of #ev_fs_stat_s.
 */
typedef struct ev_fs_stat_s ev_fs_stat_t;

struct ev_dirent_s;

/**
 * @brief Typedef of #ev_dirent_s.
 */
typedef struct ev_dirent_s ev_dirent_t;

/**
 * @brief File close callback
 * @param[in] file      File handle
 */
typedef void (*ev_file_close_cb)(ev_file_t* file);

/**
 * @brief File operation callback
 * @note Always call #ev_fs_req_cleanup() to free resource in \p req.
 * @warning Missing call to #ev_fs_req_cleanup() will cause resource leak.
 * @param[in] req       Request token
 */
typedef void (*ev_file_cb)(ev_fs_req_t* req);

/**
 * @brief File type.
 */
struct ev_file_s
{
    ev_handle_t                 base;           /**< Base object */
    ev_os_file_t                file;           /**< File handle */
    ev_file_close_cb            close_cb;       /**< Close callback */
    ev_list_t                   work_queue;     /**< Work queue */
};

/**
 * @brief File status information.
 */
struct ev_fs_stat_s
{
    uint64_t                    st_dev;         /**< ID of device containing file */
    uint64_t                    st_ino;         /**< Inode number */
    uint64_t                    st_mode;        /**< File type and mode */
    uint64_t                    st_nlink;       /**< Number of hard links */
    uint64_t                    st_uid;         /**< User ID of owner */
    uint64_t                    st_gid;         /**< Group ID of owner */
    uint64_t                    st_rdev;        /**< Device ID (if special file) */

    uint64_t                    st_size;        /**< Total size, in bytes */
    uint64_t                    st_blksize;     /**< Block size for filesystem I/O */
    uint64_t                    st_blocks;      /**< Number of 512B blocks allocated */
    uint64_t                    st_flags;       /**< File flags */
    uint64_t                    st_gen;         /**< Generation number of this i-node. */

    ev_timespec_t               st_atim;        /**< Time of last access */
    ev_timespec_t               st_mtim;        /**< Time of last modification */
    ev_timespec_t               st_ctim;        /**< Time of last status change */
    ev_timespec_t               st_birthtim;    /**< Time of file creation */
};
#define EV_FS_STAT_INVALID  \
    {\
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
        EV_TIMESPEC_INVALID,\
        EV_TIMESPEC_INVALID,\
        EV_TIMESPEC_INVALID,\
        EV_TIMESPEC_INVALID,\
    }

/**
 * @brief Directory entry.
 */
struct ev_dirent_s
{
    char*                       name;           /**< Entry name */
    ev_dirent_type_t            type;           /**< Entry type */
};

/**
 * @brief File system request token.
 */
struct ev_fs_req_s
{
    ev_fs_req_type_t            req_type;       /**< Request type */
    ev_list_node_t              node;           /**< Queue node */
    ev_work_t                   work_token;     /**< Thread pool token */
    ev_file_t*                  file;           /**< File handle */
    ev_file_cb                  cb;             /**< File operation callback */
    ssize_t                     result;         /**< Result */

    union
    {
        struct
        {
            char*               path;           /**< File path */
            int                 flags;          /**< File flags */
            int                 mode;           /**< File mode */
        }as_open;

        struct
        {
            int                 whence;         /**< Directive */
            ssize_t             offset;         /**< Offset */
        } as_seek;

        struct
        {
            ssize_t             offset;         /**< File offset */
            ev_read_t           read_req;       /**< Read token */
        }as_read;

        struct
        {
            ssize_t             offset;         /**< File offset */
            ev_write_t          write_req;      /**< Write token */
        }as_write;

        struct
        {
            char*               path;           /**< Directory path */
        }as_readdir;

        struct
        {
            char*               path;           /**< File path */
        }as_readfile;

        struct
        {
            char*               path;           /**< Directory path */
            int                 mode;           /**< The mode for the new directory */
        }as_mkdir;

        struct
        {
            char*               path;           /**< Path */
            int                 recursion;      /**< Recursion delete */
        } as_remove;
    }req;

    union
    {
        ev_fs_stat_t            fileinfo;       /**< File information */
        ev_list_t               dirents;        /**< Dirent list */
        ev_buf_t                filecontent;    /**< File content */
    }rsp;
};

#define EV_FS_REQ_INVALID \
    {\
        EV_FS_REQ_UNKNOWN,\
        EV_LIST_NODE_INIT,\
        EV_WORK_INVALID,\
        NULL,\
        NULL,\
        EV_EINPROGRESS,\
        { { NULL, 0, 0 } },\
        { EV_FS_STAT_INVALID },\
    }

/**
 * @brief Initialize a file handle
 * @param[in] loop      Event loop
 * @param[out] file     File handle
 * @return              #ev_errno_t
 */
EV_API int ev_file_init(ev_loop_t* loop, ev_file_t* file);

/**
 * @brief Destroy a file handle
 * @param[in] file      File handle
 * @param[in] cb        Close callback
 */
EV_API void ev_file_exit(ev_file_t* file, ev_file_close_cb cb);

/**
 * @brief Equivalent to [open(2)](https://man7.org/linux/man-pages/man2/open.2.html).
 * 
 * The full list of \p flags are:
 * + #EV_FS_O_APPEND
 * + #EV_FS_O_CREAT
 * + #EV_FS_O_DSYNC
 * + #EV_FS_O_EXCL
 * + #EV_FS_O_SYNC
 * + #EV_FS_O_TRUNC
 * + #EV_FS_O_RDONLY
 * + #EV_FS_O_WRONLY
 * + #EV_FS_O_RDWR
 * 
 * The full list of \p mode are:
 * + #EV_FS_S_IRUSR
 * + #EV_FS_S_IWUSR
 * + #EV_FS_S_IXUSR
 * + #EV_FS_S_IRWXU
 * 
 * @note File always open in binary mode.
 * @param[in] file      File handle.
 * @param[in] req       File token.
 * @param[in] path      File path.
 * @param[in] flags     Open flags
 * @param[in] mode      Open mode.
 * @param[in] cb        Open result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_open(ev_file_t* file, ev_fs_req_t* req, const char* path,
    int flags, int mode, ev_file_cb cb);

/**
 * @brief Like #ev_file_open(), but work in synchronous mode.
 * @see ev_file_open()
 * @param[in] file      File handle.
 * @param[in] path      File path.
 * @param[in] flags     Open flags.
 * @param[in] mode      Open mode.
 * @return              #ev_errno_t
 */
EV_API int ev_file_open_sync(ev_file_t* file, const char* path, int flags,
    int mode);

/**
 * @brief Set the file position indicator for the stream pointed to by \p file.
 * @see #EV_FS_SEEK_BEG
 * @see #EV_FS_SEEK_CUR
 * @see #EV_FS_SEEK_END
 * @param[in] file      File handle.
 * @param[in] req       File operation token.
 * @param[in] whence    Direction.
 * @param[in] offset    Offset.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_seek(ev_file_t* file, ev_fs_req_t* req, int whence,
    ssize_t offset, ev_file_cb cb);

/**
 * @brief Read data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] cb        Read callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_read(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb);

/**
 * @brief Like #ev_file_read(), but work in synchronous mode.
 * @see ev_file_read()
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @return              #ev_errno_t
 */
EV_API ssize_t ev_file_read_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf);

/**
 * @brief Read position data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file.
 * @param[in] cb        Read callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_pread(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb);

/**
 * @brief Like #ev_file_pread(), but work in synchronous mode.
 * @see ev_file_pread()
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file.
 * @return              #ev_errno_t
 */
EV_API ssize_t ev_file_pread_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf,
    ssize_t offset);

/**
 * @brief Write data
 * @param[in] file      File handle.
 * @param[in] req       File operation token.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file.
 * @param[in] cb        Write callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_write(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb);

/**
 * @brief Like #ev_file_pwrite(), but work in synchronous mode.
 * @see ev_file_write()
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @return              #ev_errno_t
 */
EV_API ssize_t ev_file_write_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf);

/**
 * @brief Write position data
 * @param[in] file      File handle.
 * @param[in] req       File operation token.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file.
 * @param[in] cb        Write callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_pwrite(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb);

/**
 * @brief Like #ev_file_pwrite(), but work in synchronous mode.
 * @see ev_file_pwrite()
 * @param[in] file      File handle.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file.
 * @return              #ev_errno_t
 */
EV_API ssize_t ev_file_pwrite_sync(ev_file_t* file, ev_buf_t bufs[], size_t nbuf,
    ssize_t offset);

/**
 * @brief Get information about a file.
 * @param[in] file      File handle.
 * @param[in] req       File system request.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_file_cb cb);

/**
 * @brief Like #ev_file_stat(), but work in synchronous mode.
 * @see ev_file_stat()
 * @param[in] file      File handle.
 * @param[out] stat     File status.
 * @return              #ev_errno_t
 */
EV_API int ev_file_stat_sync(ev_file_t* file, ev_fs_stat_t* stat);

/**
 * @brief Get all entry in directory.
 *
 * Use #ev_fs_get_first_dirent() and #ev_fs_get_next_dirent() to traverse all
 * the dirent information.
 *
 * The #ev_fs_req_t::result in \p cb means:
 * | Range | Means                      |
 * | ----- | -------------------------- |
 * | >= 0  | The number of dirent nodes |
 * | < 0   | #ev_errno_t                |
 * 
 * @param[in] loop      Event loop.
 * @param[in] req       File system request
 * @param[in] path      Directory path.
 * @param[in] callback  Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_readdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb callback);

/**
 * @brief Read file content.
 *
 * Use #ev_fs_get_filecontent() to get file content.
 *
 * @param[in] loop      Event loop.
 * @param[in] req       File system request.
 * @param[in] path      File path.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_readfile(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb);

/**
 * @brief Create the DIRECTORY(ies), if they do not already exist.
 *
 * The full list of \p mode are:
 * + #EV_FS_S_IRUSR
 * + #EV_FS_S_IWUSR
 * + #EV_FS_S_IXUSR
 * + #EV_FS_S_IRWXU
 *
 * @param[in] loop      Event loop.
 * @param[in] req       File system request.
 * @param[in] path      Directory path.
 * @param[in] mode      Creation mode.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_mkdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    int mode, ev_file_cb cb);

/**
 * @brief Like #ev_fs_mkdir(), but work in synchronous mode.
 * @see ev_fs_mkdir()
 * @param[in] path      Directory path.
 * @param[in] mode      Creation mode.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_mkdir_sync(const char* path, int mode);

/**
 * @brief Delete a name for the file system.
 * @param[in] loop      Event loop.
 * @param[in] req       File system request.
 * @param[in] path      File path.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_remove(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    int recursion, ev_file_cb cb);

/**
 * @brief Like #ev_fs_remove(), but work in synchronous mode.
 * @param[in] path      File path.
 * @param[in] recursion Recursion delete if path is a directory.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_remove_sync(const char* path, int recursion);

/**
 * @brief Cleanup file system request
 * @param[in] req       File system request
 */
EV_API void ev_fs_req_cleanup(ev_fs_req_t* req);

/**
 * @brief Get file handle from request.
 * @param[in] req       File system request.
 * @return              File handle.
 */
EV_API ev_file_t* ev_fs_get_file(ev_fs_req_t* req);

/**
 * @brief Get stat buffer from \p req.
 * @param[in] req       A finish file system request
 * @return              Stat buf
 */
EV_API ev_fs_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req);

/**
 * @brief Get first dirent information from \p req.
 * @param[in] req       File system request.
 * @return              Dirent information.
 */
EV_API ev_dirent_t* ev_fs_get_first_dirent(ev_fs_req_t* req);

/**
 * @brief Get next dirent information.
 * @param[in] curr      Current dirent information.
 * @return              Next dirent information, or NULL if non-exists.
 */
EV_API ev_dirent_t* ev_fs_get_next_dirent(ev_dirent_t* curr);

/**
 * @brief Get content of file.
 * @param[in] req       A finish file system request
 * @return              File content buffer.
 */
EV_API ev_buf_t* ev_fs_get_filecontent(ev_fs_req_t* req);

/**
 * @} EV_FILESYSTEM
 */

/**
 * @defgroup EV_PROCESS Process
 * @{
 */

/**
 * @brief Typedef of #ev_process_exit_status_e.
 */
typedef enum ev_process_exit_status_e
{
    /**
     * @brief The child terminated, but we don't known how or why.
     */
    EV_PROCESS_EXIT_UNKNOWN,

    /**
     * @brief The child terminated normally, that is, by calling exit(3) or
     *   _exit(2), or by returning from main().
     */
    EV_PROCESS_EXIT_NORMAL,

    /**
     * @brief The child process was terminated by a signal.
     */
    EV_PROCESS_EXIT_SIGNAL,
} ev_process_exit_status_t;

/**
 * @brief Typedef of #ev_process_stdio_flags_e.
 */
typedef enum ev_process_stdio_flags_e
{
    /**
     * @brief Ignore this field.
     */
    EV_PROCESS_STDIO_IGNORE         = 0x00,

    /**
     * @brief Redirect stdio from/to `/dev/null`.
     */
    EV_PROCESS_STDIO_REDIRECT_NULL  = 0x01,

    /**
     * @brief Redirect stdio from/to file descriptor.
     * @note The fd will not closed automatically, so you need to do it by
     *   yourself.
     */
    EV_PROCESS_STDIO_REDIRECT_FD    = 0x02,

    /**
     * @brief Redirect stdio from/to #ev_pipe_t.
     *
     * The #ev_process_stdio_container_t::data::pipe field must point to a
     * #ev_pipe_t object that has been initialized with #ev_pipe_init() but not
     * yet opened or connected.
     */
    EV_PROCESS_STDIO_REDIRECT_PIPE  = 0x04,
} ev_process_stdio_flags_t;

/**
 * @brief Typedef of #ev_process_s.
 */
typedef struct ev_process_s ev_process_t;

/**
 * @brief Child process exit callback
 * @param[in] handle        Process handle.
 * @param[in] exit_status   Exit status
 *   +EV_PROCESS_EXIT_UNKNOWN: \p exit_code is meaninglessness.
 *   +EV_PROCESS_EXIT_NORMAL: \p exit_code is the exit status of the child.
 *     This consists of the least significant 8 bits of the status argument
 *     that the child specified in a call to exit(3) or _exit(2) or as the
 *     argument for a return state‐ment in main().
 *   +EV_PROCESS_EXIT_SIGNAL: \p exit_code is the number of the signal that
 *     caused the child process to terminate.
 * @param[in] exit_code     Exit code.
 */
typedef void (*ev_process_sigchld_cb)(ev_process_t* handle,
        ev_process_exit_status_t exit_status, int exit_code);

/**
 * @brief Handle exit callback.
 * @param[in] handle    Process handle.
 */
typedef void (*ev_process_exit_cb)(ev_process_t* handle);

/**
 * @brief Process stdio container.
 */
typedef struct ev_process_stdio_container_s
{
    /**
     * @brief Bit-OR of #ev_process_stdio_flags_t controls how a stdio should
     *   be transmitted to the child process.
     */
    ev_process_stdio_flags_t        flag;

    /**
     * @brief Set data according to ev_process_stdio_container_t::flag.
     */
    union
    {
        /**
         * @brief Valid if #ev_process_stdio_container_t::flag set to
         *   #EV_PROCESS_STDIO_REDIRECT_FD.
         * You must close it when no longer needed.
         */
        ev_os_pipe_t                fd;

        /**
         * @brief Valid if #ev_process_stdio_container_t::flag set to
         *   #EV_PROCESS_STDIO_REDIRECT_PIPE.
         */
        ev_pipe_t*                  pipe;
    } data;
} ev_process_stdio_container_t;

/**
 * @brief Process executable options.
 */
typedef struct ev_process_options_s
{
    /**
     * @brief (Optional) Child process exit callback.
     */
    ev_process_sigchld_cb           on_exit;

    /**
     * @brief Execute file.
     */
    const char*                     file;

    /**
     * @brief Execute command line.
     */
    char* const*                    argv;

    /**
     * @brief (Optional) Environment list.
     */
    char* const*                    envp;

    /**
     * @brief (Optional) Current working directory.
     */
    const char*                     cwd;

    /**
     * @brief (Optional) Pipe for redirect stdin / stdout / stderr.
     */
    ev_process_stdio_container_t    stdios[3];
} ev_process_options_t;

/**
 * @brief Process context.
 */
struct ev_process_s
{
    ev_list_node_t                  node;           /**< List node */

    ev_process_sigchld_cb           sigchild_cb;    /**< Process exit callback */
    ev_process_exit_cb              exit_cb;        /**< Exit callback. */

    ev_os_pid_t                     pid;            /**< Process ID */
    ev_process_exit_status_t        exit_status;    /**< Exit status */
    int                             exit_code;      /**< Exit code or termainl signal  */
    ev_async_t                      sigchld;        /**< SIGCHLD notifier */

    EV_PROCESS_BACKEND              backend;
};

/**
 * @brief Execute process.
 * @param[in] loop      Event loop handler.
 * @param[out] handle   Child Process Identifier.
 * @param[in] opt       Process options.
 * @return              #ev_errno_t
 */
EV_API int ev_process_spawn(ev_loop_t* loop, ev_process_t* handle,
    const ev_process_options_t* opt);

/**
 * @brief Exit process handle.
 * @param[in] handle    Process handle.
 * @param[in] cb        Exit callback.
 */
EV_API void ev_process_exit(ev_process_t* handle, ev_process_exit_cb cb);

/**
 * @brief Notify when process receive SIGCHLD.
 * 
 * Signal SIGCHLD must be register by user application to known child process
 * exit status. If not register, waitpid() may fail with `ECHILD`.
 *
 * By default we register SIGCHLD on initialize so you don't need to care about
 * it. But there are some situation you might to register SIGCHLD yourself, so
 * when you do it, remember to call #ev_process_sigchld() in your SIGCHLD
 * handler.
 *
 * @param[in] signum    Must always SIGCHLD.
 */
EV_API void ev_process_sigchld(int signum);

/**
 * @brief Get current working directory.
 * @note The trailing slash is always removed.
 * @param[out] buffer   Buffer to store string. The terminating null byte is
 *   always appended.
 * @param[in] size      Buffer size.
 * @return The number of bytes would have been written to the buffer (excluding
 *   the terminating null byte). Thus, a return value of \p size or more means
 *   that the output was truncated.
 * @return #ev_errno_t if error occur.
 */
EV_API ssize_t ev_getcwd(char* buffer, size_t size);

/**
 * @brief Gets the executable path.
 * @param[out] buffer   Buffer to store string. The terminating null byte is
 *   always appended.
 * @param[in] size      Buffer size.
 * @return The number of bytes would have been written to the buffer (excluding
 *   the terminating null byte). Thus, a return value of \p size or more means
 *   that the output was truncated.
 * @return #ev_errno_t if error occur.
 */
EV_API ssize_t ev_exepath(char* buffer, size_t size);

/**
 * @}
 */

/**
 * @defgroup EV_MISC Miscellaneous utilities
 * @{
 */

/**
 * @defgroup EV_MISC_NET Network
 * @{
 */

/**
 * @brief Convert IPv4 ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
EV_API int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr);

/**
 * @brief Convert IPv6 ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
EV_API int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr);

/**
 * @brief Convert ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
EV_API int ev_ip_addr(const char* ip, int port, struct sockaddr* addr, size_t size);

/**
 * @brief Convert IPv4 network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
EV_API int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len);

/**
 * @brief Convert IPv6 network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
EV_API int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
EV_API int ev_ip_name(const struct sockaddr* addr, int* port, char* ip, size_t len);

/**
 * @} EV_MISC_NET
 */

/**
 * @brief Constructor for #ev_buf_t
 * @param[in] buf   Buffer
 * @param[in] len   Buffer length
 * @return          A buffer
 */
EV_API ev_buf_t ev_buf_make(void* buf, size_t len);

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
EV_API void ev_buf_make_n(ev_buf_t bufs[], size_t nbuf, ...);

/**
 * @brief Constructor for #ev_buf_t list
 * 
 * Like #ev_buf_make_n(), but accept a va_list for argument.
 * @param[out] bufs Buffer array
 * @param[in] nbuf  Buffer number
 * @param[in] ap    va_list for Buffer array
 */
EV_API void ev_buf_make_v(ev_buf_t bufs[], size_t nbuf, va_list ap);

/**
 * @brief Release any global state that holding onto.
 * @warning Only call #ev_library_shutdown() once.
 * @warning Don’t call #ev_library_shutdown() when there are still event loops
 *   or I/O requests active.
 * @warning Don’t call libev functions after calling #ev_library_shutdown().
 */
EV_API void ev_library_shutdown(void);

/**
 * @brief Returns the current high-resolution real time in microsecond.
 * @return Time in microsecond.
 */
EV_API uint64_t ev_hrtime(void);

/**
 * @} EV_MISC
 */

#ifdef __cplusplus
}
#endif

#include "ev/lua.h"

#endif
