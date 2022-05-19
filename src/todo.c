#include "todo.h"
#include "ev-common.h"

void ev__process_todo(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->todo.pending)) != NULL)
    {
        ev_todo_token_t* todo = EV_CONTAINER_OF(it, ev_todo_token_t, node);
        todo->status = EV_EBUSY;
        todo->cb(todo);
    }
}

void ev_todo_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb)
{
    token->cb = cb;
    token->status = EV_EINPROGRESS;
    token->loop = loop;
    ev_list_push_back(&loop->todo.pending, &token->node);
}

int ev_todo_cancel(ev_todo_token_t* token)
{
    ev_loop_t* loop = token->loop;
    if (token->status != EV_EINPROGRESS)
    {
        return EV_EBUSY;
    }

    ev_list_erase(&loop->todo.pending, &token->node);
    return EV_SUCCESS;
}
