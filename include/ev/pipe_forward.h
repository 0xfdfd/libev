#ifndef __EV_PIPE_FORWARD_H__
#define __EV_PIPE_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_PIPE Pipe
 * @{
 */

enum ev_pipe_flags_e;

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
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void(*ev_pipe_write_cb)(ev_pipe_write_req_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void(*ev_pipe_read_cb)(ev_pipe_read_req_t* req, size_t size, int stat);

/**
 * @} EV_PIPE
 */

#ifdef __cplusplus
}
#endif
#endif
