#include "async_unix.h"
#include "io_unix.h"
#include "work.h"
#include <unistd.h>

static void _on_work_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;

    ev_todo_token_t* token;
    ev_list_node_t* node;
    ev_loop_t* loop = EV_CONTAINER_OF(io, ev_loop_t, backend.work.io);
    ev__async_pend(loop->backend.work.evtfd[0]);

    for(;;)
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
    ev_mutex_init(&loop->backend.work.mutex, 0);

    ev__asyc_eventfd(loop->backend.work.evtfd);
    ev_list_init(&loop->backend.work.queue);

    ev__nonblock_io_init(&loop->backend.work.io, loop->backend.work.evtfd[0], _on_work_unix, NULL);
    ev__nonblock_io_add(loop, &loop->backend.work.io, EV_IO_IN);
}

void ev__exit_work(ev_loop_t* loop)
{
    close(loop->backend.work.evtfd[0]);
    loop->backend.work.evtfd[0] = -1;

    close(loop->backend.work.evtfd[1]);
    loop->backend.work.evtfd[1] = -1;
}

void ev__work_submit(ev_loop_t* loop, ev_todo_token_t* token, ev_todo_cb cb)
{
    token->cb = cb;

    ev_mutex_enter(&loop->backend.work.mutex);
    {
        ev_list_push_back(&loop->backend.work.queue, &token->node);
    }
    ev_mutex_leave(&loop->backend.work.mutex);

    ev__async_post(loop->backend.work.evtfd[1]);
}
