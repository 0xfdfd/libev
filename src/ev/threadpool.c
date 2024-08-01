#include <assert.h>

typedef struct ev_threadpool_default
{
    ev_threadpool_t pool;
    ev_os_thread_t  storage[4];
} ev_threadpool_default_t;

static ev_threadpool_default_t s_default_threadpool;

static void _ev_threadpool_on_init_default(void)
{
    int ret = ev_threadpool_init(&s_default_threadpool.pool, NULL,
        s_default_threadpool.storage, ARRAY_SIZE(s_default_threadpool.storage));
    if (ret != 0)
    {
        EV_ABORT("%s(%d)", ev_strerror(ret), ret);
    }
}

static void _ev_threadpool_init_default(void)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_threadpool_on_init_default);
}

static void _ev_threadpool_on_loop(ev_handle_t* handle)
{
    ev_work_t* work = EV_CONTAINER_OF(handle, ev_work_t, base);

    ev__handle_deactive(&work->base);
    ev__handle_exit(&work->base, NULL);

    work->data.done_cb(work, work->data.status);
}

static void _ev_threadpool_submit_to_loop(ev_work_t* work)
{
    ev_loop_t* loop = work->base.loop;

    work->base.backlog.cb = _ev_threadpool_on_loop;
    ev_mutex_enter(&loop->threadpool.mutex);
    {
        ev_list_push_back(&loop->threadpool.work_queue, &work->base.backlog.node);
    }
    ev_mutex_leave(&loop->threadpool.mutex);

    ev__threadpool_wakeup(loop);
}

static ev_work_t* _ev_threadpool_get_work_locked(ev_threadpool_t* pool)
{
    ev_queue_node_t* it;
    size_t i;

    ev_mutex_enter(&pool->mutex);
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        it = ev_queue_pop_front(&pool->work_queue[i]);
        if (it != NULL)
        {
            ev_queue_init(it);
            break;
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (it == NULL)
    {
        return NULL;
    }

    return EV_CONTAINER_OF(it, ev_work_t, node);
}

static void _ev_threadpool_commit(ev_work_t* work, int status)
{
    work->data.status = status;
    _ev_threadpool_submit_to_loop(work);
}

static void _ev_threadpool_do_work(ev_threadpool_t* pool)
{
    ev_work_t* work = _ev_threadpool_get_work_locked(pool);
    if (work == NULL)
    {
        return;
    }

    work->data.status = EV_EBUSY;
    work->data.work_cb(work);

    _ev_threadpool_commit(work, 0);
}

static void _ev_threadpool_cleanup(ev_threadpool_t* pool)
{
    ev_work_t* work;
    while ((work = _ev_threadpool_get_work_locked(pool)) != NULL)
    {
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_worker(void* arg)
{
    ev_threadpool_t* pool = arg;

    while (pool->looping)
    {
        ev_sem_wait(&pool->p2w_sem);
        _ev_threadpool_do_work(pool);
    }
    _ev_threadpool_cleanup(pool);
}

static void _cancel_work_queue_for_loop(ev_threadpool_t* pool,
        ev_queue_node_t* wqueue, ev_loop_t* loop)
{
    ev_queue_node_t* it;

    ev_mutex_enter(&pool->mutex);
    {
        it = ev_queue_head(wqueue);
        while (it != NULL)
        {
            ev_work_t* work = EV_CONTAINER_OF(it, ev_work_t, node);
            it = ev_queue_next(wqueue, it);

            if (work->base.loop != loop)
            {
                continue;
            }

            _ev_threadpool_commit(work, EV_ECANCELED);
        }
    }
    ev_mutex_leave(&pool->mutex);
}

int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num)
{
    int ret;
    size_t i, idx;

    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        ev_queue_init(&pool->work_queue[i]);
    }

    pool->threads = storage;
    pool->thrnum = num;
    pool->looping = 1;
    ev_mutex_init(&pool->mutex, 0);
    ev_sem_init(&pool->p2w_sem, 0);

    for (idx = 0; idx < num; idx++)
    {
        ret = ev_thread_init(&storage[idx], opt, _ev_threadpool_worker, pool);
        if (ret < 0)
        {
            goto err_release_thread;
        }
    }

    return 0;

err_release_thread:
    pool->looping = 0;
    for (i = 0; i < idx; i++)
    {
        ev_thread_exit(&storage[i], EV_INFINITE_TIMEOUT);
    }
    ev_sem_exit(&pool->p2w_sem);
    ev_mutex_exit(&pool->mutex);
    return ret;
}

int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_work_t* work, ev_work_type_t type,
    ev_work_cb work_cb, ev_work_done_cb done_cb)
{
    if (!pool->looping)
    {
        return EV_EACCES;
    }
    assert(type < ARRAY_SIZE(pool->work_queue));

    ev__handle_init(loop, &work->base, EV_ROLE_EV_WORK);
    ev__handle_active(&work->base);
    work->data.pool = pool;
    work->data.status = EV_ELOOP;
    work->data.work_cb = work_cb;
    work->data.done_cb = done_cb;

    ev_mutex_enter(&pool->mutex);
    {
        ev_queue_push_back(&pool->work_queue[type], &work->node);
    }
    ev_mutex_leave(&pool->mutex);

    ev_sem_post(&pool->p2w_sem);

    return 0;
}

