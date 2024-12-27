#ifndef __EV_LOOP_INTERNAL_H__
#define __EV_LOOP_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum ev_ipc_frame_flag
{
    EV_IPC_FRAME_FLAG_INFORMATION = 1,
} ev_ipc_frame_flag_t;

/**
 * @brief Event loop type.
 */
struct ev_loop
{
    uint64_t hwtime; /**< A fast clock time in milliseconds */

    struct
    {
        ev_list_t idle_list;   /**< (#ev_handle::node) All idle handles */
        ev_list_t active_list; /**< (#ev_handle::node) All active handles */
    } handles;                 /**< table for handles */

    ev_list_t backlog_queue; /**< Backlog queue */
    ev_list_t endgame_queue; /**< Close queue */

    /**
     * @brief Timer context
     */
    struct
    {
        ev_map_t heap; /**< #ev_timer_t::node. Timer heap */
    } timer;

    struct
    {
        struct ev_threadpool *pool; /**< Thread pool */
        ev_list_node_t node; /**< node for #ev_threadpool_t::loop_table */

        ev_mutex_t *mutex;      /**< Work queue lock */
        ev_list_t   work_queue; /**< Work queue */
    } threadpool;

    struct
    {
        unsigned b_stop : 1; /**< Flag: need to stop */
    } mask;

    EV_LOOP_BACKEND backend; /**< Platform related implementation */
};

/**
 * @brief Get event loop for the handle.
 * @param[in] handle    handler
 * @return              Event loop
 */
EV_LOCAL ev_loop_t *ev__handle_loop(ev_handle_t *handle);

/**
 * @brief Check IPC frame header
 * @param[in] buffer    Buffer to check
 * @param[in] size      Buffer size
 * @return              bool
 */
EV_LOCAL int ev__ipc_check_frame_hdr(const void *buffer, size_t size);

/**
 * @brief Initialize IPC frame header
 * @param[out] hdr      Frame header to initialize
 * @param[in] flags     Control flags
 * @param[in] exsz      Extra information size
 * @param[in] dtsz      Data size
 */
EV_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t *hdr, uint8_t flags,
                                     uint16_t exsz, uint32_t dtsz);

/**
 * @brief Update loop time
 * @param[in] loop  loop handler
 */
EV_LOCAL void ev__loop_update_time(ev_loop_t *loop);

/**
 * @brief Get minimal length of specific \p addr type.
 * @param[in] addr  A valid sockaddr buffer
 * @return          A valid minimal length, or (socklen_t)-1 if error.
 */
EV_LOCAL socklen_t ev__get_addr_len(const struct sockaddr *addr);

/**
 * @brief Initialize #ev_write_t
 * @param[out] req  A write request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__write_init(ev_write_t *req, ev_buf_t *bufs, size_t nbuf);

/**
 * @brief Cleanup write request
 * @param[in] req   Write request
 */
EV_LOCAL void ev__write_exit(ev_write_t *req);

/**
 * @brief Initialize #ev_read_t
 * @param[out] req  A read request to be initialized
 * @param[in] bufs  Buffer list
 * @param[in] nbuf  Buffer list size, can not larger than #EV_IOV_MAX.
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__read_init(ev_read_t *req, ev_buf_t *bufs, size_t nbuf);

/**
 * @brief Cleanup read request
 * @param[in] req   read request
 */
EV_LOCAL void ev__read_exit(ev_read_t *req);

/**
 * @brief Initialize backend
 * @param[in] loop      loop handler
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__loop_init_backend(ev_loop_t *loop);

/**
 * @brief Destroy backend
 * @param[in] loop  loop handler
 */
EV_LOCAL void ev__loop_exit_backend(ev_loop_t *loop);

/**
 * @brief Wait for IO event and process
 * @param[in] loop  loop handler
 * @param[in] timeout   timeout in milliseconds
 */
EV_LOCAL void ev__poll(ev_loop_t *loop, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif
