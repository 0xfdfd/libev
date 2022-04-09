#ifndef __EV_FILE_FORWARD_H__
#define __EV_FILE_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_FILESYSTEM FileSystem
 * @{
 */

struct ev_file_s;
typedef struct ev_file_s ev_file_t;

struct ev_file_req_s;
typedef struct ev_file_req_s ev_file_req_t;

/**
 * @brief File close callback
 * @param[in] file      File handle
 */
typedef void (*ev_file_close_cb)(ev_file_t* file);

/**
 * @brief File operation callback
 * @param[in] file      File handle
 * @param[in] req       Request token
 */
typedef void (*ev_file_cb)(ev_file_t* file, ev_file_req_t* req);

/**
 * @} EV_FILESYSTEM
 */

#ifdef __cplusplus
}
#endif
#endif
