/**
 * @file
 */
#ifndef __EV_H__
#define __EV_H__

#include <stddef.h>
#include <stdarg.h>
#include "ev/errno.h"
#include "ev/defs.h"
#include "ev/list.h"
#include "ev/todo.h"
#include "ev/mutex.h"
#include "ev/sem.h"
#include "ev/thread.h"
#include "ev/shmem.h"
#include "ev/loop.h"
#include "ev/request.h"
#include "ev/timer.h"
#include "ev/async.h"
#include "ev/threadpool.h"
#include "ev/tcp.h"
#include "ev/udp.h"
#include "ev/pipe.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate minimum size of iov storage.
 * @param[in] nbuf  The number of iov buffers
 * @return          The size of iov storage in bytes
 */
#define EV_IOV_BUF_SIZE(nbuf)   EV_IOV_BUF_SIZE_INTERNAL(nbuf)

/**
 * @defgroup EV_UTILS Miscellaneous utilities
 * @{
 */

/**
 * @defgroup EV_UTILS_NET Network
 * @{
 */

/**
 * @brief Convert ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr);

/**
 * @brief Convert ip and port into network address
 * @param[in] ip    Character string contains IP
 * @param[in] port  Port
 * @param[out] addr network address structure
 * @return          #ev_errno_t
 */
int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len);

/**
 * @brief Convert network address into ip and port
 * @param[in] addr  network address structure
 * @param[out] port Port
 * @param[out] ip   A buffer to store IP string
 * @param[in] len   Buffer length
 * @return          #ev_errno_t
 */
int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len);

/**
 * @} EV_UTILS/EV_UTILS_NET
 */

/**
 * @brief Executes the specified function one time.
 *
 * No other threads that specify the same one-time initialization structure can
 * execute the specified function while it is being executed by the current thread.
 *
 * @param[in] guard     A pointer to the one-time initialization structure.
 * @param[in] cb        A pointer to an application-defined InitOnceCallback function.
 */
void ev_once_execute(ev_once_t* guard, ev_once_cb cb);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Write complete callback
 * @return          #ev_errno_t
 */
int ev_write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf, ev_write_cb cb);

/**
 * @brief Initialize #ev_write_t
 * 
 * The extend version of #ev_write_init(). You should use this function if any
 * of following condition is meet:
 *
 *   + The value of \p nbuf is larger than #EV_IOV_MAX.<br>
 *     In this case you should pass \p iov_bufs as storage, the minimum value of
 *     \p iov_size can be calculated by #EV_IOV_BUF_SIZE(). \p take the ownership
 *     of \p iov_bufs, so you cannot modify or free \p iov_bufs until \p callback
 *     is called.
 * 
 * @param[out] req          A write request to be initialized
 * @param[in] callback      Write complete callback
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer list size
 * @param[in] iov_bufs      The buffer to store IOV request
 * @param[in] iov_size      The size of \p iov_bufs in bytes
 * @return                  #ev_errno_t
 */
int ev_write_init_ext(ev_write_t* req, ev_write_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @param[in] cb    Read complete callback
 * @return          #ev_errno_t
 */
int ev_read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf, ev_read_cb cb);

/**
 * @brief Initialize #ev_write_t
 *
 * The extend version of #ev_write_init(). You should use this function if any
 * of following condition is meet:
 *
 *   + The value of \p nbuf is larger than #EV_IOV_MAX.<br>
 *     In this case you should pass \p iov_bufs as storage, the minimum value of
 *     \p iov_size can be calculated by #EV_IOV_BUF_SIZE(). \p take the ownership
 *     of \p iov_bufs, so you cannot modify or free \p iov_bufs until \p callback
 *     is called.
 *
 * @param[out] req          A write request to be initialized
 * @param[in] callback      Write complete callback
 * @param[in] bufs          Buffer list
 * @param[in] nbuf          Buffer list size
 * @param[in] iov_bufs      The buffer to store IOV request
 * @param[in] iov_size      The size of \p iov_bufs in bytes
 * @return                  #ev_errno_t
 */
int ev_read_init_ext(ev_read_t* req, ev_read_cb callback,
    ev_buf_t* bufs, size_t nbuf,
    void* iov_bufs, size_t iov_size);

/**
 * @brief Constructor for #ev_buf_t
 * @param[in] buf   Buffer
 * @param[in] len   Buffer length
 * @return          A buffer
 */
ev_buf_t ev_buf_make(void* buf, size_t len);

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
void ev_buf_make_n(ev_buf_t bufs[], size_t nbuf, ...);

/**
 * @brief Constructor for #ev_buf_t list
 * 
 * Like #ev_buf_make_n(), but accept a va_list for argument.
 * @param[out] bufs Buffer array
 * @param[in] nbuf  Buffer number
 * @param[in] ap    va_list for Buffer array
 */
void ev_buf_make_v(ev_buf_t bufs[], size_t nbuf, va_list ap);

/**
 * @} EV_UTILS
 */

#ifdef __cplusplus
}
#endif
#endif
