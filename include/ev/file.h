#ifndef __EV_FILE_H__
#define __EV_FILE_H__

#include "ev/handle.h"
#include "ev/file_forward.h"
#include "ev/loop_forward.h"
#include "ev/threadpool.h"
#include "ev/request.h"
#include "ev/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_FILE File
 * @{
 */

struct ev_file_s
{
    ev_handle_t                 base;           /**< Base object */
    ev_os_file_t                file;           /**< File handle */
    ev_file_close_cb            close_cb;       /**< Close callback */
};

struct ev_file_req_s
{
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
    }op;
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
 * @brief Open file.
 * @note File always open in binary mode.
 * @param[in] file      File handle
 * @param[in] req       File token
 * @param[in] path      File path.
 * @param[in] flags     Open flags
 * @param[in] mode      Open mode
 * @param[in] cb        Open result callback
 * @return              #ev_errno_t
 */
int ev_file_open(ev_file_t* file, ev_file_req_t* req, const char* path,
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
int ev_file_read(ev_file_t* file, ev_file_req_t* req, ev_buf_t bufs[],
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
int ev_file_write(ev_file_t* file, ev_file_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb);

/**
 * @} EV_FILE
 */

#ifdef __cplusplus
}
#endif
#endif
