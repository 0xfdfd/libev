#ifndef __EV_TODO_H__
#define __EV_TODO_H__

#include "ev/defs.h"
#include "ev/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ev_todo;
typedef struct ev_todo ev_todo_t;

/**
 * @brief Type definition for callback to run in next loop
 * @param[in] handle    A pointer to #ev_todo_t structure
 */
typedef void(*ev_todo_cb)(ev_todo_t* todo);

/**
 * @brief TODO token type.
 */
struct ev_todo
{
    ev_list_node_t          node;           /**< List node */
    ev_todo_cb              cb;             /**< Callback */
};

/**
 * @brief Initialize #ev_todo_t to an invalid value.
 */
#define EV_TODO_INVALID     \
    {\
        EV_LIST_NODE_INIT,\
        NULL,\
    }

#ifdef __cplusplus
}
#endif

#endif
