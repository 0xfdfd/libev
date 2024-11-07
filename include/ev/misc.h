#ifndef __EV_MISC_H__
#define __EV_MISC_H__
#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Get system page size.
 * @return The page size of this system.
 */
EV_API size_t ev_os_page_size(void);

/**
 * @brief Offset granularity for #ev_file_mmap().
 * @note The Offset granularity does not always equal to system page size.
 * @return Offset granularity.
 */
EV_API size_t ev_os_mmap_offset_granularity(void);

/**
 * @defgroup EV_MISC_RANDOM Random
 * @{
 */

/**
 * @brief Typedef of random token.
 */
typedef struct ev_random_req ev_random_req_t;

/**
 * @brief Random callback.
 * @param[in] req       The request passed to #ev_random().
 * @param[in] status    Operation result. See #ev_errno_t.
 * @param[in] buf       The buf passed to #ev_random().
 * @param[in] len       The len passed to #ev_random().
 */
typedef void (*ev_random_cb)(ev_random_req_t* req, int status, void* buf, size_t len);

/**
 * @brief Asynchronous random request token.
 */
struct ev_random_req
{
    ev_work_t       work;
    void*           buf;
    size_t          len;
    int             flags;
    ev_random_cb    cb;
    int             ret;
};

/**
 * @brief Fill \p buf with exactly \p len cryptographically strong randoms
 *   bytes acquired from the system CSPRNG.
 * @param[in] loop  Event loop. If set to NULL, this function operation in
 *   synchronous mode.
 * @param[in] req   Random request. In synchronous mode, it must set to NULL.
 * @param[in] buf   The buffer to fill.
 * @param[in] len   The number of bytes to fill.
 * @param[in] flags Reserved for future extension and must currently be 0.
 * @param[in] cb    Result callback. In synchronous mode, it must set to NULL.
 * @return          #ev_errno_t. When success, short reads are not possible.
 */
EV_API int ev_random(ev_loop_t* loop, ev_random_req_t* req, void* buf,
    size_t len, int flags, ev_random_cb cb);

/**
 * @} EV_MISC_RANDOM
 */

/**
 * @} EV_MISC
 */

#ifdef __cplusplus
}
#endif
#endif
