/**
 * @file
 */
#ifndef __EV_FILE_H__
#define __EV_FILE_H__

#include "ev/handle.h"
#include "ev/file_forward.h"
#include "ev/loop_forward.h"
#include "ev/threadpool.h"
#include "ev/request.h"
#include "ev/os.h"
#include "ev/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_FILESYSTEM
 * @{
 */

enum ev_fs_req_type_e
{
    EV_FS_REQ_UNKNOWN,
    EV_FS_REQ_OPEN,
    EV_FS_REQ_READ,
    EV_FS_REQ_WRITE,
    EV_FS_REQ_FSTAT,
    EV_FS_REQ_READDIR,
    EV_FS_REQ_READFILE,
};

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

struct ev_file_s
{
    ev_handle_t                 base;           /**< Base object */
    ev_os_file_t                file;           /**< File handle */
    ev_file_close_cb            close_cb;       /**< Close callback */
    ev_list_t                   work_queue;     /**< Work queue */
};

struct ev_file_stat_s
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
    uint64_t                    st_flags;
    uint64_t                    st_gen;
    ev_timespec_t               st_atim;        /**< Time of last access */
    ev_timespec_t               st_mtim;        /**< Time of last modification */
    ev_timespec_t               st_ctim;        /**< Time of last status change */
    ev_timespec_t               st_birthtim;    /**< Time of file creation */
};

struct ev_dirent_s
{
    const char*                 name;           /**< Entry name */
    ev_dirent_type_t            type;           /**< Entry type */
};

struct ev_fs_req_s
{
    ev_fs_req_type_t            req_type;       /**< Request type */
    ev_list_node_t              node;           /**< Queue node */
    ev_threadpool_work_t        work_token;     /**< Thread pool token */
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
    }req;

    union
    {
        ev_file_stat_t          fileinfo;       /**< File information */
        ev_list_t               dirents;        /**< Dirent list */
        ev_buf_t                filecontent;    /**< File content */
    }rsp;
};

/**
 * @brief Initialize a file handle
 * @param[in] loop      Event loop
 * @param[out] file     File handle
 * @return              #ev_errno_t
 */
int ev_file_init(ev_loop_t* loop, ev_file_t* file);

/**
 * @brief Destroy a file handle
 * @param[in] file      File handle
 * @param[in] cb        Close callback
 */
void ev_file_exit(ev_file_t* file, ev_file_close_cb cb);

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
 * @param[in] file      File handle
 * @param[in] req       File token
 * @param[in] path      File path.
 * @param[in] flags     Open flags
 * @param[in] mode      Open mode
 * @param[in] cb        Open result callback
 * @return              #ev_errno_t
 */
int ev_file_open(ev_file_t* file, ev_fs_req_t* req, const char* path,
    int flags, int mode, ev_file_cb cb);

/**
 * @brief Read data.
 * @param[in] file      File handle
 * @param[in] req       File operation token
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer amount
 * @param[in] offset    Offset of file
 * @param[in] cb        Read callback
 * @return              #ev_errno_t
 */
int ev_file_read(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb);

/**
 * @brief Write data
 * @param[in] file      File handle
 * @param[in] req       File operation token
 * @param[in] bufs      Buffer list
 * @param[in] nbuf      Buffer amount
 * @param[in] offset    Offset of file
 * @param[in] cb        Write callback
 * @return              #ev_errno_t
 */
int ev_file_write(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb);

/**
 * @brief Get information about a file.
 * @param[in] file      File handle.
 * @param[in] req       File system request.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_file_cb cb);

/**
 * @brief Get all entry in directory.
 *
 * Use #ev_fs_get_first_dirent() and #ev_fs_get_next_dirent() to traverse all
 * the dirent information.
 *
 * @param[in] loop      Event loop.
 * @param[in] req       File system request
 * @param[in] path      Directory path.
 * @param[in] cb        Result callback.
 * @return              #ev_errno_t
 */
int ev_fs_readdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb);

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
int ev_fs_readfile(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb);

/**
 * @brief Cleanup file system request
 * @param[in] req       File system request
 */
void ev_fs_req_cleanup(ev_fs_req_t* req);

/**
 * @brief Get stat buffer from \p req.
 * @param[in] req       A finish file system request
 * @return              Stat buf
 */
ev_file_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req);

/**
 * @brief Get first dirent information from \p req.
 * @param[in] req       File system request.
 * @return              Dirent information.
 */
ev_dirent_t* ev_fs_get_first_dirent(ev_fs_req_t* req);

/**
 * @brief Get next dirent information.
 * @param[in] curr      Current dirent information.
 * @return              Next dirent information, or NULL if non-exists.
 */
ev_dirent_t* ev_fs_get_next_dirent(ev_dirent_t* curr);

/**
 * @brief Get content of file.
 * @param[in] req       A finish file system request
 * @return              File content buffer.
 */
ev_buf_t* ev_fs_get_filecontent(ev_fs_req_t* req);

/**
 * @} EV_FILESYSTEM
 */

#ifdef __cplusplus
}
#endif
#endif
