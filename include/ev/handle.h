#ifndef __EV_HANDLE_H__
#define __EV_HANDLE_H__

#include "ev/defs.h"
#include "ev/list.h"
#include "ev/todo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ev_role
{
    EV_ROLE_UNKNOWN         = 0,                    /**< Unknown type */

    EV_ROLE_EV_TIMER        = 1,                    /**< Type of #ev_timer_t */
    EV_ROLE_EV_ASYNC        = 2,                    /**< Type of #ev_async_t */
    EV_ROLE_EV_PIPE         = 3,                    /**< Type of #ev_pipe_t */
    EV_ROLE_EV_TCP          = 4,                    /**< Type of #ev_tcp_t */
    EV_ROLE_EV_UDP          = 5,                    /**< Reverse */
    EV_ROLE_EV_WORK         = 6,                    /**< Type of #ev_threadpool_work_t */
    EV_ROLE_EV__RANGE_BEG   = EV_ROLE_EV_TIMER,
    EV_ROLE_EV__RANGE_END   = EV_ROLE_EV_WORK,

    EV_ROLE_OS_SOCKET       = 100,              /**< OS socket */
    EV_ROLE_OS__RANGE_BEG   = EV_ROLE_OS_SOCKET,
    EV_ROLE_OS__RANGE_END   = EV_ROLE_OS_SOCKET,
}ev_role_t;

struct ev_handle;
typedef struct ev_handle ev_handle_t;

/**
 * @brief Called when a object is closed
 * @param[in] handle    A base handle
 */
typedef void(*ev_close_cb)(ev_handle_t* handle);

struct ev_handle
{
    ev_list_node_t          node;               /**< Node for #ev_loop_t::handles */

    struct
    {
        ev_loop_t*          loop;               /**< The event loop belong to */

        ev_role_t           role;               /**< The type of this object */
        unsigned            flags;              /**< Handle flags */

        ev_close_cb         close_cb;           /**< Close callback */
        ev_todo_t           close_queue;        /**< Close queue token */
    }data;
};

/**
 * @brief Initialize #ev_handle_t to an invalid value.
 */
#define EV_HANDLE_INVALID       \
    {\
        EV_LIST_NODE_INVALID,   /* .node */\
        {/* .data */\
            NULL,               /* .loop */\
            EV_ROLE_UNKNOWN,    /* .role */\
            0,                  /* .flags */\
            NULL,               /* .close_cb */\
            EV_TODO_INVALID     /* .close_queue */\
        }\
    }

#ifdef __cplusplus
}
#endif

#endif
