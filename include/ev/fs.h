#ifndef __EV_FILE_SYSTEM_H__
#define __EV_FILE_SYSTEM_H__
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
typedef enum ev_fs_req_type_e
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
} ev_fs_req_type_t;

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
#define EV_FILE_INVALID \
    {\
        EV_HANDLE_INVALID,\
        EV_OS_FILE_INVALID,\
        NULL,\
        EV_LIST_INIT,\
    }

typedef struct ev_file_map
{
    void*                       addr;       /**< The mapped address. */
    uint64_t                    size;       /**< The size of mapped address. */
    ev_file_map_backend_t       backend;    /**< Backend */
} ev_file_map_t;
#define EV_FILE_MAP_INVALID \
    {\
        NULL,\
        0,\
        EV_FILE_MAP_BACKEND_INVALID,\
    }

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
        } as_open;

        struct
        {
            int                 whence;         /**< Directive */
            int64_t             offset;         /**< Offset */
        } as_seek;

        struct
        {
            int64_t             offset;         /**< File offset */
            ev_read_t           read_req;       /**< Read token */
        } as_read;

        struct
        {
            int64_t             offset;         /**< File offset */
            ev_write_t          write_req;      /**< Write token */
        } as_write;

        struct
        {
            char*               path;           /**< Directory path */
        } as_readdir;

        struct
        {
            char*               path;           /**< File path */
        } as_readfile;

        struct
        {
            char*               path;           /**< Directory path */
            int                 mode;           /**< The mode for the new directory */
        } as_mkdir;

        struct
        {
            char*               path;           /**< Path */
            int                 recursion;      /**< Recursion delete */
        } as_remove;
    } req;

    union
    {
        ev_fs_stat_t*           stat;           /**< File information */
        ev_list_t               dirents;        /**< Dirent list */
        ev_buf_t                filecontent;    /**< File content */
    } rsp;
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
        { NULL },\
    }

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
 * @param[in] loop      Event loop. Must set to NULL if \p cb is NULL.
 * @param[out] file     File handle.
 * @param[in] req       File token. Must set to NULL if \p cb is NULL.
 * @param[in] path      File path.
 * @param[in] flags     Open flags.
 * @param[in] mode      Open mode. Only applies to future accesses of the newly
 *   created file. Ignored when \p flags does not contains #EV_FS_O_CREAT.
 * @param[in] cb        Open result callback. If set to NULL, the \p file is
 *   open in synchronous mode, so \p loop, \p req must also be NULL.
 * @return              #ev_errno_t
 */
EV_API int ev_file_open(ev_loop_t* loop, ev_file_t* file, ev_fs_req_t* req, const char* path,
    int flags, int mode, ev_file_cb cb);

/**
 * @brief Close a file handle.
 * 
 * If the file is open in synchronous mode (the callback in #ev_file_open() is
 * set to NULL), then this is a synchronous call. In this case \p cb must be NULL.
 * 
 * If the file is open in asynchronous mode, this call is also asynchronous,
 * you must wait for \p cb to actually called to release the resource.
 * 
 * @param[in] file      File handle
 * @param[in] cb        Close callback. Must set to NULL if \p file open in
 *   synchronous mode.
 */
EV_API void ev_file_close(ev_file_t* file, ev_file_close_cb cb);

/**
 * @brief Set the file position indicator for the stream pointed to by \p file.
 * @see #EV_FS_SEEK_BEG
 * @see #EV_FS_SEEK_CUR
 * @see #EV_FS_SEEK_END
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] whence    Direction.
 * @param[in] offset    Offset.
 * @param[in] cb        Result callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the resulting offset location as measured
 *   in bytes from the beginning of the file, or #ev_errno_t if failure.
 */
EV_API int64_t ev_file_seek(ev_file_t* file, ev_fs_req_t* req, int whence,
    int64_t offset, ev_file_cb cb);

/**
 * @brief Read data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[out] buff     Buffer to store data.
 * @param[in] size      Buffer size.
 * @param[in] cb        Result callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes read, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_read(ev_file_t* file, ev_fs_req_t* req, void* buff,
    size_t size, ev_file_cb cb);

/**
 * @brief Read data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] cb        Read callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes read, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_readv(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb);

/**
 * @brief Read data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[out] buff     Buffer to store data.
 * @param[in] size      Buffer size.
 * @param[in] offset    Offset of file (from the start of the file). The file
 *   offset is not changed.
 * @param[in] cb        Result callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes read, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_pread(ev_file_t* file, ev_fs_req_t* req, void* buff,
    size_t size, int64_t offset, ev_file_cb cb);

/**
 * @brief Read position data.
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file (from the start of the file). The file
 *   offset is not changed.
 * @param[in] cb        Read callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes read, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_preadv(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, int64_t offset, ev_file_cb cb);

/**
 * @brief Write data
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] data      Data to write.
 * @param[in] size      Data size.
 * @param[in] cb        Write callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes written, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_write(ev_file_t* file, ev_fs_req_t* req, const void* data,
    size_t size, ev_file_cb cb);

/**
 * @brief Write data
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] cb        Write callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes written, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_writev(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ev_file_cb cb);

/**
 * @brief Write data
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] data      Data to write.
 * @param[in] size      Data size.
 * @param[in] offset    Offset of file (from the start of the file). The file
 *   offset is not changed.
 * @param[in] cb        Write callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes written, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_pwrite(ev_file_t* file, ev_fs_req_t* req, const void* data,
    size_t size, int64_t offset, ev_file_cb cb);

/**
 * @brief Write position data
 * @param[in] file      File handle.
 * @param[in] req       File operation token. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] bufs      Buffer list.
 * @param[in] nbuf      Buffer amount.
 * @param[in] offset    Offset of file (from the start of the file). The file
 *   offset is not changed.s
 * @param[in] cb        Write callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes written, or #ev_errno_t
 *   if failure.
 */
