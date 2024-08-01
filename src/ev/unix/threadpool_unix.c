
static void _on_work_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)evts; (void)arg;

    ev_loop_t* loop = EV_CONTAINER_OF(io, ev_loop_t, backend.threadpool.io);
    ev__async_pend(loop->backend.threadpool.evtfd[0]);

    ev__threadpool_process(loop);
}

EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop)
{
    ev__async_post(loop->backend.threadpool.evtfd[1]);
}

EV_LOCAL void ev__init_work(ev_loop_t* loop)
{
    ev__asyc_eventfd(loop->backend.threadpool.evtfd);

    ev__nonblock_io_init(&loop->backend.threadpool.io, loop->backend.threadpool.evtfd[0], _on_work_unix, NULL);
    ev__nonblock_io_add(loop, &loop->backend.threadpool.io, EV_IO_IN);
}

EV_LOCAL void ev__exit_work(ev_loop_t* loop)
{
    ev__async_eventfd_close(loop->backend.threadpool.evtfd[0]);
    loop->backend.threadpool.evtfd[0] = -1;

    ev__async_eventfd_close(loop->backend.threadpool.evtfd[1]);
    loop->backend.threadpool.evtfd[1] = -1;
}
