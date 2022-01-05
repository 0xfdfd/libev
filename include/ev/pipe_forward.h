#ifndef __EV_PIPE_FORWARD_H__
#define __EV_PIPE_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PIPE Pipe
 * @{
 */

struct ev_pipe;
typedef struct ev_pipe ev_pipe_t;

struct ev_pipe_write_req;
typedef struct ev_pipe_write_req ev_pipe_write_req_t;

struct ev_pipe_read_req;
typedef struct ev_pipe_read_req ev_pipe_read_req_t;

/**
 * @brief Callback for #ev_pipe_t
 * @param[in] handle      A pipe
 */
typedef void(*ev_pipe_cb)(ev_pipe_t* handle);

/**
 * @} EV_PIPE
 */

#ifdef __cplusplus
}
#endif
#endif
