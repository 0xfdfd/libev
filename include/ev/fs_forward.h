#ifndef __EV_FILESYSTEM_FORWARD_H__
#define __EV_FILESYSTEM_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_FILESYSTEM File System
 * @{
 */

/**
 * @brief Directory type.
 */
enum ev_dirent_type_e
{
    EV_DIRENT_UNKNOWN,
    EV_DIRENT_FILE,
    EV_DIRENT_DIR,
    EV_DIRENT_LINK,
    EV_DIRENT_FIFO,
    EV_DIRENT_SOCKET,
    EV_DIRENT_CHR,
    EV_DIRENT_BLOCK
};

/**
 * @brief Typedef of #ev_dirent_type_e.
 */
typedef enum ev_dirent_type_e ev_dirent_type_t;

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
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