EV_API ssize_t ev_file_pwritev(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, int64_t offset, ev_file_cb cb);

/**
 * @brief Get information about a file.
 * @param[in] file      File handle.
 * @param[in] req       File system request. Must set to NULL if \p file open
 *   in synchronous mode.
 * @param[in] cb        Result callback. Must set to NULL if \p file open in
 *   synchronous mode.
 * @return              #ev_errno_t
 */
EV_API int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_fs_stat_t* stat,
    ev_file_cb cb);

/**
 * @brief Maps a view of a file mapping into the address space of a calling process.
 * @param[out] view     The mapped object.
 * @param[in] file      The file to map. The file is safe to close after this call.
 * @param[in] size      The maximum size of the file mapping object. Set to 0 to
 *   use current size of the \p file.
 * @param[in] flags     Map flags. Can be one or more of the following attributes:
 *   + #EV_FS_S_IRUSR: Pages may be read.
 *   + #EV_FS_S_IWUSR: Pages may be written.
 *   + #EV_FS_S_IXUSR: Pages may be executed.
 *   + #EV_FS_S_IRWXU: Combine of read, write and execute.
 *   Please note that this is a best effort attempt, which means you may get extra
 *   permissions than declaration. For example, in win32 if you declare #EV_FS_S_IXUSR
 *   only, you will also get read access.
 * @return              #ev_errno_t
 */
EV_API int ev_file_mmap(ev_file_map_t* view, ev_file_t* file, uint64_t size, int flags);

/**
 * @brief Unmap the file.
 * @param[in] view      The mapped object.
 */
EV_API void ev_file_munmap(ev_file_map_t* view);

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
 * @param[in] loop      Event loop. Must set to NULL if operator in synchronous
 *   mode.
 * @param[in] req       File system request. If operation success, use
 *   #ev_fs_req_cleanup() to cleanup \p req. Missing this cause lead to memory
 *   leak.
 * @param[in] path      Directory path.
 * @param[in] cb        Result callback, set to NULL to operator in synchronous
 *   mode. In synchronous mode, \p loop must also set to NULL.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of items in the \p path, or
 *   #ev_errno_t if failure.
 */
EV_API ssize_t ev_fs_readdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb);

/**
 * @brief Read file content.
 *
 * Use #ev_fs_get_filecontent() to get file content.
 *
 * @param[in] loop      Event loop. Must set to NULL if \p cb is NULL.
 * @param[in] req       File system request. If operation success, use
 *   #ev_fs_req_cleanup() to cleanup \p req. Missing this cause lead to memory
 *   leak.
 * @param[in] path      File path.
 * @param[in] cb        Result callback. Set to NULL to operator in synchronous
 *   mode. In synchronous mode, \p loop must also set to NULL.
 * @return              In asynchronous mode, return 0 if success, or #ev_errno_t
 *   if failure. In synchronous, return the number of bytes for the file, or
 *   #ev_errno_t if failure.
 */
EV_API ssize_t ev_fs_readfile(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
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
 * @param[in] loop      Event loop. Must set to NULL if \p cb is NULL.
 * @param[in] req       File system request. Must set to NULL if \p cb is NULL.
 * @param[in] path      Directory path.
 * @param[in] mode      Creation mode.
 * @param[in] cb        Result callback. Set to NULL to operator in synchronous
 *   mode. In synchronous mode, \p loop and \p req must also set to NULL.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_mkdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    int mode, ev_file_cb cb);

/**
 * @brief Delete a name for the file system.
 * @param[in] loop      Event loop. Must set to NULL if \p cb is NULL.
 * @param[in] req       File system request. Must set to NULL if \p cb is NULL.
 * @param[in] path      File path.
 * @param[in] recursion If \p path is a directory, recursively delete all child items.
 * @param[in] cb        Result callback. Set to NULL to operator in synchronous
 *   mode. In synchronous mode, \p loop and \p req must also set to NULL.
 * @return              #ev_errno_t
 */
EV_API int ev_fs_remove(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    int recursion, ev_file_cb cb);

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

#ifdef __cplusplus
}
#endif
#endif