int ev_loop_cancel(ev_work_t* work)
{
    ev_threadpool_t* pool = work->data.pool;

    int cancelled;
    ev_mutex_enter(&pool->mutex);
    {
        cancelled = !ev_queue_empty(&work->node);
        if (cancelled)
        {
            ev_queue_erase(&work->node);
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (!cancelled)
    {
        return EV_EBUSY;
    }

    _ev_threadpool_commit(work, EV_ECANCELED);

    return 0;
}

static void _threadpool_cancel_work_queue(ev_threadpool_t* pool, ev_queue_node_t* wqueue)
{
    (void)pool;
    ev_queue_node_t* node;
    while ((node = ev_queue_pop_front(wqueue)) != NULL)
    {
        ev_work_t* work = EV_CONTAINER_OF(node, ev_work_t, node);
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_cancel_all(ev_threadpool_t* pool)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _threadpool_cancel_work_queue(pool, &pool->work_queue[i]);
    }
}

void ev_threadpool_exit(ev_threadpool_t* pool)
{
    size_t i;
    int errcode;

    /* stop loop */
    pool->looping = 0;
    for (i = 0; i < pool->thrnum; i++)
    {
        ev_sem_post(&pool->p2w_sem);
    }

    /* exit thread */
    for (i = 0; i < pool->thrnum; i++)
    {
        errcode = ev_thread_exit(&pool->threads[i], EV_INFINITE_TIMEOUT);
        if (errcode != 0)
        {
            EV_ABORT("ev_thread_exit:%d", errcode);
        }
    }

    /* now we can do some cleanup */
    _ev_threadpool_cancel_all(pool);

    ev_mutex_exit(&pool->mutex);
    ev_sem_exit(&pool->p2w_sem);
}

EV_LOCAL void ev__loop_link_to_default_threadpool(ev_loop_t* loop)
{
    _ev_threadpool_init_default();
    if (loop->threadpool.pool == NULL)
    {
        ev_loop_link_threadpool(loop, &s_default_threadpool.pool);
    }
}

EV_LOCAL int ev__loop_submit_threadpool(ev_loop_t* loop,
    ev_work_t* work,
    ev_work_type_t type, ev_work_cb work_cb,
    ev_work_done_cb done_cb)
{
    ev_threadpool_t* pool = loop->threadpool.pool;

    return ev_threadpool_submit(pool, loop, work, type, work_cb, done_cb);
}

EV_LOCAL int ev_loop_link_threadpool(ev_loop_t* loop, ev_threadpool_t* pool)
{
    if (loop->threadpool.pool != NULL)
    {
        return EV_EBUSY;
    }

    loop->threadpool.pool = pool;

    ev_mutex_enter(&pool->mutex);
    {
        ev_list_push_back(&pool->loop_table, &loop->threadpool.node);
    }
    ev_mutex_leave(&pool->mutex);

    return 0;
}

int ev_loop_unlink_threadpool(ev_loop_t* loop)
{
    size_t i;
    ev_threadpool_t* pool = loop->threadpool.pool;

    /**
     * Cancel all pending task from target loop.
     * However, due to #ev_threadpool_submit() is a public API, there might be
     * some tasks which not come from loop directly. Thus they can be unexpected
     * canceled as well.
     */
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _cancel_work_queue_for_loop(pool, &pool->work_queue[i], loop);
    }

    /* From now there should no futher work from loop incoming. */
    loop->threadpool.pool = NULL;
    ev_mutex_enter(&pool->mutex);
    {
        ev_list_erase(&pool->loop_table, &loop->threadpool.node);
    }
    ev_mutex_leave(&pool->mutex);

    return 0;
}

void ev_threadpool_default_cleanup(void)
{
    if (s_default_threadpool.pool.looping)
    {
        ev_threadpool_exit(&s_default_threadpool.pool);
    }
}

int ev_loop_queue_work(ev_loop_t* loop, ev_work_t* token,
    ev_work_cb work_cb, ev_work_done_cb done_cb)
{
    return ev_threadpool_submit(loop->threadpool.pool, loop, token,
        EV_THREADPOOL_WORK_CPU, work_cb, done_cb);
}

EV_LOCAL void ev__threadpool_process(ev_loop_t* loop)
{
    ev_list_node_t* node;
    for (;;)
    {
        ev_mutex_enter(&loop->threadpool.mutex);
        {
            node = ev_list_pop_front(&loop->threadpool.work_queue);
        }
        ev_mutex_leave(&loop->threadpool.mutex);

        if (node == NULL)
        {
            break;
        }

        ev_handle_t* handle = EV_CONTAINER_OF(node, ev_handle_t, backlog.node);
        handle->backlog.cb(handle);
    }
}
