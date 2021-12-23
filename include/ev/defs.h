/**
 * @file
 */
#ifndef __EV_DEFINES_H__
#define __EV_DEFINES_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The maximum number of iov buffers can be support.
 */
#define EV_IOV_MAX              16

#define EV_EXPAND(...)          __VA_ARGS__

#define EV_INIT_REPEAT(n, ...)   EV_INIT_REPEAT2(n, __VA_ARGS__)
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

enum ev_errno;
typedef enum ev_errno ev_errno_t;

enum ev_role;
typedef enum ev_role ev_role_t;

struct ev_handle;
typedef struct ev_handle ev_handle_t;

struct ev_loop;
typedef struct ev_loop ev_loop_t;

struct ev_timer;
typedef struct ev_timer ev_timer_t;

struct ev_once;
typedef struct ev_once ev_once_t;

struct ev_todo;
typedef struct ev_todo ev_todo_t;

struct ev_async;
typedef struct ev_async ev_async_t;

struct ev_tcp;
typedef struct ev_tcp ev_tcp_t;

struct ev_pipe;
typedef struct ev_pipe ev_pipe_t;

struct ev_shmv;
typedef struct ev_shmv ev_shmv_t;

struct ev_shmv_token;
typedef struct ev_shmv_token ev_shmv_token_t;

struct ev_write;
typedef struct ev_write ev_write_t;

struct ev_read;
typedef struct ev_read ev_read_t;

struct ev_buf;
typedef struct ev_buf ev_buf_t;

/**
 * @brief An application-defined callback function.
 *
 * Specify a pointer to this function when calling the #ev_once_execute function.
 */
typedef void(*ev_once_cb)(void);

/**
 * @brief Callback for #ev_pipe_t
 * @param[in] handle      A pipe
 */
typedef void(*ev_pipe_cb)(ev_pipe_t* handle);

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
typedef void(*ev_accept_cb)(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);

/**
 * @brief Connect callback
 * @param[in] sock      Connect socket
 * @param[in] stat      #ev_errno_t
 */
typedef void(*ev_connect_cb)(ev_tcp_t* sock, int stat);

/**
 * @brief Write callback
 * @param[in] req       Write request
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void(*ev_write_cb)(ev_write_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void(*ev_read_cb)(ev_read_t* req, size_t size, int stat);

/**
 * @brief Thread callback
 * @param[in] arg       User data
 */
typedef void (*ev_thread_cb)(void* arg);

#ifdef __cplusplus
}
#endif
#endif
