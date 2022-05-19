#ifndef __EV_TODO_H__
#define __EV_TODO_H__

#include "ev/defs.h"
#include "ev/list.h"
#include "ev/loop_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ev_todo_token;
typedef struct ev_todo_token ev_todo_token_t;

/**
 * @brief Type definition for callback to run in next loop
 * @param[in] handle    A pointer to #ev_todo_token_t structure.
 */
typedef void(*ev_todo_cb)(ev_todo_token_t* todo);

/**
 * @brief work token type.
 */
struct ev_todo_token
{
    ev_list_node_t          node;           /**< List node */
    ev_loop_t*              loop;           /**< Event loop */
    ev_todo_cb              cb;             /**< Callback */

    /**
     * @brief Work status.
     * + EV_EINPROGRESS: Submit but not callback yet
     * + EV_EBUSY: Going to callback
     */
    int                     status;         /**< Status */
};

/**
 * @brief Initialize #ev_todo_token_t to an invalid value.
 */
#define EV_TODO_TOKEN_INVALID     \
    {\
        EV_LIST_NODE_INIT,\
        NULL,\
        NULL,\
        EV_UNKNOWN,\
    }

/**
 * @brief Submit one time task to event loop without multi-thread support.
 * @not MT-UnSafe
 * @param[in] loop      Event loop
 * @param[in] token     A pointer to the pending token
 * @param[in] cb        A callback when the pending task is active
 */
void ev_todo_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb);

/**
 * @brief Cancel task submit by #ev_todo_submit().
 * @note MT-UnSafe. You must use this function in the same thread as \p loop in
 *   #ev_todo_submit().
 * @return EV_SUCCESS:  Operation success.
 * @return EV_EBUSY:    Task is already in callback.
 */
int ev_todo_cancel(ev_todo_token_t* token);

#ifdef __cplusplus
}
#endif

#endif
