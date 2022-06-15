#ifndef __EV_HANDLE_H__
#define __EV_HANDLE_H__

#include "ev/defs.h"
#include "ev/list.h"
#include "ev/loop_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ev_role
{
    EV_ROLE_UNKNOWN         = -1,                   /**< Unknown type */

    EV_ROLE_EV_HANDLE       = 0,                    /**< Type of #ev_handle_t */
    EV_ROLE_EV_TIMER        = 1,                    /**< Type of #ev_timer_t */
    EV_ROLE_EV_ASYNC        = 2,                    /**< Type of #ev_async_t */
    EV_ROLE_EV_PIPE         = 3,                    /**< Type of #ev_pipe_t */
    EV_ROLE_EV_TCP          = 4,                    /**< Type of #ev_tcp_t */
    EV_ROLE_EV_UDP          = 5,                    /**< Type of #ev_udp_t */
    EV_ROLE_EV_WORK         = 6,                    /**< Type of #ev_threadpool_work_t */
    EV_ROLE_EV_FILE         = 7,                    /**< Type of #ev_file_t */
    EV_ROLE_EV_REQ_UDP_R    = 100,                  /**< Type of #ev_udp_read_t */
    EV_ROLE_EV_REQ_UDP_W    = 101,                  /**< Type of #ev_udp_write_t */
    EV_ROLE_EV__RANGE_BEG   = EV_ROLE_EV_HANDLE,
    EV_ROLE_EV__RANGE_END   = EV_ROLE_EV_REQ_UDP_W,

    EV_ROLE_OS_SOCKET       = 1000,                 /**< OS socket */
    EV_ROLE_OS__RANGE_BEG   = EV_ROLE_OS_SOCKET,
    EV_ROLE_OS__RANGE_END   = EV_ROLE_OS_SOCKET,
} ev_role_t;

struct ev_handle;
typedef struct ev_handle ev_handle_t;

/**
 * @brief Called when a object is closed
 * @param[in] handle    A base handle
 */
typedef void(*ev_handle_cb)(ev_handle_t* handle);

/**
 * @brief Base class for all major object.
 */
struct ev_handle
{
    ev_loop_t*              loop;               /**< The event loop belong to */
    ev_list_node_t          handle_queue;       /**< Node for #ev_loop_t::handles */

    struct
    {
        ev_role_t           role;               /**< The type of this object */
        unsigned            flags;              /**< Handle flags */

        /**
         * @brief Active event counter.
         * When active event counter set to zero, the active flag is removed
         * immediately.
         */
        size_t              active_events;
    } data;                                     /**< Data field */

    struct
    {
        /**
         * @brief Backlog status.
         * | Status         | Meaning                     |
         * | -------------- | --------------------------- |
         * | EV_ENOENT      | Not in backlog queue        |
         * | EV_EEXIST      | In backlog queue            |
         */
        int                 status;
        ev_handle_cb        cb;                 /**< Callback */
        ev_list_node_t      node;               /**< Node for #ev_loop_t::backlog */
    } backlog;

    struct
    {
        ev_handle_cb        close_cb;           /**< Close callback */
        ev_list_node_t      node;                /**< Close queue token */
    } endgame;
};

/**
 * @brief Initialize #ev_handle_t to an invalid value.
 */
#define EV_HANDLE_INVALID       \
    {\
        NULL,                       /* .loop */\
        EV_LIST_NODE_INIT,          /* .handle_queue */\
        {/* .data */\
            EV_ROLE_UNKNOWN,        /* .role */\
            0,                      /* .flags */\
            0,                      /* .active_events */\
        },\
        {/* .backlog */\
            EV_ECANCELED,           /* .status */\
            NULL,                   /* .cb */\
            EV_LIST_NODE_INIT,      /* .node */\
        },\
        {/* .endgame */\
            NULL,                   /* .close_cb */\
            EV_LIST_NODE_INIT,      /* .node */\
        },\
    }

#ifdef __cplusplus
}
#endif

#endif
