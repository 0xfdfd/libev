#ifndef __EV_FILE_FORWARD_H__
#define __EV_FILE_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_FILESYSTEM FileSystem
 * @{
 */

enum ev_dirent_type_e;
typedef enum ev_dirent_type_e ev_dirent_type_t;

enum ev_fs_req_type_e;
typedef enum ev_fs_req_type_e ev_fs_req_type_t;

struct ev_file_s;
typedef struct ev_file_s ev_file_t;

struct ev_fs_req_s;
typedef struct ev_fs_req_s ev_fs_req_t;

struct ev_file_stat_s;
typedef struct ev_file_stat_s ev_file_stat_t;

struct ev_dirent_s;
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
 * @} EV_FILESYSTEM
 */

#ifdef __cplusplus
}
#endif
#endif
