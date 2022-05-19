#include "todo.h"
#include "loop.h"

void ev__init_todo(ev_loop_t* loop)
{
    ev_list_init(&loop->todo.pending);
}

void ev__process_todo(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->todo.pending)) != NULL)
    {
        ev_todo_token_t* todo = EV_CONTAINER_OF(it, ev_todo_token_t, node);
        todo->cb(todo);
    }
}

void ev_todo_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb)
{
    token->cb = cb;
    ev_list_push_back(&loop->todo.pending, &token->node);
}
