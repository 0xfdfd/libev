#include "win/loop.h"
#include "work.h"

static void _on_work_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;

    ev_todo_token_t* token;
    ev_list_node_t* node;
    ev_loop_t* loop = EV_CONTAINER_OF(iocp, ev_loop_t, backend.work.io);

    for (;;)
    {
        ev_mutex_enter(&loop->backend.work.mutex);
        {
            node = ev_list_pop_front(&loop->backend.work.queue);
        }
        ev_mutex_leave(&loop->backend.work.mutex);

        if (node == NULL)
        {
            break;
        }

        token = EV_CONTAINER_OF(node, ev_todo_token_t, node);
        token->cb(token);
    }
}

void ev__init_work(ev_loop_t* loop)
{
    ev__iocp_init(&loop->backend.work.io, _on_work_win, NULL);
    ev_mutex_init(&loop->backend.work.mutex, 0);
    ev_list_init(&loop->backend.work.queue);
}

void ev__exit_work(ev_loop_t* loop)
{
    (void)loop;
}

void ev__work_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb)
{
    token->cb = cb;

    ev_mutex_enter(&loop->backend.work.mutex);
    {
        ev_list_push_back(&loop->backend.work.queue, &token->node);
    }
    ev_mutex_leave(&loop->backend.work.mutex);

    ev__iocp_post(loop, &loop->backend.work.io);
}
