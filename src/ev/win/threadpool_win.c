
static void _on_work_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred; (void)arg;

    ev_loop_t* loop = EV_CONTAINER_OF(iocp, ev_loop_t, backend.threadpool.io);

    ev__threadpool_process(loop);
}

EV_LOCAL void ev__threadpool_wakeup(ev_loop_t* loop)
{
    ev__iocp_post(loop, &loop->backend.threadpool.io);
}

EV_LOCAL void ev__threadpool_init_win(ev_loop_t* loop)
{
    ev__iocp_init(&loop->backend.threadpool.io, _on_work_win, NULL);
}

EV_LOCAL void ev__threadpool_exit_win(ev_loop_t* loop)
{
    (void)loop;
}
